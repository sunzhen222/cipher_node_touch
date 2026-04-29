---
name: embeded
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
| 编译并通过J-LINK 烧录 (暂未实现) | `.\build.bat flash` |
| 清除重编 + 烧录 | `.\build.bat rebuild copy` |

**编译流程说明**：
1. CMake 配置 + Ninja 构建
2. 生成 `cipher_node_touch.bin` 后，用 `OTA_File_Maker_Console.exe` 打包为 `update.bin`
3. 若带 `copy` 参数，将 `update.bin` 复制到 `j:/update.bin`（设备挂载盘符）

**前置依赖**：按工程依赖正常安装即可。

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

## 常见问题

- **`j:/update.bin` 复制失败**：确认设备已通过 USB 挂载且盘符为 `J:`
- **CMake 缓存问题**：使用 `.\build.bat rebuild` 清除构建目录后重新编译
