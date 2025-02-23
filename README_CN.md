# RP2040电压电流表

使用[带屏版的12指神探](https://www.eetree.cn/platform/2585)制作的非常简单的电压/电流表.

## 特点
* 深色/浅色主题
* 自动量程切换
* 电压/电流同时测量
* 电压测量范围：0 - 14V
* 电流测量范围：0 - 1.4A
* 电压测量精度：10mV
* 电流测量精度：10mA

## 硬件方案
![hw_diagram](https://github.com/user-attachments/assets/e25b0dbe-360e-40d7-817c-86210c92af1b)  
该仪表使用`带屏版的12指神探`作为主控，它是由`硬禾学堂`制造的开发板，具有以下特点：
- MCU: RP2040，双核 ARM Cortex-M0+ 处理器，具有 264KB 的 SRAM
- 闪存：2MB SPI NOR Flash
- 显示屏：240x240 ST7789 LCD
- USB-C接口用于供电和编程/调试
- 一个 12 针连接器用于电源输出和 9 个 GPIO（包括 3 个 ADC 通道）  

该仪表使用一块自制的模拟前端扩展板，用于电压/电流信号的采集和初步处理，原理图如下：  
![sch](https://github.com/user-attachments/assets/30df5474-728e-4151-9ffa-18b45bcf70e0)  
其中左下角为扩展板的电源，从主控取5V供电，用一个LDO产生3V3的电压给模拟前端。虽然主控已经有3V3电源输出，但是查原理图发现其它是DC-DC电源，纹波较大，不适合模拟电路使用，因此扩展板采用了独立的LDO。  
原理图其余部分左边是电压信号处理，使用低压Rail-to-Rail运放`LMV321`搭建了一个差分放大器，用双刀四掷模拟开关`CH444`切换放大器增益来切换量程，由两个GPIO控制。最后经过一个RC低通滤波器输出到主控的ADC通道。  
右边是电流信号处理，基本组成和电压信号处理一样，只是多了一个采样电阻，并且增益更大，最后输出到主控的另一个ADC通道。

#### 运放差分放大器原理简介
运放差分放大器是一种用于放大两个输入信号差值的放大器。  
![diff_amp](https://github.com/user-attachments/assets/b688dcc6-d232-43f2-9c69-8ff72eb6f24e)  
上面是一个基本的应用原理图，其中$R_2=R_4$，$R_1=R_3$，$v_1$和$v_2$是输入信号，$v_{out}$是输出信号。该放大器的输出为
$$A=(v_2-v_1)\frac{R_2}{R_1}$$
因此可以通过同时调整$R_2$和$R_4$来调整增益。此外该放大器的差分输入阻抗为$R_{id}=2R_1$。

#### 为什么电压采样不直接用电阻分压？
* 电阻分压只能降低电压，不能放大电压，因此在输入电压较小时精度较低
* 使用运放差分放大器输入阻抗比较恒定，基本不随量程变化而变化

## 软件
该仪表的软件使用 PlatformIO 开发，感谢 earlephilhower 的 [arduino-pico core](https://github.com/earlephilhower/arduino-pico)  
软件的UI使用[LVGL](https://github.com/lvgl/lvgl)；屏幕驱动使用[TFT_eSPI库](https://github.com/Bodmer/TFT_eSPI)；日志使用[ulog库](https://github.com/rdpoor/ulog)。  
软件主要分为以下模块：
* `main.cpp` - 主程序，包括初始化和主循环
* `Display` - 显示，负责构建显示内容
* `VoltMeter` - 电压，用于配置放大器的增益、读取/平滑ADC数值和计算电压值
* `KeyPad` - 按键，用于读取按键输入作为LVGL的输入设备
* `Console` - 控制台，用于输出日志信息和提供命令行接口

RP2040有两个CPU核心，一个专门用于处理显示，另一个用于处理电压/电流采集和计算等。

### Display
这个模块负责构建显示内容，包括初始化和更新数值。

``` C++
namespace Display
{
    // 读取按键事件的回调函数定义，返回按键值和是否按下
    using ReadKeyEventCallback = std::function<std::pair<uint32_t, bool>()>;

    // 初始化显示
    void init();

    // 处理显示事件，需要在循环中反复调用
    void run();

    // 设置读取按键事件的回调函数
    void setReadKeyEventCb(ReadKeyEventCallback);

    // 更新电压显示
    void updateVoltage(const float);

    // 更新电流显示
    void updateCurrent(const float);

} // namespace display
```

### VoltMeter
这个类包含一个缓冲区，每次ADC转换后将数据存储在缓冲区中。当需要读取电压时，将缓冲区中的数据取出计算平均值再转换为电压值。只要缓冲区填满后才能成功读取电压值。  
该类的原型如下：
```C++
class VoltMeter
{
    ...
public:
    // 构造函数，传入ADC通道和量程切换控制引脚
    VoltMeter(uint32_t adc_pin, uint32_t scale_pin0, uint32_t scale_pin1);

    // 设置放大器增益
    inline void setGains(const float scale0_gain, const float scale1_gain, const float scale2_gain, const float scale3_gain);

    inline void setGains(const float gains[4]);

    // 选择量程
    inline void selectScale(const uint8_t scale);

    // 获取当前量程
    inline uint8_t getActiveScale();

    // 读取ADC数值并塞进缓冲区，需要在循环中反复调用
    void convertOnce();

    // 从缓冲区数据获取输入ADC电压数值
    float getRawVoltage();

    // 读取被测电压
    float readVoltage();
};
```

### KeyPad
这个类通过中断读取按键输入，并提供获取最新按键状态的接口。虽然最后只用到了一个键，但是我懒得改了。
``` C++

class KeyPad
{
private:
...
    // 按键中断的回调函数
    static void onKeyStateChange(void *params);

public:
    // 构造函数，传入按键引脚和是否为低电平触发
    KeyPad(std::span<const int> keys, bool activeLow = true);

    // 构造函数，不传入按键引脚
    KeyPad(bool activeLow = true);

    // 添加按键到按键列表
    void addKey(int key);

    // 获取最后一个按键事件
    std::pair<int, bool> getLastKeyEvent();
};
```

### Console
这个模块提供了一个简单的命令行接口，可以通过串口查看日志和输入命令来调试仪表。  
``` C++
namespace Console
{
    // 命令回调函数定义，传入参数列表
    using CmdCb = std::function<void(std::span<String>)>;

    // 命令结构体定义，包括命令名、帮助信息、参数个数、回调函数
    struct Command
    {
        const char *name;
        const char *help;
        uint8_t minArgCount;
        uint8_t maxArgCount;
        CmdCb cb;
    };

    // 初始化串口和日志系统
    void init();

    // 注册命令
    void registerCommand(const Command &cmd);

    // 处理控制台事件，需要在循环中反复调用
    void handleConsoleEvent();

} // namespace Console
```
当前的软件只添加了俩命令：`help`和`cal`，分别用于查看帮助信息和校准。
```
8=> help
help - Display the help message
  Usage: help [command]

cal - Calibrate the current and voltage scales
  Usage: cal <start|save|exit|scale|in|gains> [options]
        cal start <u|i> - Start the calibration process for the voltage or current scale, should be run before other calibration commands.
        cal save - Save the calibration data for the voltage or current scale and exit
        cal exit - Exit the calibration process and discard the changes
        cal scale [level] - Show or set the scale level(0-3)
        cal in <value> - Input the actual value(in V or A)
        cal gains - Show the current gains setting
```

### 主程序
主程序定义了两个核心执行的函数。核心0首先初始化各个模块（除了显示），从EEPROM中读取校准数据并应用，然后进入主循环。主循环中不断读取电压/电流数值并显示、根据输入值切换量程，并处理控制台事件；核心1则负责初始化显示和处理显示事件。按键事件通过中断处理，因此不需要循环检测。    
代码详见[main.cpp](https://github.com/redstonee/RP2040-UI_Meter/blob/main/src/main.cpp)。

## 使用
### 构建软件
0. 安装PlatformIO
1. 克隆本仓库
2. 用PlatformIO打开本仓库
3. 连接RP2040并构建和上传

### 校准
1. 连接RP2040，打开串口监视器
2. 输入`cal start u`开始电压校准
3. 输入`cal scale x`选择量程x (0-3)，将会显示该量程的电压范围
3. 把该范围内已知电压连接到表笔上
4. 输入`cal in xx.xx`输入实际的电压，然后程序会计算出这个量程的增益
5. 重复3-5步骤，直到所有量程的增益都校准完
6. 输入`cal save`保存校准数据
7. 输入`cal start i`开始电流校准，重复3-6步骤

### 测量
把被测电压/电流连接到表笔上，然后读取显示即可。  
如果输入超出量程，会显示`Overload`。  
可以按下按键来切换深色/浅色主题。

### 实测的电压表输入特性曲线
取了几个输入电压测量通过电压表的电流，计算电压表的输入阻抗：  
![u_input_feature](https://github.com/user-attachments/assets/5ce9e94a-5707-44bd-adb8-c14be9b28f2a)
可以看到在不同量程下输入阻抗都在430kΩ附近，与理论值基本相符。
