
/*

1.上电后要等待40ms，读取温湿度值之前， 首
先要看状态字的校准使能位Bit[3]是否为 1(通
过发送0x71可以获取一个字节的状态字)，如果
不为1，要发送0xBE命令(初始化)，此命令参数
有两个字节， 第一个字节为0x08，第二个字节
为0x00,然后等待10ms。

void AHT20_Init(void);
unsigned char AHT20_Status(unsigned char i);

2.直接发送 0xAC命令(触发测量)，此命令参数
有两个字节，第一个字节为 0x33，第二个字节
为0x00。

3.等待80ms待测量完成，如果读取状态字Bit[7]
为0，表示测量完成，然后可以连续读取六个字
节；否则继续等待。

4.当接收完六个字节后，紧接着下一个字节是
CRC校验数据，用户可以根据需要读出，如果接
收端需要CRC校验，则在接收完第六个字节后发
ACK应答，否则发NACK结束，CRC初始值为0XFF,
CRC8校验多项式为：

5.计算温湿度值。

unsigned char AHT20_Measure(unsigned float *ht);

用于在无需关闭和再次打
开电源的情况下，重新启动传感器系统。在接
收到这个命令之后，传感器系统开始重新初始
化，并恢复默认设置状态，软复位所需时间不
超过20 毫秒。

void AHT20_Reset(void);

*/

#ifndef _AHT20_H_
#define _AHT20_H_

// i2c写入、读出操作， rw为读写状态 rw=0 写入 rw=1 读出； *buff 数据数组，读入就是指令集，返回就是空数组； leng 数组的长度 不可以为0； 
void AHT20_I2C_RW(unsigned char rw, unsigned char *buff, unsigned int leng);


// AHT20 初始化
void AHT20_Init(void);

// 返回 AHT20 的状态值 i是第几位 0~7 返回状态值第i位的值，结果1/0
unsigned char AHT20_Status(unsigned char i);

// AHT20 测量温湿度值 输入1个2位的浮点数组，用于接收测量结果，返回值1/0，1为测量完成，0为有错误
unsigned char AHT20_Measure(float *ht);

// AHT20 软复位 在不掉电的情况下长时间没有使用，从新启动设备
void AHT20_Reset(void);

#endif