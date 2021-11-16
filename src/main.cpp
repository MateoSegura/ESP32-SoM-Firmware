/*
 * File Name: soc_example.cpp
 * Hardware needed: No extra hardware needed, just a good ol' ESP32 dev. board
 *
 * Copyright 2021, Mateo Segura, All rights reserved.
 */

#include <Arduino.h>
#include <esp32_utilities.h>
#include <system_on_module/settings/soc_pinout.h>
#include <system_on_module/settings/soc_clk_freq.h>
#include <ACAN2517FD.h>

#define UART0_BAUD_RATE 115200         // Set the baudrate of the terminal uart port
#define MICROS_TIMESTAMP_ENABLED false // Set to true to enabled microsecond time stamp in terminal messages
#define SYSTEM_TIME_ENABLED false      // Set to true if using a real time clock. Refer to "rtc_example.cpp" for more

// BLE Service
#define BLE_NAME "Bluetooth Server Example"
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

SystemOnChip esp;
Terminal terminal;
EMMC_Memory emmc;
BluetoothLowEnergyServer bleServer;

SX1509 io_expansion;
AD7689 adc;
ACAN2517FD can(CAN0_CONTROLLER_CS_PIN, esp.hspi, CAN0_CONTROLLER_INT_PIN);

class SystemOnModule
{
public:
  bool initIOExpansion();
  bool initADC();
  bool initCAN();
  bool initEMMC();

} SoM;

class MyServerCallbacks : public BLEServerCallbacks
{
  // -- On client connect
  void onConnect(BLEServer *pServer)
  {
    bleServer.onClientConnect();
    terminal.printMessage(TerminalMessage("Client connected", "BLE", INFO, micros()));
  };

  // -- On client disconnect
  void onDisconnect(BLEServer *pServer)
  {
    bleServer.onClientDisconnect();
    terminal.printMessage(TerminalMessage("Client disconnected", "BLE", INFO, micros()));

    delay(200); // Allow some time for the server to stat advertising again
    bleServer.startAdvertising();
    terminal.printMessage(TerminalMessage("Bluetooth server is advertising", "BLE", INFO, micros()));
  }
} ServerCallbacks;

class MyCallbacks : public BLECharacteristicCallbacks
{
  void onWrite(BLECharacteristic *pCharacteristic)
  {
    std::string rxValue = pCharacteristic->getValue();

    String incoming_bluetooth_message = "Incoming data (size: ";
    incoming_bluetooth_message += rxValue.length();
    incoming_bluetooth_message += ") -> ";

    // Store incoming message in string
    if (rxValue.length() > 0)
    {
      for (int i = 0; i < rxValue.length(); i++)
      {
        incoming_bluetooth_message += rxValue[i];
        // TODO: Handle your incoming data as you wish here
      }
      terminal.printMessage(TerminalMessage(incoming_bluetooth_message, "BLE", INFO, micros()));
    }
  }

} CharacteristicCallbacks;

//****************** Setup
void setup()
{
  // 1. Init UART0(Serial)
  esp.uart0.begin(UART0_BAUD_RATE);

  // 2. Init Terminal
  terminal.begin(esp.uart0, MICROS_TIMESTAMP_ENABLED, SYSTEM_TIME_ENABLED);
  terminal.printBanner("Test App");

  // 3. Init IO expansion
  if (!SoM.initIOExpansion())
  {
    esp.uart0.println("\n\nStopping boot\n\n");
    while (1)
      ;
  }

  // 4. Init ADC
  if (!SoM.initADC())
  {
    esp.uart0.println("\n\nStopping boot\n\n");
    while (1)
      ;
  }

  // 3. Init CAN
  if (!SoM.initCAN())
  {
    esp.uart0.println("\n\nStopping boot\n\n");
    while (1)
      ;
  }

  // 3. Init eMMC
  if (!SoM.initEMMC())
  {
    esp.uart0.println("\n\nStopping boot\n\n");
    while (1)
      ;
  }

  // 3. Init bluetooth server
  long initial_time = micros();
  bleServer.begin(BLE_NAME,
                  SERVICE_UUID,
                  CHARACTERISTIC_UUID,
                  &ServerCallbacks,
                  &CharacteristicCallbacks);

  bleServer.startAdvertising();

  terminal.printMessage(TerminalMessage("Bluetooth server is advertising", "BLE", INFO, micros(), micros() - initial_time));

  // 4. Set MTU size. Max iOS size is 185 bytes (11KB/s)
  ESP_ERROR mtu_set = bleServer.setMaxMTUsize(185); // -- Try to negotiate Max size MTU (iOS max. MTU is 185 bytes)

  if (mtu_set.on_error) // Catch error
  {
    terminal.printMessage(TerminalMessage(mtu_set.debug_message, "MMC", ERROR, micros()));
  }
  else
  {
    terminal.printMessage(TerminalMessage("MTU size set to 185 bytes. Max throughput is ~ 11KB/s", "MMC", INFO, micros()));
  }
}

