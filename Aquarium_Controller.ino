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
bool relay1State = false;
bool relay2State = false;
byte pwmState = 0;



AsyncWebServer server(80);
DNSServer dns;
AsyncWiFiManager wifiManager(&server, &dns);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);



const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
  <head>
    <title>ESP8266 Aquarium Web Server</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    
    <style>
      html {
        font-family: Helvetica;
        display: inline-block;
        margin: 0px auto;
        text-align: center;
      }
      .button {
        background-color: #195B6A;
        border: none;
        color: white;
        padding: 16px 40px;
        text-decoration: none;
        font-size: 30px;
        margin: 2px;
        cursor: pointer;
      }
      .button2 {
        background-color: #77878A;
      }
      .block {
        display: block;
        border: 2px solid blue;
      } 
    </style>
  
  </head>
  
  <body>
    <h1>ESP8266 Aquarium Web Server</h1>
    <div class="block">
      <h2>System status:</h2>
      <h3>Date - %DATE%</h3>
      <h3>Time - %TIME%</h3>
      <h3>Relay 1 - %RELAY1%</h3>
      <h3>Relay 2 - %RELAY2%</h3>
      <h3>PWM - %PWM%</h3>
    </div>
  </body>
</html>)rawliteral";



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
    request->send_P(200, "text/html", index_html, processor);
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

String processor(const String& var){
  //Serial.println(var);
  if(var == "DATE"){
    return curentDateToString();
  }
  else if(var == "TIME"){
    return curentTimeToString();
  }
  else if(var == "RELAY1"){
    return relayStateToString(relay1State);
  }
  else if(var == "RELAY2"){
    return relayStateToString(relay2State);
  }
  return String();
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

String relayStateToString(bool relayState){
  if(relayState){
    return "On";
  }else{
    return "Off";
  }
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
