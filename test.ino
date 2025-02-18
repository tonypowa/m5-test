// Written for Grafana Labs to demonstrate how to use the M5 ENVIV Unit with Grafana Cloud
// Updated for SHT40 and BMP280

#include "M5Unified.h"
#include "M5UnitENV.h"          // SHT40 (Temp, Humidity, Pressure)
#include "Adafruit_BMP280.h"    // BMP280 Pressure sensor


// ===================================================
// All the things that need to be changed 
// Your local WiFi details
// Your Grafana Cloud details
// ===================================================
#include "config.h"
#include "utility.h"

// ===================================================
// Includes - Needed to write Prometheus or Loki metrics/logs to Grafana Cloud
// No need to change anything here
// ===================================================
#include "certificates.h"
#include <PromLokiTransport.h>
#include <PrometheusArduino.h>

// ===================================================
// Global Variables
// ===================================================
SHT40 sht40;                // temperature and humidity sensor (SHT40)
Adafruit_BMP280 bmp280;     // BMP280 pressure sensor
//Adafruit_SGP30 sgp;         // VOC/CO2 sensor (commented out)
//M5_DLight dlight;           // Light sensor (commented out)

float temp     = 0.0;
float hum      = 0.0;
float pressure = 0.0;

int voc      = 0;
int co2      = 0;

uint16_t lux      = 0;

// Client for Prometheus metrics
PromLokiTransport transport;
PromClient client(transport);

// Create a write request for 6 time series (commented out some sensors)
WriteRequest req(6, 2048);  // Adjusted to 6 since other sensors are commented out

// Define all our timeseries
TimeSeries ts_m5stick_temperature(1, "m5stick_temp", PROM_LABELS);
TimeSeries ts_m5stick_humidity(1, "m5stick_hum", PROM_LABELS);
TimeSeries ts_m5stick_pressure(1, "m5stick_pressure", PROM_LABELS);
TimeSeries ts_m5stick_bat_volt(1, "m5stick_bat_volt", PROM_LABELS);
TimeSeries ts_m5stick_bat_current(1, "m5stick_bat_current", PROM_LABELS);
TimeSeries ts_m5stick_bat_level(1, "m5stick_bat_level", PROM_LABELS);
// TimeSeries ts_m5stick_voc(1, "m5stick_voc", PROM_LABELS); // Commented out
// TimeSeries ts_m5stick_co2(1, "m5stick_co2", PROM_LABELS); // Commented out
// TimeSeries ts_m5stick_lux(1, "m5stick_lux", PROM_LABELS); // Commented out

void setup() {
    Serial.begin(115200);
    Serial.println("Booting up!");

    auto cfg = M5.config();
    M5.begin(cfg);

    M5.Display.setRotation(3);
    M5.Display.setTextSize(2);
    M5.Display.setCursor(10, 10);
    M5.Display.setTextColor(ORANGE, BLACK);
    M5.Display.printf("== Grafana Labs ==");
    M5.Display.setTextColor(WHITE, BLACK);

    Wire.begin();       // Wire init, adding the I2C bus.
  
    // Initialize SHT40 (Temperature and Humidity Sensor)
    if (!sht40.begin()) {
        Serial.println("Couldn't find SHT40 sensor!");
    }

    // Initialize BMP280 (Pressure Sensor)
    if (!bmp280.begin()) {
        Serial.println("Couldn't find BMP280 sensor!");
    }

    // Initialize the Lux sensor (commented out)
    //dlight.begin();
    //dlight.setMode(CONTINUOUSLY_H_RESOLUTION_MODE2);

    M5.Display.setCursor(10, 30);
    M5.Display.printf("Hello, %s", YOUR_NAME);
    M5.Display.setCursor(10, 60);
    M5.Display.printf("Please wait:\r\n Connecting to WiFi");

    // Connecting to WiFi
    transport.setUseTls(true);
    transport.setCerts(grafanaCert, strlen(grafanaCert));
    transport.setWifiSsid(WIFI_SSID);
    transport.setWifiPass(WIFI_PASSWORD);
    transport.setDebug(Serial);  // Remove this line to disable debug logging of the client.
    if (!transport.begin()) {
        Serial.println(transport.errmsg);
        while (true) {};        
    }

    M5.Display.setCursor(10, 105);
    M5.Display.setTextColor(GREEN, BLACK);
    M5.Display.printf("Connected!");
    delay(1500); 

    // Configure the Grafana Cloud client
    client.setUrl(GC_URL);
    client.setPath((char*)GC_PATH);
    client.setPort(GC_PORT);
    client.setUser(GC_USER);
    client.setPass(GC_PASS);
    client.setDebug(Serial);  // Remove this line to disable debug logging of the client.
    if (!client.begin()) {
        Serial.println(client.errmsg);
        while (true) {};
    }

    // Add our TimeSeries to the WriteRequest (adjusted to reflect only the used sensors)
    req.addTimeSeries(ts_m5stick_temperature);
    req.addTimeSeries(ts_m5stick_humidity);
    req.addTimeSeries(ts_m5stick_pressure);
    req.addTimeSeries(ts_m5stick_bat_volt);
    req.addTimeSeries(ts_m5stick_bat_current);
    req.addTimeSeries(ts_m5stick_bat_level);
    // req.addTimeSeries(ts_m5stick_voc); // Commented out
    // req.addTimeSeries(ts_m5stick_co2); // Commented out
    // req.addTimeSeries(ts_m5stick_lux); // Commented out
    Serial.println("End of setup()");
}

