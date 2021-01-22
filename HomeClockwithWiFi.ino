#include <ESP8266WiFi.h>
#include <time.h>
#include <Wire.h>
#include <RTClib.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "NewFont.h"
#include <Ticker.h>

/* prototype define */
void loadRTC();
void setRTC();
int get_NTP();
void printLocalTime();
int connectingWiFi();
void ledblink();
void displayUpdate();

/* Wifi define */
const char* ssid = "genoray-3f-1";
const char* password = "genoray12345";

const char* ntpServer = "pool.ntp.org";
int timeZone = 9;
int summerTime = 0;

#define NUM_RETRY_WIFI 100
#define NUM_RETRY_GET_NTP 100

/* RTC define */
RTC_DS3231 rtc;

// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char daysOfTheWeek[7][12] = {"일", "월", "화", "수", "목", "금", "토"};
// char hourAfterNoon[2][15] = {"Ante Meridiem", "Post Meridiem"};
char hourAfterNoon[2][15] = {"오전", "오후"};
int Hour, AmPm, Min, Sec, Temp;
int Year, Mon, Day;
char* wDay;
char* wAmPm;

/* max7219 dot matrix define */
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4
#define CS_PIN D8 // 15

// Create a new instance of the MD_Parola class with hardware SPI connection:
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

// Setup for software SPI:
// #define DATAPIN 2
// #define CLK_PIN 4
// MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

/* Ticker Timer define */
Ticker timer1;
Ticker timer2;
Ticker timer3;
//-----------------------------------------------------------------------------------------------

void loadRTC() {
  DateTime now = rtc.now();
  Year = now.year();
  Mon = now.month();
  Day = now.day();
  wDay = daysOfTheWeek[now.dayOfTheWeek()];
  Hour = now.hour();
  if (Hour > 12) {
    Hour -= 12;
    AmPm = 1;
  }
  else if (Hour == 0) {
    Hour = 12;
    AmPm = 0;
  }
  wAmPm = hourAfterNoon[AmPm];
  Min = now.minute();
  Sec = now.second();
  Temp = rtc.getTemperature();

  Serial.printf("현재 시간: %d년 %d월 %d일(%s) ", Year, Mon, Day, wDay);
  Serial.printf("%s %d시 %d분 %d초 온도: %d도\n", wAmPm, Hour, Min, Sec, Temp);
}

void setRTC() {
  time_t now = time(nullptr);
  Serial.print(ctime(&now));
  struct tm* ptm;
  ptm = localtime(&now);

  Year = ptm->tm_year - 100;
  Mon = ptm->tm_mon + 1;
  Day = ptm->tm_mday;
  wDay = daysOfTheWeek[ptm->tm_wday];
  Hour = ptm->tm_hour;
  Min = ptm->tm_min;
  Sec = ptm->tm_sec;
  rtc.adjust(DateTime(Year, Mon, Day, Hour, Min, Sec));
  Serial.printf("저장 시간: %d년 %d월 %d일(%s) ", Year + 2000, Mon, Day, wDay);
  Serial.printf("%d시 %d분 %d초\n", Hour, Min, Sec);
}

int get_NTP() {
  configTime(3600 * timeZone, 3600 * summerTime, ntpServer); //init and get the time
  Serial.println("\nWaiting for time");

  struct timeval inittm = { .tv_sec = 0};
  settimeofday(&inittm, NULL);

  while (1) {
    time_t now = time(nullptr);
    Serial.print(now);
    if (now % 20 == 0)  Serial.println("");
    else  Serial.print(".");
    delay(1000);
    if (now > NUM_RETRY_GET_NTP - 1) {
      if (now == NUM_RETRY_GET_NTP) {
        Serial.println("sync failed");
        return 0;
      }
      else {
        Serial.println("sync sucess");
        return 1;
      }
    }
  }
}

void printLocalTime() {
  time_t now = time(nullptr);
  //  Serial.print(ctime(&now));
  //  Serial.print(now);
  struct tm* ptm;
  ptm = localtime(&now);
  Serial.printf("현재시간 %02d시 %02d분 %02d초\n",
                ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
}

int connectingWiFi() {
  int cntTry = 0;
  Serial.println("\nConnecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    if (++cntTry % 20 == 0)  Serial.println("");
    else  Serial.print(".");
    delay(1000);
    if (cntTry > NUM_RETRY_WIFI - 1) return 0;
  }
  return 1;
}

void ledblink() {
  digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  rtc.begin();
  /*
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (connectingWiFi()) {
      if (get_NTP()) {
        printLocalTime();
        setRTC();
      }
    }
    WiFi.disconnect(true); //disconnect WiFi as it's no longer needed
    WiFi.mode(WIFI_OFF);
  */

  // Intialize the object:
  myDisplay.begin();
  myDisplay.setFont(newFont);
  // Set the intensity (brightness) of the display (0-15):
  myDisplay.setIntensity(10);
  // Clear the display:
  // myDisplay.displayClear();

  timer1.attach_ms(500, ledblink);
  timer2.attach_ms(1000, loadRTC);
  timer3.attach_ms(100, displayUpdate);
}

void displayUpdate() {
  //  if (myDisplay.displayAnimate()) {
  //    myDisplay.displayReset();
  //  }
  //  myDisplay.displayAnimate();

  char buf[16];
  sprintf(buf, "%d : %02d", Hour, Min);
  //  myDisplay.displayText(buf, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  //  myDisplay.displayAnimate();
  //
  //    if (myDisplay.displayAnimate()) {
  ////        sprintf(buf, "%d:%02d %dC ", Hour, Min, Temp);
  //  myDisplay.displayText("123:21 1234C", PA_LEFT, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  //    }

  //  myDisplay.displayAnimate();

  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.printf("%s", buf);
  //  Serial.printf("myDisplay %s \n", buf);

}

// the loop function runs over and over again forever
void loop() {

}
