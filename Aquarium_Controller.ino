#include <WiFiManager.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define RELAY_1_PIN D0
#define RELAY_2_PIN D1
#define PWM_PIN D2

WiFiManager wifiManager;

void setup() {
  Serial.begin(9600);


  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }
  //wifiManager.resetSettings();
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.autoConnect("Aquarium");
}

void loop() {
  wifiManager.process();
  
  Serial.println("Test");
  delay(1000);
}
