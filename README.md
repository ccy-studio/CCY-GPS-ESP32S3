# GPS Dashboard For  ESP32-S3



> 这是一个基于ESP32S3为主控UI框架使用LVGL9.2开发的一款GPS仪表盘码表



## 硬件设计

- 集成充电芯片，支持3.7V电池充电，自动的电源路径管理
- 集成电量计芯片
- 设计有一键开关机电路
- 使用中科微的GPS模块
- 主控为ESP32-S3R8
  - 内置PSRAM 8 MB (Octal SPI) 
  - 外置Flash 16M
- 设计有SDIO接口的SD/TF卡
- 使用ESP32-USB外设调试
- 三个用户按键、其中一个作为开机键



## 软件设计

核心框架使用LVGL9.2、ESP-IDF 5.3.1

项目框架设计为模块化设计，使用CMake进行管理

lvgl源码、项目UI代码(framework) 一套代码同时为SDL模拟器、ESP32IDF共同使用，使用CMake环境变量区分不同的编译文件

script脚本文件夹，提供了字体生成器、图片生成器、图片合成器(多图生成一个.bin文件)，脚本全部使用Python编写。





## 使用

要拉取包含子模块的Git仓库，请按照以下步骤操作：

1. 克隆包含子模块的仓库：

   ``git clone --recurse-submodules https://github.com/ccy-studio/CCY-GPS-ESP32S3.git``

2. 如果已经克隆了不包含子模块的仓库，可以运行以下命令来初始化并更新子模块：

   ``git submodule update --init --recursive``

3. 如果需要更新现有的子模块，可以进入子模块目录并拉取最新代码：

   ````shell
   cd path/to/submodule
   git pull
   ````

   



## 其他

等待更新.....

