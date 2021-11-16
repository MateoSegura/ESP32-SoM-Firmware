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
#include <Adafruit_NeoPixel.h>

#define APP_NAME "Test App"

#define UART0_BAUD_RATE 115200         // Set the baudrate of the terminal uart port
#define MICROS_TIMESTAMP_ENABLED false // Set to true to enabled microsecond time stamp in terminal messages
#define SYSTEM_TIME_ENABLED true       // Set to true if using a real time clock. Refer to "rtc_example.cpp" for more

#define NUMPIXELS 2

// BLE Service
#define BLE_NAME "Bluetooth Server Example"
#define SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

SystemOnChip esp;
Terminal terminal;
EMMC_Memory emmc;
BluetoothLowEnergyServer bleServer;

RealTimeClock rtc;
SX1509 io_expansion;
AD7689 adc;
ACAN2517FD can(CAN0_CONTROLLER_CS_PIN, esp.hspi, CAN0_CONTROLLER_INT_PIN);
Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

class SystemOnModule
{
public:
  DateTime system_time;

  void onBootError();

  void initAll(bool debug_enabled);

  bool initIOExpansion();
  bool initRTC();
  bool initADC();
  bool initCAN();
  bool initEMMC();
  bool initLED();

  bool initBLE();

private:
  bool debugging_enabled;

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

//****************** Tasks
QueueHandle_t io_expansion_interrupt_queue = NULL;
uint16_t io_expansion_interrupt_queue_length = 10;

void handleIO_ExpInterrupt(void *parameters);

//****************** Interrupts
void IRAM_ATTR updateSystemTime()
{
  rtc.updateMillisecondsCounter(SoM.system_time);
}

bool request = false;

void IRAM_ATTR ioExpansionInterrupt()
{
  xQueueSend(io_expansion_interrupt_queue, (void *)&request, 0);
}

//****************** Setup
void setup()
{
  esp.uart0.begin(UART0_BAUD_RATE);

  terminal.begin(esp.uart0, MICROS_TIMESTAMP_ENABLED, SYSTEM_TIME_ENABLED);
  terminal.printBanner(APP_NAME);

  SoM.initAll(true);
}

//********************LOOP
void loop()
{
  delay(500);
  pixels.setPixelColor(0, pixels.Color(10, 10, 10));
  pixels.show();
  delay(500);
  pixels.setPixelColor(0, pixels.Color(0, 0, 0));
  pixels.show();
}

//******************** Tasks Definition
void handleIO_ExpInterrupt(void *parameters)
{
  bool action;
  while (1)
  {
    // Read if available first
    if (xQueueReceive(io_expansion_interrupt_queue, (void *)&action, portMAX_DELAY) == pdTRUE)
    {
      long initial_time = micros();
      int interrupt_pin = io_expansion.interruptSource(true);

      terminal.printMessage(TerminalMessage("Pin: " + String(interrupt_pin),
                                            "I/O", INFO, micros(), micros() - initial_time));

      attachInterrupt(IO_EXPANSION_INT_PIN, ioExpansionInterrupt, FALLING);
    }
  }
}

//******************** SoM Function definitions
void SystemOnModule::initAll(bool debug_enabled)
{
  long initial_time = millis();
  debugging_enabled = debug_enabled;

  if (!initIOExpansion())
    onBootError();

  if (!initRTC())
    onBootError();

  if (!initADC())
    onBootError();

  if (!initCAN())
    onBootError();

  if (!initEMMC())
    onBootError();

  initLED();

  if (debug_enabled)
  {
    terminal.printMessage(TerminalMessage("Boot time: " + String(millis() - initial_time) + " mS", "SOM", INFO, micros()));
  }
}

void SystemOnModule::onBootError()
{
  SoM.initLED();
  pixels.setPixelColor(0, pixels.Color(100, 0, 0));
  pixels.show();
  terminal.println("\n\nStopping boot");
  while (1)
    ;
}

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

