---
name: embedded
description: 本工程嵌入式开发专用技能。适用于编译、烧录、调试、代码格式化、驱动开发、RTOS任务、UI开发等嵌入式相关任务。关键词：STM32、FreeRTOS、LVGL、CMake、Ninja、arm-none-eabi、build.bat、OTA。
applyTo: "**"
---

# 嵌入式开发技能 - cipher_node_touch

## 项目概述

- **目标芯片**：STM32F405RGTx（Cortex-M4，带 FPU）
- **构建系统**：CMake + Ninja（工具链：`arm-none-eabi-gcc`）
- **UI 框架**：LVGL v9（移植在 `src/porting/`）
- **RTOS**：FreeRTOS（含 CMSIS-RTOS V2 封装）
- **固件升级**：OTA，生成 `update.bin`（AES 加密）

---

## 编译命令

> 所有编译命令均在**工程根目录**下执行。

| 操作 | 命令 |
|------|------|
| 增量编译 | `.\build.bat` |
| 清除重新编译 | `.\build.bat rebuild` |
| 编译并复制到设备（U盘 OTA 烧录） | `.\build.bat copy` |
| 编译并通过 J-LINK 烧录 | `.\build.bat flash` |
| 清除重编 + 烧录 | `.\build.bat rebuild flash` |
| 烧录后串口自动测试（默认 COM11） | `.\tools\flash_and_serial_test.bat` |

**编译流程说明**：
1. CMake 配置 + Ninja 构建
2. 生成 `cipher_node_touch.bin` 后，用 `OTA_File_Maker_Console.exe` 打包为 `update.bin`
3. 若带 `copy` 参数，自动扫描盘符容量并将 `update.bin` 复制到匹配盘（默认 750KB~800KB）
4. 若带 `flash` 参数，且存在 `cipher_node_touch.hex`，调用 `JLink -CommanderScript "../program.jlink"` 进行烧录

**前置依赖**：按工程依赖正常安装即可。

**串口自动测试说明**：
- 默认使用 `COM11`、`921600` 波特率（与 `USART1` 命令口一致）
- 启动检测默认关键字：`device started`（可通过参数覆盖）
- 测试指令来自 `tools/serial_test_plan.json`，可随时增删命令
- 仅串口测试：`powershell -ExecutionPolicy Bypass -File .\tools\serial_test.ps1 -Port COM11 -BaudRate 921600 -CommandsFile .\tools\serial_test_plan.json -StartPattern "device started"`
- 一键烧录+测试：`.\tools\flash_and_serial_test.bat COM11 .\tools\serial_test_plan.json "device started"`
- 每次测试会在 `test_output/` 生成时间戳日志（`.log`）和结果文件（`.json`）
- 若串口不存在或被占用，脚本会提示可用串口和处理建议

### test_cmd 命令通道说明

- 命令接收入口：`USART1` 中断回调（`HAL_UART_RxCpltCallback`）逐字节喂给 `TestCmdRcvByte()`
- 帧格式：以 `#` 开始，以换行 `\n`（或下一个 `#`）结束
- 解析内容：`CmdTask` 在收到 `CMD_MSG_FRAME` 后执行 `CompareAndRunTestCmd((char *)g_testCmdRcvBuffer + 1)`，即去掉首字符 `#` 后做命令匹配
- 超时机制：同一帧字节间隔超过约 `200ms` 会丢弃当前缓冲并重新同步
- 快速示例：发送 `#test\n`，设备会打印 `test!!`

### test_cmd 匹配规则

- 无冒号命令（如 `test`、`all task info`）：全字符串精确匹配
- 带冒号命令（如 `erase flash:`、`lora:`）：前缀匹配，冒号后按空格分割参数，回调参数为 `(argc, argv)`
- 参数解析上限：`CMD_MAX_ARGC = 16`

### test_cmd 扩展规范

- 公共接口在 `src/test/test_cmd.h`
- 注册接口：`RegisterTestCmd(const char *cmdString, const TestCmdFunc_t func)`
- 典型用法：在模块初始化处注册，例如 `RegisterTestCmd("usb:", UsbTestFunc)`、`RegisterTestCmd("lora:", LoraTestFunc)`
- 新增命令建议：
  - 无参数命令使用不带冒号的完整字符串（例：`"show assert"`）
  - 需要参数时使用带冒号前缀（例：`"i2c:"`、`"send_uart2:"`）
  - 回调函数签名统一为：`static void XxxTestFunc(int argc, char *argv[])`

---

## 目录结构

```
src/
  core/          # 主函数、系统初始化、软件/硬件版本、设备设置
  driver/        # 外设驱动（LCD、Touch、SPI、I2C、UART、ADC、DMA 等）
  application/   # 应用层（LCD 绘制、错误保存）
  tasks/         # FreeRTOS 任务（UI、Touch、USB、CMD、Background 等）
  ui/            # LVGL UI（pages、widgets、themes、images、fonts）
  porting/       # LVGL 移植层（显示、输入设备、内存）
  FreeRTOS/      # FreeRTOS 源码及配置
  components/    # 通用组件（CRC校验、cJSON）
  fatfs/         # FatFS 文件系统
  usb_device/    # USB 设备（CDC + MSC）
  msg/           # 消息系统
  utils/         # 工具函数
  test/          # 测试代码
  cm_backtrace/  # 崩溃回溯
third/
  lvgl/          # LVGL 库源码
tools/           # 辅助脚本（OTA）
build/           # 编译输出目录（CMake/Ninja 产物）
```

---

## 硬件资料来源（重要）

