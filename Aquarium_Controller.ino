#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#define RTC_GND_PIN D4
#define RTC_VCC_PIN D3

#define RELAY_1_PIN D0
#define RELAY_2_PIN D1
#define PWM_PIN D2

const long utcOffsetInSeconds = 3 * 3600;

// Auxiliar variables to store the current output state
String relay1State = "off";
String relay2State = "off";

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// Variable to store the HTTP request
String header;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


WiFiManager wifiManager;
WiFiServer server(80);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

void setup() {
  Serial.begin(9600);
  pinMode(RTC_GND_PIN, OUTPUT);
  pinMode(RTC_VCC_PIN, OUTPUT);
  digitalWrite(RTC_GND_PIN, LOW);
  digitalWrite(RTC_VCC_PIN, HIGH);

  setSyncProvider(RTC.get);
  if (timeStatus() != timeSet) {
    Serial.println("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }
  //wifiManager.resetSettings();
  wifiManager.setConfigPortalBlocking(false);
  wifiManager.autoConnect("Aquarium");
  server.begin();
  timeClient.begin();
  delay(1000);
  setTimeFromInternet();
}

void loop() {
  wifiManager.process();
  WiFiClient client = server.available();
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /relay1/on") >= 0) {
              relay1State = "on";
              //digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /relay1/off") >= 0) {
              relay1State = "off";
              //digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /relay2/on") >= 0) {
              relay2State = "on";
              //digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /relay2/off") >= 0) {
              relay2State = "off";
              //digitalWrite(output4, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP8266 Aquarium Web Server</h1>");
            client.println("<h1>Today - " + curentDateToString() + "</h1>");
            client.println("<h1>Time - " + curentTimeToString() + "</h1>");

            // Display current state, and ON/OFF buttons for Relay 1
            client.println("<p>Relay 1 - State " + relay1State + "</p>");
            if (relay1State == "off") {
              client.println("<p><a href=\"/relay1/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/relay1/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for Relay 2
            client.println("<p>Relay 2 - State " + relay2State + "</p>");
            if (relay2State == "off") {
              client.println("<p><a href=\"/relay2/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/relay2/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
  //Main Loop
}


void setTimeFromInternet() {
  timeClient.update();
  setTime(timeClient.getEpochTime());
  RTC.set(timeClient.getEpochTime());
}

String curentTimeToString(){
  return String(hour()) + ":" + String(minute()) + ":" + String(second());
}

String curentDateToString(){
  return String(day()) + "." + String(month()) + "." + String(year());
}
