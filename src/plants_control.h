#include <Arduino.h>
#include <LCD_I2C.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <BME280I2C.h>

#define SERIAL_BAUD 115200
#define SLEEP_IN_US 300000000
#define HI EnvironmentCalculations::HeatIndex
#define SLP EnvironmentCalculations::EquivalentSeaLevelPressure
#define LIGHT_DET GPIO_NUM_34 // light sensor
#define POWER1 GPIO_NUM_25   // power circuit 1
#define POWER2 GPIO_NUM_26   // power circuit 2
#define REG GPIO_NUM_35
#define SCL GPIO_NUM_32
#define SDA GPIO_NUM_33

uint8_t temperature_icon[8] = {0x04, 0x0A, 0x0A, 0x0A, 0x0A, 0x1F, 0x1F, 0x0E}; //PROGMEM saves variable to flash & keeps dynamic memory free
uint8_t humidity_icon[8] = {0x04, 0x0E, 0x0E, 0x1F, 0x1F, 0x1F, 0x0E, 0x00};
uint8_t plus_minus_icon[8] = {0x00, 0x04, 0x0E, 0x04, 0x00, 0x0E, 0x00, 0x00};

LCD_I2C lcd(0x27); // Default address of most PCF8574 modules, change according
IPAddress local_IP(192, 168, 1, 11);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 1, 1);

WiFiClient espClient;
PubSubClient client(espClient);

BME280I2C::Settings settings(
    BME280::OSR_X8,
    BME280::OSR_X8,
    BME280::OSR_X8,
    BME280::Mode_Forced,
    BME280::StandbyTime_1000ms,
    BME280::Filter_Off,
    BME280::SpiEnable_False,
    BME280I2C::I2CAddr_0x76,
    BME280::CalcMethodFloat_True,
    0.0f
);
BME280I2C bme(settings);

const char* ssid = "ssid";
const char* password = "ssid_pwd";
const char* mqtt_server = "mqtt.server";

unsigned long lastMsg = 0;
uint8_t global_counter = 0;
uint8_t light_delay = 0;
uint16_t light_sensor_low = 500;
uint16_t light_sensor_high = 1800;
bool light_auto = true;
bool light_state = false;
uint16_t reg_voltage = 0;
uint16_t reg_power = 10;

double round2(double value);

void setup_wifi();

void reconnect_mqtt();

struct weatherData
{
    float temperature;
    float pressure;
    float humidity;
    float heatindex;
    uint16_t light;
};

struct weatherData readSensor();

void displayValues(struct weatherData dataToPrint, bool update_all);

void sendValues(struct weatherData dataToSend);

void plants_light();

void mqtt_callback(char* topic, byte* payload, unsigned int length);
