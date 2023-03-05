#define BLYNK_TEMPLATE_ID "TMPLqb-MRBCp"
#define BLYNK_TEMPLATE_NAME "Crop"
#define BLYNK_AUTH_TOKEN "_YWkKHFtgrWHbSVhv6HFTMm-cBllbEqz"

#define PH_SENSOR A0
#define DP_PH V3

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Your WiFi credentials.
const char ssid[] = "PLDT_Home_A2CC7";
const char pass[] = "pldthome";
// const char ssid[] = "tiiaaan!";
// const char pass[] = "00000000";
const char auth[] = BLYNK_AUTH_TOKEN;

unsigned long int avgValue;
int buf[10], temp;
float value;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
}

void loop() {
  // put your main code here, to run repeatedly:
  Blynk.run();
  for (int i = 0; i < 10; i++)  //Get 10 sample value from the sensor for smooth the value
  {
    buf[i] = analogRead(PH_SENSOR);
    delay(10);
  }
  for (int i = 0; i < 9; i++)  //sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++) {
      if (buf[i] > buf[j]) {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)  //take the average value of 6 center sample
    avgValue += buf[i];
  value = (float)avgValue * 5.0 / 1024 / 6;  //convert the analog into millivolt
  value = 3.5 * value;
  Serial.println(value);
  if (value > 1)
    Blynk.virtualWrite(DP_PH, value);
  else
    Blynk.virtualWrite(DP_PH, NULL);
}
