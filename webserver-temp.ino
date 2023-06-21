#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define DHTTYPE DHT11

const char* ssid = "SSID";
const char* password = "PASSWORD";

ESP8266WebServer server(80);

const int DHTPin = 2;
DHT dht(DHTPin, DHTTYPE);

bool isLEDon = false;

void setCrossOrigin(){
    server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
    server.sendHeader(F("Access-Control-Max-Age"), F("600"));
    server.sendHeader(F("Access-Control-Allow-Methods"), F("PUT,POST,GET,OPTIONS"));
    server.sendHeader(F("Access-Control-Allow-Headers"), F("*"));
};

void sendCrossOriginHeader(){
    Serial.println(F("sendCORSHeader"));
    setCrossOrigin();
    server.send(204);
}

// Serving Hello world
void getSettings() {
  setCrossOrigin();
  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  //  StaticJsonDocument<512> doc;
  // You can use DynamicJsonDocument as well
  DynamicJsonDocument doc(512);

  doc["ip"] = WiFi.localIP().toString();
  doc["gw"] = WiFi.gatewayIP().toString();
  doc["nm"] = WiFi.subnetMask().toString();

  if (server.arg("signalStrength") == "true") {
    doc["signalStrengh"] = WiFi.RSSI();
  }

  if (server.arg("chipInfo") == "true") {
    doc["chipId"] = ESP.getChipId();
    doc["flashChipId"] = ESP.getFlashChipId();
    doc["flashChipSize"] = ESP.getFlashChipSize();
    doc["flashChipRealSize"] = ESP.getFlashChipRealSize();
  }
  if (server.arg("freeHeap") == "true") {
    doc["freeHeap"] = ESP.getFreeHeap();
  }

  Serial.print(F("Stream..."));
  String buf;
  serializeJson(doc, buf);
  server.send(200, F("application/json"), buf);
  Serial.print(F("done."));
}

void getDHT() {
  setCrossOrigin();
  DynamicJsonDocument doc(512);

  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  static char humidityTemp[9];
  dtostrf(h, 6, 4, humidityTemp);
  // Read temperature as Celsius (the default)
  float c = dht.readTemperature();
  static char temperatureTemp[9];
  dtostrf(c, 6, 4, temperatureTemp);
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  static char fahrenheitTemp[9];
  dtostrf(f, 6, 4, fahrenheitTemp);

    if (!isnan(c)) {
    doc["tempC"] = temperatureTemp;
  }
  else {
    doc["tempC"] = NULL;
  }
  if (!isnan(f)) {
    doc["tempF"] = fahrenheitTemp;
  } else {
    doc["tempF"] = NULL;
  }
  if (!isnan(h)) {
    doc["tempH"] = humidityTemp;
  } else {
    doc["tempH"] = NULL;
  }

  // doc["tempF"] = f;
  // doc["humidity"] = h;

  Serial.print(F("Stream..."));
  String buf;
  serializeJson(doc, buf);
  server.send(200, F("application/json"), buf);
  Serial.println(F("done."));
}

// POST Route
void setLED() {
  setCrossOrigin();
  String postBody = server.arg("plain");
  Serial.println(postBody);
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, postBody);
  if (error) {
    Serial.print(F("Error parsing JSON "));
    Serial.print(error.c_str());
    String msg = error.c_str();
    server.send(400, F("text/html"), "Error in parsing JSON body </br>" + msg);
  } else {
    JsonObject postObj = doc.as<JsonObject>();

    Serial.print(F("HTTP Method: "));
    Serial.println(server.method());

    if (server.method() == HTTP_POST) {
      if (postObj.containsKey("status")) {
        Serial.println(F("done."));
        if (!isLEDon) {
            digitalWrite(0, HIGH);
            isLEDon = true;
          }
        else {
          digitalWrite(0, LOW);
          isLEDon = false;
        }


        DynamicJsonDocument doc(512);
        doc["status"] = "OK";
        Serial.print(F("Stream..."));
        String buf;
        serializeJson(doc, buf);

        server.send(201, F("application/json"), buf);
        Serial.print(F("done"));
      } else {
        DynamicJsonDocument doc(512);
        doc["status"] = "BAD REQUEST";
        doc["message"] = F("No data found, or incorrect.");

        Serial.print(F("Stream..."));
        String buf;
        serializeJson(doc, buf);

        server.send(400, F("application/json"), buf);
        Serial.print(F("done"));
      }
    }
  }
}

// Define routing
void restServerRouting() {
  server.on("/", HTTP_GET, []() {
    server.send(200, F("text/html"),
                F("Welcome to the REST Web Server"));
  });
  server.on(F("/settings"), HTTP_GET, getSettings);
  server.on(F("/settings"), HTTP_OPTIONS, sendCrossOriginHeader);
  server.on(F("/temperature"), HTTP_GET, getDHT);
  server.on(F("/led"), HTTP_OPTIONS, sendCrossOriginHeader);
  server.on(F("/led"), HTTP_POST, setLED);
}

// Manage not found URL
void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  dht.begin();
  Serial.begin(115200);
  pinMode(0, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Activate mDNS this is used to be able to connect to the server
  // with local DNS hostmane esp8266.local
  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  // Set server routing
  restServerRouting();
  // Set not found response
  server.onNotFound(handleNotFound);
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
