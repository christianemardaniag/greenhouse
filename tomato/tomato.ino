#define BLYNK_TEMPLATE_ID "TMPLqb-MRBCp"
#define BLYNK_TEMPLATE_NAME "Crop"
#define BLYNK_AUTH_TOKEN "_YWkKHFtgrWHbSVhv6HFTMm-cBllbEqz"

#define DHTTYPE DHT11
#define DHT_SENSOR D1
#define SOIL_SENSOR A0
#define DP_TEMP V0
#define DP_HUMI V1
#define DP_MOIS V2
#define SW_FAN V4
#define SW_LIGHT V5
#define SW_PUMP V6
#define LED_DHT V7
#define LED_SOIL V8
#define MODE V9

#define RL_FAN D5
#define RL_LIGHT D6
#define RL_PUMP D7

#define TEMP_MIN 18
#define TEMP_MAX 30
#define HUMI_MIN 65
#define HUMI_MAX 85
#define MOIS_MIN 70  // WET
#define MOIS_MAX 80  // DRY

#define ON LOW
#define OFF HIGH

#include "DHT.h"
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Your WiFi credentials.
const char ssid[] = "PLDT_Home_A2CC7";
const char pass[] = "pldthome";
// const char ssid[] = "tiiaaan!";
// const char pass[] = "00000000";
const char auth[] = BLYNK_AUTH_TOKEN;

unsigned long previousMillisDelay = 0;
const long intervalDelay = 2000;
const char crop[] = "TOMATO";
bool mode = LOW;
bool isHighTemp = false;
int fanStatus = OFF, lightStatus = OFF, pumpStatus = OFF;

DHT dht(DHT_SENSOR, DHTTYPE);

BLYNK_CONNECTED() {
  Blynk.syncAll();
}
BLYNK_WRITE(SW_FAN) {
  int value = param.asInt();
  digitalWrite(RL_FAN, value);
}
BLYNK_WRITE(SW_LIGHT) {
  int value = param.asInt();
  digitalWrite(RL_LIGHT, value);
}
BLYNK_WRITE(SW_PUMP) {
  int value = param.asInt();
  digitalWrite(RL_PUMP, value);
}
BLYNK_WRITE(MODE) {
  int value = param.asInt();
  mode = value;
}

void temperature_humidity();
void soil_moisture();

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  pinMode(RL_FAN, OUTPUT);
  pinMode(RL_LIGHT, OUTPUT);
  pinMode(RL_PUMP, OUTPUT);
  digitalWrite(RL_FAN, fanStatus);
  digitalWrite(RL_LIGHT, lightStatus);
  digitalWrite(RL_PUMP, pumpStatus);
  Blynk.virtualWrite(SW_FAN, fanStatus);
  Blynk.virtualWrite(SW_LIGHT, lightStatus);
  Blynk.virtualWrite(SW_PUMP, pumpStatus);
  Blynk.virtualWrite(MODE, mode);
}

void loop() {
  Blynk.run();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisDelay >= intervalDelay) {
    previousMillisDelay = currentMillis;
    temperature_humidity();
    soil_moisture();
  }
}

void temperature_humidity() {
  float humi = dht.readHumidity();
  float temp = dht.readTemperature();
  fanStatus = OFF;
  lightStatus = OFF;
  pumpStatus = OFF;
  if (isnan(humi) || isnan(temp)) {
    Blynk.virtualWrite(LED_DHT, LOW);
    Blynk.virtualWrite(DP_TEMP, NULL);
    Blynk.virtualWrite(DP_HUMI, NULL);
    if (mode) {
      Blynk.virtualWrite(SW_FAN, fanStatus);
      Blynk.virtualWrite(SW_LIGHT, lightStatus);
      Blynk.virtualWrite(SW_PUMP, pumpStatus);
      digitalWrite(RL_FAN, fanStatus);
      digitalWrite(RL_LIGHT, lightStatus);
      digitalWrite(RL_PUMP, pumpStatus);
    }
    Serial.println("DHT not working");
    Blynk.logEvent("sensor", crop + String(" DHT Sensor is not working properly"));
    return;
  }
  if (temp >= TEMP_MAX || humi >= HUMI_MAX) {
    fanStatus = ON;
  } else if (temp <= TEMP_MIN || humi <= HUMI_MIN) {
    lightStatus = ON;
  }
  if (temp >= (TEMP_MAX + 3)) {
    isHighTemp = true;
    pumpStatus = ON;
  } else {
    isHighTemp = false;
  }
  if (temp >= (TEMP_MAX + 5)) {
    Blynk.logEvent("temperature_warning", String("Attention! Greenhouse with tomato plants experiencing temperature fluctuations. Please monitor and take action to adjust temperature accordingly to ensure optimal growth and health of the plants."));
  }

  Blynk.virtualWrite(LED_DHT, HIGH);
  Blynk.virtualWrite(DP_TEMP, temp);
  Blynk.virtualWrite(DP_HUMI, humi);
  if (mode) {
    Blynk.virtualWrite(SW_FAN, fanStatus);
    Blynk.virtualWrite(SW_LIGHT, lightStatus);
    Blynk.virtualWrite(SW_PUMP, pumpStatus);
    digitalWrite(RL_FAN, fanStatus);
    digitalWrite(RL_LIGHT, lightStatus);
    digitalWrite(RL_PUMP, pumpStatus);
  }
}

void soil_moisture() {
  float moisture = analogRead(SOIL_SENSOR);
  if (!isHighTemp) {
    pumpStatus = OFF;
  }
  if (moisture < 150) {
    Blynk.virtualWrite(LED_SOIL, LOW);
    Blynk.virtualWrite(DP_MOIS, NULL);
    Blynk.virtualWrite(SW_PUMP, pumpStatus);
    digitalWrite(RL_PUMP, pumpStatus);
    Blynk.logEvent("sensor", crop + String(" Soil Moisture Sensor is not working properly"));
    return;
  }
  moisture = (moisture / 1024) * 100;
  Blynk.virtualWrite(DP_MOIS, moisture);
  Blynk.virtualWrite(LED_SOIL, HIGH);
  if (moisture > MOIS_MAX) {
    pumpStatus = ON;
  }
  if (mode) {
    Blynk.virtualWrite(SW_PUMP, pumpStatus);
    digitalWrite(RL_PUMP, pumpStatus);
  }
}