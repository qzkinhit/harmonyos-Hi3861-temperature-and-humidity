# harmonyos-Hi3861-temperature-and-humidity
基于harmonyos的Hi3861开发—测温和湿度装置
由于需要使用wifiiot_gpio.h，wifiiot_gpio_ex.h，wifiiot_i2c.h，wifiiot_errno.h这几个头文件，因此本项目是harmony1.0版本（1.1版本把这几个头文件删了）。
code1.0的gitee地址：https://repo.huaweicloud.com/harmonyos/os/1.0/code-1.0.tar.gz

文件中的bin文件是build后的文件，可以直接烧录。



编译时使用python build.py wifiiot命令进行编译，如果编译报错，是因为默认情况下，hi3861_sdk中，PWM的CONFIG和I2C的CONFIG选项没有打开。

- **解决：修改`vendor\hisi\hi3861\hi3861\build\config\usr_config.mk`文件中的`CONFIG_I2C_SUPPORT`行和CONFIG_PWM_SUPPORT行：

# CONFIG_PWM_SUPPORT is not set`修改为`CONFIG_PWM_SUPPORT=y`

# CONFIG_I2C_SUPPORT is not set`修改为`CONFIG_I2C_SUPPORT=y`



项目文件在applications\sample\wifi-iot\app\aht_oled_demo中，功能是实时显示当前环境的湿度和温度到led屏幕上，所以需要温度传感器组件和led屏幕组件。