  io_expansion.pinMode(V3V3_OUT_PIN_A, OUTPUT);
  io_expansion.digitalWrite(V3V3_OUT_PIN_A, HIGH);
  io_expansion.pinMode(V3V3_OUT_PIN_B, OUTPUT);
  io_expansion.digitalWrite(V3V3_OUT_PIN_B, HIGH); // 3.3V Out

  // 3. Setup GPIOs
  io_expansion.pinMode(V5V_EN_PIN, OUTPUT);
  io_expansion.digitalWrite(V5V_EN_PIN, HIGH); // 5V Output

  if (debugging_enabled)
  {
    io_expansion.pinMode(GND_DEBUG_PIN, OUTPUT);
    io_expansion.digitalWrite(GND_DEBUG_PIN, LOW); // GND Debug

    io_expansion.pinMode(V3V3_DEBUG_PIN, OUTPUT);
    io_expansion.digitalWrite(V3V3_DEBUG_PIN, HIGH); // 3.3V Debug
  }

  io_expansion.pinMode(ADC0_EN_PIN, OUTPUT);
  io_expansion.digitalWrite(ADC0_EN_PIN, HIGH); // ADC

  io_expansion.pinMode(CAN0_EN_PIN, OUTPUT);
  io_expansion.digitalWrite(CAN0_EN_PIN, LOW); // CAN

  io_expansion.pinMode(eMMC0_CD_PIN, INPUT_PULLUP);
  io_expansion.enableInterrupt(eMMC0_CD_PIN, FALLING);

  io_expansion_interrupt_queue = xQueueCreate(io_expansion_interrupt_queue_length, sizeof(bool));

  xTaskCreatePinnedToCore(
      handleIO_ExpInterrupt,
      "io handle",
      3000,
      nullptr,
      25,
      nullptr,
      1);

  pinMode(IO_EXPANSION_INT_PIN, INPUT);
  attachInterrupt(IO_EXPANSION_INT_PIN, ioExpansionInterrupt, FALLING);

  if (debugging_enabled)
  {
    terminal.printMessage(TerminalMessage("External I/O initialized", "I/O", INFO, micros(),
                                          micros() - initial_time));
  }

  return true;
}

bool SystemOnModule::initRTC()
{
  long initial_time = micros();

  ESP_ERROR initialize_rtc = rtc.begin(RealTimeClock::RV3028_IC, esp.i2c0);

  if (initialize_rtc.on_error)
  {
    terminal.printMessage(TerminalMessage(initialize_rtc.debug_message, "RTC", ERROR, micros()));
    return false;
  }

  if (debugging_enabled)
  {
    terminal.printMessage(TerminalMessage("RTC initialized", "RTC", INFO, micros(),
                                          micros() - initial_time));
  }

  rtc.setToCompilerTime();

  system_time = rtc.getTime();

  terminal.setTimeKeeper(system_time);

  esp.timer0.setup();
  esp.timer0.attachInterrupt(updateSystemTime);
  esp.timer0.timerPeriodMilliseconds(1);
  esp.timer0.enableInterrupt();

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

  if (debugging_enabled)
  {
    terminal.printMessage(TerminalMessage("ADC initialized", "ADC", INFO, micros(),
                                          micros() - initial_time));
  }

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

  if (debugging_enabled)
  {
    terminal.printMessage(TerminalMessage("CAN controller initialized", "CAN", INFO, micros(),
                                          micros() - initial_time));
  }

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

  if (debugging_enabled)
  {
    terminal.printMessage(TerminalMessage("eMMC initialized", "MMC", INFO, micros(),
                                          micros() - initial_time)); // How long did SD card take to init
  }

  return true;
}

bool SystemOnModule::initLED()
{
  pixels.begin();
  pixels.clear();
  pixels.show();
}

bool SystemOnModule::initBLE()
{
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
    return false;
  }
  else
  {
    terminal.printMessage(TerminalMessage("MTU size set to 185 bytes. Max throughput is ~ 11KB/s", "MMC", INFO, micros()));
  }

  return true;
}