void loop() {
    int64_t time;
    time = transport.getTimeMillis();
    Serial.printf("\r\n====================================\r\n");

    // Get new updated values from our sensor
    temp = sht40.readTemperature();  // Get temperature from SHT40
    hum = sht40.readHumidity();      // Get humidity from SHT40

    // Get pressure data from BMP280
    pressure = bmp280.readPressure() / 100.0F;  // Pressure in hPa

    if (pressure < 950) { ESP.restart(); } // Restart if invalid pressure
    if (pressure > 1200) { ESP.restart(); } // Restart if pressure is too high
    Serial.printf("Temp: %2.1f °C \r\nHumidity: %2.0f%%  \r\nPressure: %2.0f hPa\r\n\n", temp, hum, pressure);

    // Read eCO2 and TVOC from SGP30 sensor (commented out)
    // if (!sgp.IAQmeasure()) {  
    //     Serial.println("eCO2/VOC Measurement failed!");
    // }
    // voc = int(sgp.TVOC);
    // co2 = int(sgp.eCO2);
    // Serial.printf("eCO2: %d ppm\nTVOC: %d ppb\n\n", co2, voc);

    // Get Lux (light) level (commented out)
    // lux = dlight.getLUX();
    // Serial.printf("Lux (light level): %d Lux\n\n", lux);

    // Gather battery state info
    int bat_volt = M5.Power.getBatteryVoltage();
    Serial.printf("Battery Volt: %dmv \n", bat_volt);

    int bat_current = M5.Power.getBatteryCurrent();
    Serial.printf("Battery Current: %dmv \n", bat_current);

    int bat_level = M5.Power.getBatteryLevel();
    Serial.printf("Battery Level: %d% \n\n", bat_level);

    // Now add all of our collected data to the timeseries
    ts_m5stick_temperature.addSample(time, temp);
    ts_m5stick_humidity.addSample(time, hum);
    ts_m5stick_pressure.addSample(time, pressure);
    ts_m5stick_bat_volt.addSample(time, bat_volt);
    ts_m5stick_bat_current.addSample(time, bat_current);
    ts_m5stick_bat_level.addSample(time, bat_level);

    // Send all data to Grafana Cloud
    PromClient::SendResult res = client.send(req);

    // Reset time series after sending data
    ts_m5stick_temperature.resetSamples();
    ts_m5stick_humidity.resetSamples();
    ts_m5stick_pressure.resetSamples();
    ts_m5stick_bat_volt.resetSamples();
    ts_m5stick_bat_current.resetSamples();
    ts_m5stick_bat_level.resetSamples();

    // Display updated information
    M5.Lcd.fillRect(0, 0, 256, 256, BLACK);
    M5.Display.setText
