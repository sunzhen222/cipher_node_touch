param(
    [string]$Port = "COM11",
    [int]$BaudRate = 921600,
    [int]$ReadTimeoutMs = 1500,
    [int]$StartTimeoutMs = 20000,
    [int]$CommandTimeoutMs = 3000,
    [string]$StartPattern = "device started",
    [string]$CommandsFile = ".\tools\serial_test_plan.json",
    [string]$OutputRoot = ".\test_output",
    [switch]$AutoDetectPort,
    [switch]$SkipStartWait
)

$ErrorActionPreference = "Stop"
$script:LogPath = $null
$script:AllOutput = ""

function Get-AvailablePorts {
    $ports = [System.IO.Ports.SerialPort]::GetPortNames()
    return $ports | Sort-Object
}

function Resolve-Port {
    param(
        [string]$SpecifiedPort,
        [switch]$AutoDetect
    )

    $ports = Get-AvailablePorts
    if (-not $ports -or $ports.Count -eq 0) {
        Write-Host "[ERROR] No serial ports found. Please check USB connection and driver." -ForegroundColor Red
        exit 2
    }

    if ($AutoDetect) {
        if ([string]::IsNullOrWhiteSpace($SpecifiedPort)) {
            $detected = $ports[0]
            Write-Host "[INFO] Auto-detected serial port: $detected"
            return $detected
        }

        if ($ports -contains $SpecifiedPort) {
            Write-Host "[INFO] Using specified serial port: $SpecifiedPort"
            return $SpecifiedPort
        }

        $detected = $ports[0]
        Write-Host "[WARN] Specified port $SpecifiedPort not found. Falling back to auto-detected port $detected"
        return $detected
    }

    if (-not ($ports -contains $SpecifiedPort)) {
        Write-Host "[ERROR] Specified serial port $SpecifiedPort does not exist." -ForegroundColor Red
        Write-Host "[INFO] Available ports: $($ports -join ', ')" -ForegroundColor Yellow
        Write-Host "[HINT] If the port is in use by another tool, close that tool and retry." -ForegroundColor Yellow
        exit 3
    }

    return $SpecifiedPort
}

function New-TestSessionFiles {
    param(
        [string]$Root,
        [string]$ResolvedPort
    )

    if (-not (Test-Path -LiteralPath $Root)) {
        New-Item -ItemType Directory -Path $Root | Out-Null
    }

    $timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
    $safePort = $ResolvedPort -replace "[^a-zA-Z0-9_-]", "_"
    $baseName = "serial_test_${timestamp}_${safePort}"
    $logPath = Join-Path $Root ($baseName + ".log")
    $resultPath = Join-Path $Root ($baseName + ".json")
    New-Item -ItemType File -Path $logPath -Force | Out-Null
    return [PSCustomObject]@{ LogPath = $logPath; ResultPath = $resultPath; BaseName = $baseName }
}

function Append-Log {
    param([string]$Text)

    if ([string]::IsNullOrEmpty($Text)) {
        return
    }
    Add-Content -LiteralPath $script:LogPath -Value $Text -Encoding UTF8
    $script:AllOutput += $Text
}

function Add-LogLine {
    param([string]$Line)
    $stamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss.fff"
    Append-Log "[$stamp] $Line`r`n"
}

function Read-ForDuration {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [int]$TimeoutMs
    )

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $captured = ""
    while ($sw.ElapsedMilliseconds -lt $TimeoutMs) {
        try {
            $chunk = $Serial.ReadExisting()
            if (-not [string]::IsNullOrEmpty($chunk)) {
                $captured += $chunk
                Append-Log $chunk
            } else {
                Start-Sleep -Milliseconds 20
            }
        } catch {
            Start-Sleep -Milliseconds 20
        }
    }
    return $captured
}

function Wait-ForStartup {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [string]$Pattern,
        [int]$TimeoutMs
    )

    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    while ($sw.ElapsedMilliseconds -lt $TimeoutMs) {
        $chunk = Read-ForDuration -Serial $Serial -TimeoutMs 200
        if (($script:AllOutput -match $Pattern) -or ($chunk -match $Pattern)) {
            Add-LogLine "startup pattern matched: $Pattern"
            return $true
        }
    }
    Add-LogLine "startup pattern not found within timeout: $Pattern"
    return $false
}

function Get-TestCommands {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "Commands file not found: $Path"
    }

    $raw = Get-Content -LiteralPath $Path -Raw -Encoding UTF8
    $parsed = $raw | ConvertFrom-Json
    if ($null -eq $parsed.commands -or $parsed.commands.Count -eq 0) {
        throw "Commands file has no commands: $Path"
    }

    $normalized = @()
    foreach ($item in $parsed.commands) {
        if ($item -is [string]) {
            $normalized += [PSCustomObject]@{ command = $item; expect = $null; timeoutMs = $null }
        } else {
            $normalized += [PSCustomObject]@{
                command = $item.command
                expect = $item.expect
                timeoutMs = $item.timeoutMs
            }
        }
    }

    return $normalized
}

