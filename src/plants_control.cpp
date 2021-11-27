#include <Wire.h>
#include <SPI.h> // needed to build BME module
#include <EnvironmentCalculations.h>
#include <ArduinoJson.h>
#include <plants_control.h>

double round2(double value)
{
    return (int)(value * 100 + 0.5) / 100.0;
}

void setup_wifi()
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    WiFi.config(local_IP, gateway, subnet, primaryDNS);
    Serial.print("Connecting WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" connected");
    randomSeed(micros());
}

void reconnect_mqtt()
{
    while (!client.connected())
    {
        String clientId = "IntKitchen1-";
        clientId += String(random(0xffff), HEX);
        if (!client.connect(clientId.c_str()))
        {
            delay(5000);
        }
        client.subscribe("int/kitchen1/on/");
        client.subscribe("int/kitchen1/off/");
        client.subscribe("int/kitchen1/toggle/");
    }
}

struct weatherData measurement;

struct weatherData readSensor()
{
    uint16_t light_sensor_data = 0;

    if (global_counter % 60 == 0)
    {
        measurement.temperature = bme.temp();
        measurement.pressure = SLP(213.0f, measurement.temperature, bme.pres());
        measurement.humidity = bme.hum();
        measurement.heatindex = HI(measurement.temperature, measurement.humidity);
    }
    for (int i = 0; i < 10; i++) {
        light_sensor_data = light_sensor_data + analogRead(LIGHT_DET);
    }
    measurement.light = int(light_sensor_data * 0.1);
    return measurement;
}

void displayValues(struct weatherData newValues, bool update_all = false)
{
    if (update_all)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.write(0);
        lcd.print(newValues.temperature);
        lcd.setCursor(10, 0);
        lcd.write(1);
        lcd.print(newValues.humidity);
        lcd.print(" ");
        lcd.setCursor(0, 1);
        lcd.write(2);
        lcd.print(newValues.pressure);
    }
    else
    {
        lcd.setCursor(10, 1);
        lcd.print("    ");
        lcd.setCursor(10, 1);
        // lcd.print(newValues.light);
        lcd.print(reg_power);
    }
}

void sendValues(struct weatherData dataToSend)
{
    StaticJsonDocument<256> JSONbuffer;
    JSONbuffer["temp"] = round2(dataToSend.temperature);
    JSONbuffer["hidx"] = round2(dataToSend.heatindex);
    JSONbuffer["hum"] = round2(dataToSend.humidity);
    JSONbuffer["pres"] = round2(dataToSend.pressure);
    JSONbuffer["lht"] = round2(dataToSend.light);
    char out[128];
    serializeJson(JSONbuffer, out);
    client.publish("int/sensor1", out);
}

void plants_light()
{
    if (measurement.light <= light_sensor_low && light_delay > 90 && light_auto == true)
    {
        // digitalWrite(POWER1, LOW);
        light_state = true;
        lcd.noBacklight();
    }
    else if (measurement.light >= light_sensor_high && light_auto == true)
    {
        // digitalWrite(POWER1, HIGH);
        light_state = false;
        lcd.backlight();
        light_delay = 0;
    }
}

void mqtt_callback(char *topic, byte *payload, unsigned int length)
{
    payload[length] = '\0';
    // char payload_str[length];
    // strncpy(payload_str, (char *) payload, length);
    Serial.println((char *)payload);
    if (strcmp(topic, "int/kitchen1/off/") == 0)
    {
        light_sensor_high = (uint16_t)atoi((char *)payload);
        Serial.print("light_sensor_high: ");
        Serial.println(light_sensor_high);
    }
    if (strcmp(topic, "int/kitchen1/on/") == 0)
    {
        light_sensor_low = (uint16_t)atoi((char *)payload);
        Serial.print("light_sensor_low: ");
        Serial.println(light_sensor_low);
    }
    if (strcmp(topic, "int/kitchen1/toggle/") == 0)
    {
        if ((char)payload[0] == 'O' && (char)payload[1] == 'n')
        {
            light_auto = false;
            // digitalWrite(POWER1, LOW);
            light_state = true;
            Serial.println("Light on");
        }
        else if ((char)payload[0] == 'O' && (char)payload[1] == 'f')
        {
            light_auto = false;
            // digitalWrite(POWER1, HIGH);
            light_state = false;
            Serial.println("Light off");
        }
        else
        {
            light_auto = true;
            Serial.println("Light Auto");
        }
    }
}

void measure_voltage(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        reg_voltage = analogRead(REG);
        if (3820 < reg_voltage && reg_voltage < 3870 && light_state){
            ;
        }
        else if (reg_voltage < 3820 && reg_power > 10 && light_state) {
            reg_power--;
            ledcWrite(0, reg_power);
        }
        else if (reg_voltage > 3870 && reg_power < 125 && light_state) {
            reg_power++;
            ledcWrite(0, reg_power);
        }
        if (reg_voltage < 3870 && !light_state) {
            ledcWrite(0, 0);
        }
        vTaskDelay(1);
    }
}

void mqtt_client(void *pvParameters)
{
    (void)pvParameters;
    for (;;)
    {
        if (!client.connected())
        {
            reconnect_mqtt();
        }
        client.loop();
        vTaskDelay(1);
    }
}

void setup()
{
    pinMode(LIGHT_DET, INPUT);
    pinMode(REG, INPUT);
    adcAttachPin(REG);
    pinMode(POWER1, OUTPUT);
    digitalWrite(POWER1, LOW);
    pinMode(POWER2, OUTPUT);
    digitalWrite(POWER2, LOW);

    ledcSetup(0, 300000, 8);
    ledcAttachPin(POWER1, 0);
    ledcWrite(0, 0);
    xTaskCreate(measure_voltage, "REG voltage", configIDLE_TASK_STACK_SIZE + 128, NULL, tskIDLE_PRIORITY + 1, NULL);

    Serial.begin(SERIAL_BAUD);
    delay(100);
    Wire.begin(SDA, SCL);
    delay(100);
    lcd.begin();
    delay(100);
    lcd.backlight();
    lcd.createChar(0, temperature_icon);
    lcd.createChar(1, humidity_icon);
    lcd.createChar(2, plus_minus_icon);
    setup_wifi();

    Serial.print("Connecting BME");
    Serial.print("...");
    while (!bme.begin())
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println(" connected");
    bme.temp();
    client.setServer(mqtt_server, 1883);
    client.setCallback(mqtt_callback);
    delay(1000);
    xTaskCreate(mqtt_client, "MQTT Client", configIDLE_TASK_STACK_SIZE + 1024, NULL, tskIDLE_PRIORITY + 1, NULL);
}

void loop()
{
    // Serial.print("Reg voltage: ");
    // Serial.print(reg_voltage);
    // Serial.print(" Reg power: ");
    // Serial.println(reg_power);
    // Serial.println();
    const struct weatherData newValues = readSensor();
    // Serial.println("Light");
    plants_light();

    if (global_counter % 60 == 0)
    {
        // Serial.println("MQTT");
        displayValues(newValues, true);
        sendValues(newValues);
    }
    if (global_counter % 3 == 0)
    {
        // Serial.println("Display");
        displayValues(newValues);
    }

    light_delay++;
    global_counter++;
    if (global_counter == 240)
    {
        global_counter = 0;
    }

    if (light_delay == 240)
    {
        light_delay = 90;
    }
    //     Serial.println("Done");
    //     Serial.println();
    delay(1000);
}
