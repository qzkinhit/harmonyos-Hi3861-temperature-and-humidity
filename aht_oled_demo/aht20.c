/*
    AHT20
*/

#include "aht20.h"

#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#include "wifiiot_i2c.h"    // i2c 方法函数
#include "wifiiot_errno.h"  // i2c 状态信息

// 每个参数 都写在函数里 是为了方便理解阅读 最后做最终版 要尽量减少冗余操作

// i2c写入、读出操作； rw=0 写入 rw=1 读出； *buff 数据数组，读入就是指令集，返回就是空数组； leng 数组的长度 不可以为0； 
void AHT20_I2C_RW(unsigned char rw, unsigned char *buff, unsigned int leng)
{
    unsigned int state = 0; // I2C 运行 状态值 ，单列出来是为了方便作为返回值 做判断

    WifiIotI2cIdx id = WIFI_IOT_I2C_IDX_0;  //设置I2C使用的通道

    unsigned short writAddr = 0x70; // aht20 ((0x38<<1)|0x0)  写入地址
    unsigned short readAddr = 0x71; // aht20 ((0x38<<1)|0x1)  读出地址   
    
    WifiIotI2cData i2cData = {0};   // 参考 wiffiiot_i2c.h 的说明 位置在 \base\iot_hardware\interfaces\kits\wifiiot_lite

    if(rw == 0) // 写入
    {
        i2cData.sendBuf = buff;      //unsigned char*    发送 数据 指针
        i2cData.sendLen = leng;      //unsigned int      发送 数据 长度

        state = I2cWrite(id, writAddr, &i2cData);   //i2c写入方法 会有1个状态返回值 WIFI_IOT_SUCCESS = 0 代表成功 出错会返回错误代码 需要加入 wifiiot_errno.h 头文件        
    }
    else if(rw == 1) // 读出
    {
        i2cData.receiveBuf = buff;   //unsigned char*    接收 数据 指针
        i2cData.receiveLen = leng;   //unsigned int      接收 数据 长度

        state = I2cRead(id, readAddr, &i2cData);    //i2c读出方法       
    }

    if(state != WIFI_IOT_SUCCESS)   // 如果返回值 不等于 WIFI_IOT_SUCCESS 打印state 查询 wifiiot_errno.h 看错哪了
    {
        printf("[AHT20_I2C_RW] ERROR !!! %d ： %d \r\n", rw, state);    // 打印 错误信息
    }

}

// AHT20 初始化
void AHT20_Init(void)
{
    usleep(40*1000); // 首次开机 AHT20 等待 40ms  

    // 0xa8 进入 NOR 工作模式
    unsigned char buff1[] = {0xa8, 0x00, 0x00};
    unsigned char leng1 = 3;

    AHT20_I2C_RW(0, buff1, leng1);

    usleep(10*1000); //等待 10ms    

    // 0xbe 初始化
    unsigned char buff2[] = {0xbe, 0x08, 0x00};
    unsigned char leng2 = 3;

    AHT20_I2C_RW(0, buff2, leng2);

    usleep(10*1000); //等待 10ms

}

// 返回 AHT20 的状态值 i是第几位 0~7 
unsigned char AHT20_Status(unsigned char i)
{
    unsigned char buff[] = {0};
    unsigned char leng = 1;

    AHT20_I2C_RW(1, buff, leng);

    unsigned char s;    //返回状态值 0、1

    s = (buff[0] >> i) & 0x01;  //状态值是1个字节数据，我们只需要知道某位的具体值就行 i就是第几位

    //printf("[AHT20_Status] AHT20_Status 0x%x [%d : %d] !!! \r\n", buff4[0], i, s);    // 此行 调试用 打印 状态值

    return s;
}

// AHT20 测量温湿度值
unsigned char AHT20_Measure(float *ht)
{
    // 发送 测量指令
    unsigned char buff1[] = {0xac, 0x33, 0x00};
    unsigned char leng1 = 3;

    AHT20_I2C_RW(0, buff1, leng1);  // 默认状态值第[7]位是0,发送完测量指令后状态值[7]会置1

    usleep(80*1000); //等待 80ms 时钟偏快的 所以这个时间内 总是不能完成测量

    unsigned char t = 10;    //等待时间的值

    while(AHT20_Status(7) != 0) //检查 状态值第[7]位是否从1变为0，如果没变就等待5ms，如果已经置0说明测量完成
    {
        usleep(10*1000);   //10ms 这个时间不要设置太长，也不要设置太短，太长时间，小器件很难长时间存储测量结果，太短反复应答也能会影响测量的稳定性，一般情况等1次就会过 

        if(--t == 0)  // 如果等待时间很长依然没有变0，说明设备可能出现异常，为了避免死机，返回0，重新测量 这个情况我还没遇到
        {
            return 0;
        }
    }

    // 接收 测量结果
    unsigned char buff2[7] = {0};
    unsigned char leng2 = 7;

    AHT20_I2C_RW(1, buff2, leng2);  // 读返回结果，一共7个字节，第1个字节是状态值 最后1个字节是效验值

    unsigned char i, j;
    unsigned char crc = 0xFF;   // 效验 初值

    // CRC 效验 固定算法 
    for(i=0; i<6; i++)
	{
		crc ^= (buff2[i]);
		
		for(j=8; j>0; --j)
		{
			if(crc & 0x80)
			{
				crc = (crc << 1) ^ 0x31;
			}
			else
			{
				crc = (crc << 1);
			} 
    	}
  	}

    if(buff2[6] != crc) //CRC值不对 说明传输过程可能有干扰 出错了
    {
        //printf(" CRC8 NO \r\n");
        return 0;   // 效验不正确 回执1个错误信息
    }

    unsigned int dat1 = 0;  // 湿度
    unsigned int dat2 = 0;  // 温度

    dat1 = (dat1 | buff2[1]) << 8;
    dat1 = (dat1 | buff2[2]) << 8;
    dat1 = (dat1 | buff2[3]) >> 4;

    dat2 = (dat2 | buff2[3]) << 8;
    dat2 = (dat2 | buff2[4]) << 8;
    dat2 = (dat2 | buff2[5]) & 0xfffff;

    // 这1大段搬来搬去的 主要是因为 buff2[3] 前4位属于湿度 后4位属于 温度
    // 单片机处理能力有限，主要是针对寄存器值的处理，使用位运算，这样能节省算力
    // 处理数据 单列出来 便于理解，代码写的太简练 不容易看懂 最终 不需要这么繁琐

    float hum = 0;  // 温度
    float tem = 0;  // 湿度

    // 2^20=1048576 要先类型转换 暂时先这么写 以后再改得顺溜点
    hum = ((float)dat1 / (float)1048576) * (float)100;                 // 湿度
    tem = ((float)dat2 / (float)1048576) * (float)200 - (float)50;     // 温度

    ht[0] = hum;
    ht[1] = tem;

    return 1;   // 测量完毕

}

// AHT20 软复位 虽然没用到 手册上有说明了 就写上吧
void AHT20_Reset(void)
{
    unsigned char buff[] = {0xba};
    unsigned char leng = 1;

    AHT20_I2C_RW(0, buff, leng);

    usleep(20*1000);    //等待 20毫秒
}