function Invoke-CommandStep {
    param(
        [System.IO.Ports.SerialPort]$Serial,
        [PSCustomObject]$Step,
        [int]$DefaultTimeoutMs
    )

    $cmd = [string]$Step.command
    if ([string]::IsNullOrWhiteSpace($cmd)) {
        return [PSCustomObject]@{ Command = $cmd; Passed = $true; Expect = $null }
    }

    $timeoutMs = $DefaultTimeoutMs
    if ($null -ne $Step.timeoutMs -and [int]$Step.timeoutMs -gt 0) {
        $timeoutMs = [int]$Step.timeoutMs
    }

    Add-LogLine "send command: #$cmd"
    $Serial.Write("#$cmd`r`n")
    $response = Read-ForDuration -Serial $Serial -TimeoutMs $timeoutMs

    $passed = $true
    if (-not [string]::IsNullOrWhiteSpace($Step.expect)) {
        $passed = ($response -match [string]$Step.expect)
    }

    return [PSCustomObject]@{
        Command = $cmd
        Passed = $passed
        Expect = $Step.expect
        TimeoutMs = $timeoutMs
    }
}

function Write-ResultJson {
    param(
        [string]$Path,
        [string]$PortName,
        [int]$Baud,
        [string]$Pattern,
        [bool]$Started,
        [object[]]$CommandResults,
        [string]$LogPath
    )

    $failed = @($CommandResults | Where-Object { -not $_.Passed })
    $payload = [PSCustomObject]@{
        testTime = (Get-Date).ToString("o")
        port = $PortName
        baudRate = $Baud
        startPattern = $Pattern
        startMatched = $Started
        totalCommands = $CommandResults.Count
        failedCommands = $failed.Count
        passed = ($Started -and $failed.Count -eq 0)
        commands = $CommandResults
        logFile = $LogPath
    }
    $json = $payload | ConvertTo-Json -Depth 6
    Set-Content -LiteralPath $Path -Value $json -Encoding UTF8
}

$resolvedPort = Resolve-Port -SpecifiedPort $Port -AutoDetect:$AutoDetectPort
$session = New-TestSessionFiles -Root $OutputRoot -ResolvedPort $resolvedPort
$script:LogPath = $session.LogPath
$serial = $null

try {
    $commands = Get-TestCommands -Path $CommandsFile

    $serial = New-Object System.IO.Ports.SerialPort $resolvedPort, $BaudRate, ([System.IO.Ports.Parity]::None), 8, ([System.IO.Ports.StopBits]::One)
    $serial.ReadTimeout = $ReadTimeoutMs
    $serial.WriteTimeout = $ReadTimeoutMs
    $serial.NewLine = "`n"
    $serial.DtrEnable = $false
    $serial.RtsEnable = $false
    $serial.Open()

    Write-Host "[INFO] Opened $resolvedPort @ $BaudRate"
    Write-Host "[INFO] Output log: $($session.LogPath)"
    Write-Host "[INFO] Result json: $($session.ResultPath)"
    Add-LogLine "opened port $resolvedPort @ $BaudRate"
    Add-LogLine "commands file: $CommandsFile"

    $started = $true
    if (-not $SkipStartWait) {
        Write-Host "[INFO] Waiting startup pattern: $StartPattern"
        $started = Wait-ForStartup -Serial $serial -Pattern $StartPattern -TimeoutMs $StartTimeoutMs
        if (-not $started) {
            Write-Host "[ERROR] Startup pattern '$StartPattern' was not detected." -ForegroundColor Red
            Write-Host "[HINT] Check firmware startup log or use -SkipStartWait for manual testing." -ForegroundColor Yellow
        }
    }

    $results = @()
    if ($started) {
        foreach ($step in $commands) {
            $res = Invoke-CommandStep -Serial $serial -Step $step -DefaultTimeoutMs $CommandTimeoutMs
            $results += $res
            if ([string]::IsNullOrWhiteSpace($res.Expect)) {
                Write-Host "[INFO] Sent #$($res.Command)" -ForegroundColor Cyan
            } elseif ($res.Passed) {
                Write-Host "[PASS] #$($res.Command) -> matched /$($res.Expect)/" -ForegroundColor Green
            } else {
                Write-Host "[FAIL] #$($res.Command) -> not matched /$($res.Expect)/" -ForegroundColor Red
            }
        }
    }

    # Collect tail output for stable logs after last command.
    Read-ForDuration -Serial $serial -TimeoutMs 300 | Out-Null

    Write-ResultJson -Path $session.ResultPath -PortName $resolvedPort -Baud $BaudRate -Pattern $StartPattern -Started $started -CommandResults $results -LogPath $session.LogPath

    if (-not $started) {
        Write-Host "[SUMMARY] Serial test failed: startup pattern not found." -ForegroundColor Red
        exit 11
    }

    $failed = @($results | Where-Object { -not $_.Passed })
    if ($failed.Count -gt 0) {
        Write-Host "[SUMMARY] Serial test failed: $($failed.Count)/$($results.Count) expected checks failed." -ForegroundColor Red
        exit 10
    }

    Write-Host "[SUMMARY] Serial test finished. Commands sent: $($results.Count)." -ForegroundColor Green
    exit 0
}
catch [System.UnauthorizedAccessException] {
    Write-Host "[ERROR] Failed to open ${resolvedPort}: access denied (port may be in use)." -ForegroundColor Red
    Write-Host "[HINT] Close serial monitor tools (e.g. VSCode Serial Monitor, PuTTY) and retry." -ForegroundColor Yellow
    exit 4
}
catch {
    Write-Host "[ERROR] Serial test failed: $($_.Exception.Message)" -ForegroundColor Red
    if ($session -and $session.ResultPath) {
        try {
            Write-ResultJson -Path $session.ResultPath -PortName $resolvedPort -Baud $BaudRate -Pattern $StartPattern -Started $false -CommandResults @() -LogPath $session.LogPath
        } catch {
        }
    }
    exit 5
}
finally {
    if ($serial -ne $null -and $serial.IsOpen) {
        Add-LogLine "closing serial port"
        $serial.Close()
    }
}