//********************LOOP
void loop()
{
}

//******************** SoM Function definitions

bool SystemOnModule::initIOExpansion()
{
  long initial_time = micros();

  // 1. Begin I2C bus
  esp.i2c0.begin(I2C0_SDA_PIN, I2C0_SCL_PIN, I2C0_CLK_FREQUENCY); // Refer to soc_example.cpp for information on this function

  // 2. Begin IO expansion SX1509
  ESP_ERROR initialize_io_expansion = io_expansion.begin(&esp.i2c0, IO_EXP_ADDRESS);

  if (initialize_io_expansion.on_error) // Catch error
  {
    terminal.printMessage(TerminalMessage(initialize_io_expansion.debug_message, "I/O", ERROR, micros()));
    return false;
  }

  terminal.printMessage(TerminalMessage("External I/O initialized", "I/O", INFO, micros(),
                                        micros() - initial_time));

  // 3. Setup GPIOs
  io_expansion.pinMode(V5V_EN_PIN, OUTPUT);
  io_expansion.digitalWrite(V5V_EN_PIN, HIGH); // 5V Output

  io_expansion.pinMode(GND_DEBUG_PIN, OUTPUT);
  io_expansion.digitalWrite(GND_DEBUG_PIN, LOW); // GND Debug

  io_expansion.pinMode(V3V3_DEBUG_PIN, OUTPUT);
  io_expansion.digitalWrite(V3V3_DEBUG_PIN, HIGH); // 3.3V Debug

  io_expansion.pinMode(V3V3_OUT_PIN_A, OUTPUT);
  io_expansion.digitalWrite(V3V3_OUT_PIN_A, HIGH);
  io_expansion.pinMode(V3V3_OUT_PIN_B, OUTPUT);
  io_expansion.digitalWrite(V3V3_OUT_PIN_B, HIGH); // 3.3V Out

  io_expansion.pinMode(ADC0_EN_PIN, OUTPUT);
  io_expansion.digitalWrite(ADC0_EN_PIN, HIGH); // ADC

  io_expansion.pinMode(CAN0_EN_PIN, OUTPUT);
  io_expansion.digitalWrite(CAN0_EN_PIN, LOW); // CAN

  return true;
}

bool SystemOnModule::initADC()
{
  long initial_time = micros();

  // 1. Begin SPI bus
  esp.vspi.begin(VSPI_CLCK_PIN, VSPI_MISO_PIN, VSPI_MOSI_PIN);
  esp.vspi.setFrequency(VSPI_CLCK_FREQUENCY);

  // 2. Begin ADC
  adc.begin(ADC0_CS, esp.vspi, VSPI_CLCK_FREQUENCY);

  // 3. Run self test
  if (adc.selftest() == false)
  {
    terminal.printMessage(TerminalMessage("Error initializing ADC", "ADC", ERROR, micros()));
    return false;
  }

  terminal.printMessage(TerminalMessage("ADC initialized", "ADC", INFO, micros(),
                                        micros() - initial_time));
  return true;
}

bool SystemOnModule::initCAN()
{
  long initial_time = micros();

  // 1. Init SPI bus
  esp.hspi.begin(HSPI_CLCK_PIN, HSPI_MISO_PIN, HSPI_MOSI_PIN);

  // 2. Set CAN settings
  ACAN2517FDSettings settings(ACAN2517FDSettings::OSC_40MHz, 1000 * 1000, DataBitRateFactor::x1);
  settings.mDriverReceiveFIFOSize = 200;

  // 3. Attempt to initialize
  const uint32_t error_code = can.begin(settings, []
                                        { can.isr(); });

  if (error_code != 0)
  {
    terminal.printMessage(TerminalMessage("Could not initialize CAN controller. Error code: " + String(error_code),
                                          "CAN", ERROR, micros()));
    return false;
  }

  terminal.printMessage(TerminalMessage("CAN controller initialized", "CAN", INFO, micros(),
                                        micros() - initial_time));

  return true;
}

bool SystemOnModule::initEMMC()
{
  long initial_time = micros();
  ESP_ERROR initialize_emmc = emmc.begin(-1, -1, eMMC_MODE, MODE_1_BIT);

  if (initialize_emmc.on_error)
  {
    terminal.printMessage(TerminalMessage(initialize_emmc.debug_message, "MMC", ERROR, micros()));
    return false;
  }

  terminal.printMessage(TerminalMessage("eMMC initialized", "MMC", INFO, micros(),
                                        micros() - initial_time)); // How long did SD card take to init

  return true;
}
