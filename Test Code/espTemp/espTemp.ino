/*
 * This ESP32 code is created by esp32io.com
 *
 * This ESP32 code is released in the public domain
 *
 * For more detail (instruction and wiring diagram), visit https://esp32io.com/tutorials/esp32-temperature-via-web
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SENSOR_PIN 18  // ESP32 pin GPIO17 connected to DS18B20 sensor's DATA pin

const char* ssid = "ATTiSt38u4";     // CHANGE IT
const char* password = "SimpleIndoorLife";  // CHANGE IT

OneWire oneWire(SENSOR_PIN);          // setup a oneWire instance
DallasTemperature DS18B20(&oneWire);  // pass oneWire to DallasTemperature library

AsyncWebServer server(80);

float getTemperature() {
  DS18B20.requestTemperatures();             // send the command to get temperatures
  float tempC = DS18B20.getTempCByIndex(0);  // read temperature in °C
  return tempC;
}

void setup() {
  Serial.begin(9600);
  DS18B20.begin();  // initialize the DS18B20 sensor

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Print the ESP32's IP address
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());

  // Define a route to serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    Serial.println("ESP32 Web Server: New request received:");  // for debugging
    Serial.println("GET /");                                    // for debugging

    // get temperature from sensor
    float temperature = getTemperature();
    // Format the temperature with two decimal places
    String temperatureStr = String(temperature, 2);

    String html = "<!DOCTYPE HTML>";
    html += "<html>";
    html += "<head>";
    html += "<link rel=\"icon\" href=\"data:,\">";
    html += "</head>";
    html += "<p>";
    html += "Temperature: <span style=\"color: red;\">";
    html += temperature;
    html += "&deg;C</span>";
    html += "</p>";
    html += "</html>";

    request->send(200, "text/html", html);
  });

  // Start the server
  server.begin();
}

void loop() {
  // Your code here
}
