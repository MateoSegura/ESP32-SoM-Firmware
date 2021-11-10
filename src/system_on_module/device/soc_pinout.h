#pragma once

/*
 * File Name: soc_pinout.h
 * Project: ESP32 System on Module
 */

//***************************** READ ME

//*  This is the pinout of all the systems on the chip, such as interface buses, general purpose I/O, GPIO expansion I/O, etc.
//*  Baud rates & bus clock frequencies are in "soc_settings.h"

//***************************** ESP32 PINS

//* UART0
#define UART0_TX 1
#define UART0_RX 3

//* I2C0
#define I2C0_SDA 21
#define I2C0_SCL 22

//* I2C1
#define I2C1_SDA 4
#define I2C1_SCL 5

//* HSPI
#define HSPI_MOSI_PIN 25
#define HSPI_MISO_PIN 17
#define HSPI_CLCK_PIN 16

//* CAN FD Controller
#define CAN0_CONTROLLER_CS_PIN 27  // MCP2518 is connected to this HSPI bus
#define CAN0_CONTROLLER_INT_PIN 36 // Active Low. Pull up resistor on module

//* VSPI
#define VSPI_MOSI_PIN 23
#define VSPI_MISO_PIN 19
#define VSPI_CLCK_PIN 18

//* Analog to Digital Converter
#define ADC0_CS 33 // AD7689A is connected to this VSPI bus

//* Real Time Clock
#define RTC0_INT 39 // Active Low. Pull up resistor on module

//* Internal Motion Unit
#define IMU0_INT 13 // Populate R12 & remove R11 to enable

//* GPIO Expansion
#define IO_EXPANSION_INT_PIN 35 // Active Low. Pull up resistor on module
#define IO_EXPANSION_RST_PIN 13 // Populate R11 & remove R12 to enable

//* General Purpose I/O mapped out to user

#define GPIO12 12 // Input, Output. DO NOT PULL HIGH. Boot fail if high after reset
#define GPIO13 13 // Input, Output
#define GPIO26 26 // Input, Output66
#define GPIO27 27 // Input, Output
#define GPIO34 34 // Input Only

//***************************** GPIO EXPANSION PINS

//* EMMC Memory
#define eMMC0_EN_PIN -1
#define eMMC0_CD_PIN -1

//* End.