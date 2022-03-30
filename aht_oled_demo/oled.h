#ifndef _OLED_H_
#define _OLED_H_

// I2C 写入 
void SSD1306_I2C_W(unsigned char cd, unsigned char byt);

// SSD1306 初始化
void OLED_Init(void);

// OLED 显示信息
void OLED_Show(void);

//清空显示数组
void OLED_Clear(void);

//在某个任意坐标点 输入文字 横坐标x: 0~127 竖坐标y: 0~63 *c 文字显示阵列数组，默认高度都是16，为什么，因为好算，l是文字的宽度，汉字宽16，数字宽8 
void OLED_Word(unsigned char x, unsigned char y, unsigned char *c, unsigned char l); 

#endif