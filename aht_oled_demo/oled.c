
#include "oled.h"

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include "wifiiot_i2c.h"    // i2c 方法函数
#include "wifiiot_errno.h"  // i2c 状态信息


unsigned char oled_buff[8][128];    // 全屏显示内容

// I2C协议 读写函数 只有写需求, cd = 0 写指令 cd = 1 写数据 byt 要写入的值 
void SSD1306_I2C_W(unsigned char cd, unsigned char byt)
{

    unsigned int state = 0; // I2C 运行 状态

    WifiIotI2cIdx id = WIFI_IOT_I2C_IDX_0;  //I2C 通道 0

    unsigned short deviceAddr = 0x78; // SSD1306 地址 

    WifiIotI2cData i2cData = {0};   // 接收发送信息的数组 查 wifiiot_i2c.h 看详细说明

    unsigned char buf[] = {0x00, byt};  //默认 0x00 写入 指令集 byt 要写入的指令  

    if(cd == 1)     // 输入 数据
    {
        buf[0] = 0x40;  // 0x40 表示写入的是数据 byt 就是要写入的数据              
    }

    i2cData.sendBuf = buf;
    i2cData.sendLen = 2;

    state = I2cWrite(id, deviceAddr, &i2cData);
    
    if(state != WIFI_IOT_SUCCESS)
    {
        printf("[SSD1306_I2C_W] write error ： < %d > !!! \r\n", state);   // 如果状态异常 就打印 错误信息
    }

    // return state;    // 也可以作为返回值 

}


// SSD1306 初始化
void OLED_Init(void)
{

    SSD1306_I2C_W(0, 0xAE); // 关闭OLED

    // 列地址 由两部分组成 半个上 半个下 可以理解为把光标 停到起始位 
    SSD1306_I2C_W(0, 0x00); // 
    SSD1306_I2C_W(0, 0x10); // 
    SSD1306_I2C_W(0, 0xB0); // 设置页地址 

    SSD1306_I2C_W(0, 0x20); // 内存 地址 模式
    SSD1306_I2C_W(0, 0x00); // 0x00 水平(我采用的方式)  0x01 垂直  0x02 页模式（例程采用方式）

    // -----------

    SSD1306_I2C_W(0, 0xD5); // 设置显示时钟分频率、振荡器频率
    SSD1306_I2C_W(0, 0x80); // 1000 0000

    SSD1306_I2C_W(0, 0xA8); // duty设置
    SSD1306_I2C_W(0, 0x3F); // 1/64

    SSD1306_I2C_W(0, 0xD3); // 设置显示偏移
    SSD1306_I2C_W(0, 0x00); // 不偏

    SSD1306_I2C_W(0, 0x40); // 设置显示起始行 开始位置40 ~ 7F 

    // 如果使用内部DC/DC电路 要打开 ？？？ 有的例子开 有的例子不开 不开是点不亮滴
    SSD1306_I2C_W(0, 0x8D); // 设置电荷泵
    SSD1306_I2C_W(0, 0x14); // 0x10 10000 [2] = 0 禁用 0x14 10100 [2] = 1 启用 

    SSD1306_I2C_W(0, 0xA1); // 设置段重映射    

    SSD1306_I2C_W(0, 0xC8); // 扫描方向
    SSD1306_I2C_W(0, 0xA1); // 从左到右

    SSD1306_I2C_W(0, 0xDA); // 设置COM引脚
    SSD1306_I2C_W(0, 0x12); // 

    SSD1306_I2C_W(0, 0x81); // 微调对比度
    SSD1306_I2C_W(0, 0x66); // 1~128

    SSD1306_I2C_W(0, 0xD9); // 设置预充电时间
    SSD1306_I2C_W(0, 0xF1); // ？ 充电时间有长有短 选个长的

    SSD1306_I2C_W(0, 0xDB); // 设置 VCOMH 采用传输协议不需要改吧？？？ 
    SSD1306_I2C_W(0, 0x30); // 

    SSD1306_I2C_W(0, 0xA4); // 输出 RAM 0xA4输出RAM 0xA5不输出RAM

    SSD1306_I2C_W(0, 0xA6); // 设置正显 0xA6正显 0xA7反显

    SSD1306_I2C_W(0, 0xAF); // 开启OLED

    usleep(100*1000);   //等待100ms

}

// OLED 显示信息
void OLED_Show(void)
{

    unsigned char i, j;

    for(i=0; i<8; i++)
    {
        for(j=0; j<128; j++)
        {
            SSD1306_I2C_W(1, oled_buff[i][j]);
        }
    }

}

//清空显示数组
void OLED_Clear(void)
{

    unsigned char i, j;

    for(i=0; i<8; i++)
    {
        for(j=0; j<128; j++)
        {
            oled_buff[i][j] = 0x00;
        }
    }

}

//在某个任意坐标点 输入文字 横坐标x: 0~127 竖坐标y: 0~63 *c 文字显示阵列数组，默认高度都是16，为什么，因为好算，l是文字的宽度，汉字宽16，数字宽8 
void OLED_Word(unsigned char x, unsigned char y, unsigned char *c, unsigned char l)
{
    // 输入 x,y 要算好再输入 我没做检查错误

    unsigned char i, m, n;

    m = y/8;
    n = y%8;

    for(i=0; i<l; i++)
    {
        unsigned char d1, d2;

        d1 = c[i];
        d2 = c[i+l];

        if(n == 0){
            oled_buff[m][x+i] = d1;
            oled_buff[m+1][x+i] = d2;
        }
        else
        {
            oled_buff[m][x+i] = d1 << n;
            oled_buff[m+1][x+i] = (d1 >> (8-n)) | (d2 << n);
            oled_buff[m+2][x+i] = d2 >> (8-n);
        }
    }

    // 以上代码 可能有bug 谨慎用

}