#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <TimeLib.h>
#include <DS1307RTC.h>
#include <TimeAlarms.h>



#define RTC_GND_PIN D4
#define RTC_VCC_PIN D3

#define RELAY_1_PIN D0
#define RELAY_2_PIN D1
#define PWM_PIN D2



const long utcOffsetInSeconds = 3 * 3600;
// Auxiliar variables to store the current output state
String relay1State = "off";
String relay2State = "off";



AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server,&dns);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);



const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    input1: <input type="text" name="input1">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input2: <input type="text" name="input2">
    <input type="submit" value="Submit">
  </form><br>
  <form action="/get">
    input3: <input type="text" name="input3">
    <input type="submit" value="Submit">
  </form>
</body></html>)rawliteral";



void setup() {
  Serial.begin(9600);

  pinMode(RTC_GND_PIN, OUTPUT);
  pinMode(RTC_VCC_PIN, OUTPUT);
  digitalWrite(RTC_GND_PIN, LOW);
  digitalWrite(RTC_VCC_PIN, HIGH);

  //wifiManager.resetSettings();
  wifiManager.autoConnect("Aquarium");
  timeClient.begin();
  setSyncProvider(timeSyncProvider);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync time!");
  }

  initAsyncWebServer();
}

void loop() {
  Serial.println("Main Loop");
  Alarm.delay(1);
}

void initAsyncWebServer() {
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/get?input1=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    String inputParam;
    // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value();
      inputParam = PARAM_INPUT_1;
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_2)) {
      inputMessage = request->getParam(PARAM_INPUT_2)->value();
      inputParam = PARAM_INPUT_2;
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_INPUT_3)) {
      inputMessage = request->getParam(PARAM_INPUT_3)->value();
      inputParam = PARAM_INPUT_3;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field ("
                  + inputParam + ") with value: " + inputMessage +
                  "<br><a href=\"/\">Return to Home Page</a>");
  });
  server.onNotFound(notFound);
  server.begin();
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

time_t timeSyncProvider() {
  if (timeClient.update()) {
    RTC.set(timeClient.getEpochTime());
    return timeClient.getEpochTime();
  } else {
    return RTC.get();
  }

}

String curentTimeToString() {
  return String(hour()) + ":" + String(minute()) + ":" + String(second());
}

String curentDateToString() {
  return String(day()) + "." + String(month()) + "." + String(year());
}

void turnOffRelay1() {
  digitalWrite(RELAY_1_PIN, HIGH);
  relay1State = "off";
}

void turnOnRelay1() {
  digitalWrite(RELAY_1_PIN, LOW);
  relay1State = "on";
}

void turnOffRelay2() {
  digitalWrite(RELAY_2_PIN, HIGH);
  relay2State = "off";
}

void turnOnRelay2() {
  digitalWrite(RELAY_2_PIN, LOW);
  relay2State = "on";
}
