#pragma once

/*
 * File Name: soc_clk_freq.h
 * Project: ESP32 System on Module
 */

//***************************** READ ME

//*  This file contains all the default speeds of the interface buses

//********************************************************     CPU SPEED     ********************************************************/
#define CPU_CLK_FREQUENCY_MHz

//********************************************************     BUS SPEEDS    ********************************************************/

//* UART
#define UART0_BAUD_RATE 115200

//* eMMC
#define eMMC_CLK_FREQUENCY_MHz

//* SPI
#define HSPI_CLCK_FREQUENCY 10000000
#define VSPI_CLCK_FREQUENCY 25000000

//* I2C0
#define I2C0_CLK_FREQUENCY 400000

//* I2C1
#define I2C1_CLK_FREQUENCY 400000

//* End.