- `hardware/` 目录包含当前工程**最新**硬件资料，默认作为硬件相关开发的唯一权威来源。
- 重点包括：
  - 最新原理图网表（用于引脚、网络、电源、总线连接关系确认）
  - 当前项目实际使用的 datasheet（用于时序、寄存器、命令、电气规格确认）
- 当前仓库已确认的关键硬件资料：
  - `hardware/*.tel`：原理图网表文件（文件名可能变化，扩展名固定为 `.tel`，使用时优先选择最新版本）
  - `hardware/stm32f40Xrg.pdf`：STM32F40X 数据手册；嵌入式软件开发最常用的管脚定义集中在 **第 47-59 页**
  - `hardware/SHT3X-DIS.pdf`：SHT30 温湿度传感器数据手册
- 涉及驱动、引脚映射、时序参数、上电流程、复位逻辑等问题时，优先查 `hardware/` 下文件，不以历史聊天结论替代原文。

## Datasheet 读取建议流程

- 目标：快速得到“可落地到代码”的结论，而不是只摘参数。
- 推荐步骤：
  1. 先全文提取 PDF 文本，转为可检索文件（便于快速定位 Command/Timing/Reset/CRC/Conversion）。
  2. 按实现链路阅读：硬件连接 -> 上电与复位 -> 触发测量 -> 读数格式 -> 校验 -> 物理量换算 -> 异常恢复。
  3. 对照命令表与时序表交叉验证，避免“命令正确但等待时间错误”或 ACK/NACK 处理错误。
  4. 最后回看时序图和引脚图，核对边沿约束与引脚行为。
- 在本仓库可使用 Python + pypdf 批量提取，例如：

```powershell
python -c "from pypdf import PdfReader; from pathlib import Path; p=Path('hardware/SHT3X-DIS.pdf'); r=PdfReader(str(p)); texts=[]; [texts.append(f'\n===== PAGE {i+1} =====\n'+(pg.extract_text() or '')) for i,pg in enumerate(r.pages)]; Path('test_output/SHT3X-DIS_extracted.txt').write_text(''.join(texts), encoding='utf-8')"
```

- 提取后优先检索关键词：`I2C address`、`command`、`single shot`、`periodic`、`reset`、`CRC`、`timing`。
- 对 MCU 管脚相关任务，先定位 `hardware/stm32f40Xrg.pdf` 的第 47-59 页，再结合 `hardware/` 下最新 `.tel` 网表做引脚与网络的双向核对。

---

## 代码格式化

使用 AStyle 统一格式化 `src/` 下所有 C/H 文件：

```bat
.\astyle.bat
```

- 格式化样式：`-A3`（Allman/BSD 风格）
- 按需排除自动生成文件，避免手工代码与生成代码混排格式化

---

## 驱动开发规范

- 驱动文件放置在 `src/driver/`，命名格式：`drv_<外设名>.c/.h`
- 初始化函数命名：`drv_<外设名>_init()`
- 驱动对外接口在 `.h` 中声明，内部函数用 `static` 修饰
- 内存分配使用 `SRAM_MALLOC`，**不在调用处检查 NULL**（由 allocator 端 ASSERT 保证）

---

## FreeRTOS 任务规范

- 任务文件放置在 `src/tasks/`，命名格式：`<功能>_task.c/.h`
- 任务函数原型：`void <功能>_task(void *argument)`
- 任务间通信优先使用消息队列（见 `src/msg/`）

---

## 编译输出产物

构建成功后，在 `build/` 目录下生成：

| 文件 | 说明 |
|------|------|
| `cipher_node_touch.elf` | 调试符号完整的 ELF 文件 |
| `cipher_node_touch.bin` | 裸二进制固件 |
| `cipher_node_touch.hex` | Intel HEX 格式固件 |
| `cipher_node_touch.map` | 链接 Map 文件（查看内存分布） |
| `update.bin` | OTA 升级包（AES 加密） |

---

## LoRa 协议宏观注意事项

- 协议头固定顺序为：`Head(1) + Flag(2) + Length(2) + ContentCrc(4)`，其中 `ContentCrc` 使用 `crc32_ieee`。
- `Length` 语义：不包含前 9 字节头（`Head + Flag + Length + ContentCrc`）以及末尾 `CRC16(2)`。
- 整帧长度关系：`total_frame_length = head.length + 11`。
- 内容 CRC32 计算范围：从 `TimeStamp` 开始到 TLV/填充区结束（不含末尾 `CRC16`）。
- 接收校验建议顺序：帧头 -> 长度 -> `CRC16` -> 网络 ID -> 解密（如启用）-> `ContentCrc`。
- `ContentCrc` 失败应直接丢帧，禁止继续 TLV 解析，避免错误密钥解密后产生伪数据导致异常逻辑。
- 加密模式下，明文前缀长度为 9 字节，加密区从字节偏移 9 开始，长度必须 16 字节对齐。
- 命令 `Command ID 0x01` 统一为 `lora_chat`，TLV 必填 `Username(0x01)`、`Text(0x02)`、`AvatarColor(0x03, 3字节RGB)`。

---

## 常见问题

- **`copy` 未执行复制**：脚本会按容量区间自动识别目标盘（默认 750KB~800KB）。若未找到，会提示 `No drive found ... Skip copy.`，请确认设备已通过 USB 挂载且容量落在该区间，或按需调整 `build.bat` 中的 `target_min_bytes` / `target_max_bytes`。
- **CMake 缓存问题**：使用 `.\build.bat rebuild` 清除构建目录后重新编译
