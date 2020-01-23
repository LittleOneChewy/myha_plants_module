#include <Arduino.h>
#include <FreeRTOS.h>
#include <task.h>
#include <SPI.h>    // needed to build BME module
#include <Wire.h>
#include <BME280I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EnvironmentCalculations.h>

#define I2C_1_SDA PB7
#define I2C_1_SCL PB6
#define I2C_2_SDA PB11
#define I2C_2_SCL PB10
#define MOVE_DET PA15   // movement sensor
#define POWER1 PB3   // power circuit 1
#define POWER2 PB4   // power circuit 2


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET 255

#define HI EnvironmentCalculations::HeatIndex
#define SLP EnvironmentCalculations::EquivalentSeaLevelPressure

TwoWire Wire1(I2C_2_SDA, I2C_2_SCL);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire1, OLED_RESET);
BME280I2C bme;
EnvironmentCalculations::TempUnit envTempUnit = EnvironmentCalculations::TempUnit_Celsius;

float temp = 0.0;
float hum = 0.0;
float pres = 0.0;
float hidx = 0.0;
bool temp_updated = false;

void get_sensor_data(void *args)
{
  while (true)
  {
    temp = bme.temp();
    hum = bme.hum();
    pres = SLP(213.0, temp, bme.pres());
    hidx = HI(temp, hum);
    temp_updated = true;
    vTaskDelay(600 * configTICK_RATE_HZ);
  }
}

void setup()
{
  Wire.setSCL(I2C_1_SCL);
  Wire.setSDA(I2C_1_SDA);
  Wire.begin();
  Wire1.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  display2.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  // test images
  // display.display();
  // display2.display();
  // delay(1000);
  display.clearDisplay();
  display.display();
  display2.clearDisplay();
  display2.display();
  display.setTextSize(1);
  display2.setTextSize(1);
  display.setTextColor(WHITE);
  display2.setTextColor(WHITE);
  display.println("Init peripherals...");
  display.display();
  delay(500);
  if (!bme.begin())
  {
    display.println("Failed to initialize BME280 :(");
    display.display();
  }
  display.println("BME280 initialized.");
  display.display();
  delay(500);
  pinMode(PC13, OUTPUT);
  pinMode(MOVE_DET, INPUT);
  pinMode(POWER1, OUTPUT_OPEN_DRAIN);
  pinMode(POWER2, OUTPUT_OPEN_DRAIN);
  display.println("Ports configured.");
  display.display();
  delay(500);
  display.println("Creating threads:");
  display.println("1. Sensor data.");
  display.display();
  xTaskCreate(get_sensor_data, "sensor data", configMINIMAL_STACK_SIZE + 256, NULL, tskIDLE_PRIORITY + 1, NULL);
  delay(500);
  display.println("Starting main loop...");
  display.display();
  delay(1000);
  display.println("Started :)");
  display.display();
  delay(1000);

  
}

void loop()
{
  // digitalWrite(PC13, HIGH);
  // delay(1000);
  // digitalWrite(PC13, LOW);
  // delay(1000);

  digitalWrite(PC13, !digitalRead(PC13));
  delay(1000);
}
