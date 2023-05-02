#include <LiquidCrystal_Software_I2C.h>     // Include library
#include "DHT.h"   // Include library
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "time.h"
#include "sntp.h"

#define DHTPIN 12
#define DHTTYPE DHT22 
const char* ssid       = "WIFI_SSID";
const char* password   = "WIFI_PASSWORD";

const char* ntpServer1 = "kr.pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = 32400;
const int   daylightOffset_sec = 0;

int year=0;
int month=0;
int day=0;
int hour=0;
int minute=0;
int second=0;

float h = 0.0;
float t = 0.0;

int old_year=0;
int old_month=0;
int old_day=0;
int old_hour=0;
int old_minute=0;

DHT dht(DHTPIN, DHTTYPE);

WebServer server(80);

const char* time_zone = " KST-9";  // TimeZone rule for Korea including daylight adjustment rules (optional)

LiquidCrystal_I2C lcd(0x27, 16, 2, 16, 15);   // Set the LCD address to 0x27 for a 16 chars and 2 line display with 16, 15 pin

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("No time available (yet)");
    return;
  }
  year = timeinfo.tm_year - 100;
  month = timeinfo.tm_mon + 1 ;
  day = timeinfo.tm_mday ;
  hour = timeinfo.tm_hour;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;

  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    h = 99.9;
    t = 99.9;
  }
  
  if(day != old_day || hour != old_hour || minute != old_minute )
  {
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(String(" ") +String(year) + String("/") + String(month) + String("/") + String(day) + String(" ") + String(hour) +String(":")+String(minute));

    Serial.println(year);
    Serial.println(month);
    Serial.println(day);
    Serial.println(hour);
    Serial.println(minute);

    old_year=year;
    old_month=month;
    old_day=day;
    old_hour=hour;
    old_minute=minute;
  }
  if(second % 10 == 0)
  {
    lcd.setCursor(0, 1);
    lcd.print(String("H:") +String(h,1) + String("% ") + String("T:") + String(t,1) + String("C") );
  }
}

void handleRoot() {
  char temp[400];

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP32 Demo</title>\
    <style>\
      body { background-color: #f5f5dc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP32!</h1>\
    <p>Time: %02d/%02d/%02d %02d:%02d</p>\
    <p>Humidity: %.2f %,  Temperature: %.2f &#8451;</p>\
  </body>\
</html>",

           year, month, day, hour, minute, h, t
          );
  server.send(200, "text/html", temp);
}

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

// Callback function (get's called when time adjusts via NTP)
void timeavailable(struct timeval *t)
{
  Serial.println("Got time adjustment from NTP!");
  printLocalTime();
}


void setup() {
  Serial.begin(115200);
  
  lcd.init();                               // LCD initialization
  lcd.backlight();                          // Turn on backlight
  dht.begin();

  sntp_set_time_sync_notification_cb( timeavailable );
  sntp_servermode_dhcp(1);    // (optional)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  //connect to WiFi
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  server.on("/", handleRoot);
  server.on("/inline", []() {
  server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println(" CONNECTED");
}

void loop() 
{
  delay(1000);
  server.handleClient();
  printLocalTime(); 
}
