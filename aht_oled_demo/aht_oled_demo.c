
#include "aht20.h"
#include "oled.h"
#include "fonts.h"

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include "wifiiot_i2c.h"    // i2c 方法函数
#include "wifiiot_errno.h"  // i2c 状态信息

// 设置 不变的文字
void OLED_Str(void)
{

    unsigned char l = 16;   //字宽

    unsigned char x1 = 10;  //1行 X
    unsigned char y1 = 12;  //1行 Y 

    // 温度字符坐标组   [ 温度∶    ℃ ] 
    unsigned char tws[4][4] = {
        {(x1-1), y1, 0, l}, {(x1-1+16), y1, 1, l}, {(x1-1+32), y1, 4, l}, {(x1-1+48+40+4), y1, 5, l}
    };

    unsigned char x2 = 10;			//2行 X
    unsigned char y2 = 12+16+8;		//2行 Y

    // 湿度字符坐标组   [ 湿度∶    ％ ] 
    unsigned char hws[4][4] = {
        {(x2-1), y2, 2, l}, {(x2-1+16), y2, 3, l}, {(x2-1+32), y2, 4, l}, {(x2-1+48+40+4), y2, 6, l}
    }; 

    unsigned char i;

    for(i=0; i<4; i++)
    {
        OLED_Word(tws[i][0], tws[i][1], FA[tws[i][2]], tws[i][3]); 
        OLED_Word(hws[i][0], hws[i][1], FA[hws[i][2]], hws[i][3]); 
    } 

}

// 设置 变化的数字
void OLED_Num(float *ht)
{

    char hum[5];	//湿度
    char tem[5];	//温度

    sprintf(hum, "%.2f", ht[0]);    // 转成字符数组
    sprintf(tem, "%.2f", ht[1]);    // 转成字符数组


    unsigned char l = 8;   //字宽

    unsigned char x1 = 10;  //1行 X
    unsigned char y1 = 12;  //1行 Y 

    // 温度字符坐标组    [ xx.xx ] 
    unsigned char tns[5][4] = {
        {(x1-1+48), y1, tem[0]-48, l}, {(x1-1+48+8), y1, tem[1]-48, l}, {(x1-1+48+16), y1, 10, l}, {(x1-1+48+24), y1, tem[3]-48, l}, {(x1-1+48+32), y1, tem[4]-48, l}
    };

    unsigned char x2 = 10;			//2行 X
    unsigned char y2 = 12+16+8;		//2行 Y

    // 湿度字符坐标组 [ xx.xx ] 
    unsigned char hns[5][4] = {
        {(x2-1+48), y2, hum[0]-48, l}, {(x2-1+48+8), y2, hum[1]-48, l}, {(x2-1+48+16), y2, 10, l}, {(x2-1+48+24), y2, hum[3]-48, l}, {(x2-1+48+32), y2, hum[4]-48, l}
    }; 

    unsigned char i;

    for(i=0; i<5; i++)
    {
        OLED_Word(tns[i][0], tns[i][1], FB[tns[i][2]], tns[i][3]); 
        OLED_Word(hns[i][0], hns[i][1], FB[hns[i][2]], hns[i][3]); 
    } 

}



static void AhtOledTask(void *arg)
{
    (void)arg;

    GpioInit();

    // I2C 管脚 初始化

    // 将13/14引脚 设置为 I2C 通信端口 AHT20 SSD1306 公用相同端口

    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_13, WIFI_IOT_IO_FUNC_GPIO_13_I2C0_SDA);
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_I2C0_SCL);

    WifiIotI2cIdx id = WIFI_IOT_I2C_IDX_0;  // 选择 使用 I2C 通道 1 = WIFI_IOT_I2C_IDX_0 2 = WIFI_IOT_I2C_IDX_1 
    unsigned int baudrate = 400*1000;       // I2C通信 波特率 400k 要看模块支持发送的频率范围

    if(I2cInit(id, baudrate) == WIFI_IOT_SUCCESS)
    {
        printf("[AHT20_Task] I2C Init OK !!! \r\n");
    }
    else
    {
        printf("[AHT20_Task] I2C Init NO !!! \r\n");
    }

    // ------------  模块的各种初始设置

    // OLED 初始化
    OLED_Init();

    // AHT20初始化
    if(AHT20_Status(3) != 1) // 如果[3] = 0 没有校准 进行初始化
    {
        AHT20_Init();    // 初始化
    }

    //清空显示数组
    OLED_Clear();

    OLED_Str();    //先设置固定的文字

    // -------------- aht20 开始测量   

    float ht[2] = {0}; //测量结果
    
    unsigned char t = 20;   // 测试20次 间隔 2秒 但是时间不准 偏快 可能是 CPU 160MHz 1次usleep等于 0.6us 而不是1us 

    while(--t > 0)
    {

        if(AHT20_Measure(ht))   //如果返回值为1 测量完成
        {

            OLED_Num(ht);   // 设置显示的数字

            OLED_Show();    // OLED 显示

        }

        usleep(1000*1000);  // 1秒 
        usleep(1000*1000);  // 1秒 
    }      

    while(1);

}


// 看网站教程吧 这里不赘述了
static void AHT_OLED_Test(void)
{
    osThreadAttr_t attr;

    attr.name = "AhtOledTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 4096;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)AhtOledTask, NULL, &attr) == NULL) {
        printf("[TEST] AHT OLED Task !\n");
    }
}

SYS_RUN(AHT_OLED_Test);