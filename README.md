## Windows环境编译指导

本项目使用 arm-none-eabi-gcc 进行编译，建议使用 MSYS2 进行环境搭建。

### 1. 安装 MSYS2

下载并安装 [MSYS2](https://www.msys2.org/)。

打开 MSYS2 MinGW 64-bit 终端。

逐条运行以下命令安装工具链：
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-arm-none-eabi-gcc
pacman -S mingw-w64-x86_64-make
pacman -S mingw-w64-x86_64-cmake
```

### 2. 环境变量

在环境变量Path增加以下2个路径（根据自己msys64的安装路径）

C:\msys64\mingw64\bin

C:\msys64\usr\bin

环境变量设置完成后需要重启计算机。

### 3. 工具检测

打开终端，输入以下指令确认工具已正常安装：
```bash
gcc --version
arm-none-eabi-gcc --version
make --version
cmake --version
```

如正常显示各个软件版本，说明已正常安装。

### 4. 编译项目

本项目提供 build.bat，双击运行或在终端运行即可完成编译。

### 5. 固件烧录

编译完成后，会在build目录下生成update.bin和cipher_node_touch.hex等文件。

update.bin文件可用模拟U盘的方式升级。

cipher_node_touch.hex是固件原始hex文件，可用J-LINK等工具进行烧录。

