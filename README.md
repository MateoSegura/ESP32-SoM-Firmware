# ESP32-SoM-Firmware

This project is a template for developing with the [ESP32 IoT System on Module](https://github.com/MateoSegura/ESP32-Internet-of-Things-SoM)

![alt text](https://github.com/MateoSegura/ESP32-SoM-Firmware/blob/master/images/som_class_overview.jpg)

The class *SystemOnModule* allows you to initalize all the hardware on the module with a single function call. After calling the begin method of this class, you are free to use any of the following objects in your application

``` C++

Terminal terminal;
SystemOnChip esp;
BluetoothLowEnergyServer bleServer;

AD7689 adc;
MCP2518FD can;
MPU9250 imu;
EMMC_Memory emmc;
SPIFFS_Memory spiffs;
RV3027 rtc;
SX1509 ioExpansion;

```

Refer to the [ESP32 Utilites](https://github.com/MateoSegura/ESP32-Utilities) repo for more information and examples 
