#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include "DHT.h"
#include <ArduinoJson.h>

#define DHTTYPE DHT22

uint8_t DHTPin = D8;
DHT dht(DHTPin, DHTTYPE);

const char *ssid = "";
const char *password = "";

String serverName = "http://iot.ozcberkay.com:3000";

String getCall(String url);
void setSensorDelaySecond(int delaySecond);
void readSensorData();

unsigned long sensorDelay = 10000;
unsigned long optionsDelay = 10 * 60 * 1000;

void setup()
{
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DHTPin, INPUT);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

unsigned long sensorTimer = 0;
unsigned long optionsTimer = 0;
String Temperature;
String Humidity;

void loop()
{
  if ((millis() - sensorTimer) > sensorDelay)
  {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED)
    {
      digitalWrite(LED_BUILTIN, LOW); // turn the LED on (HIGH is the voltage level)

      readSensorData();

      String url = serverName + "/push" + "?temp=" + Temperature + "&hum=" + Humidity;
      String payload = getCall(url);
      Serial.println(payload);

      digitalWrite(LED_BUILTIN, HIGH); // turn the LED off by making the voltage LOW
    }
    else
    {
      Serial.println("WiFi Disconnected");
    }
    sensorTimer = millis();
  }

  if ((millis() - optionsTimer) > optionsDelay)
  {
    String url = serverName + "/options";
    String payload = getCall(url);
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
    }
    else
    {
      int loop_interval = doc["loop_interval"];
      setSensorDelaySecond(loop_interval);
    }
    optionsTimer = millis();
  }
}

String getCall(String url)
{
  HTTPClient http;
  String payload = "";
  http.begin(url);

  int httpResponseCode = http.GET();

  if (httpResponseCode > 0)
  {
    payload = http.getString();
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;
}

void setSensorDelaySecond(int delaySecond)
{
  if ((delaySecond * 1000) != sensorDelay)
  {
    Serial.println("New sensor delay : " + String(delaySecond) + "(sec)");
    sensorDelay = delaySecond * 1000;
  }
}

void readSensorData()
{
  Temperature = String(dht.readTemperature());
  Humidity = String(dht.readHumidity());
}