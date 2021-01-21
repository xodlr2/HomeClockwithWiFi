#include <ESP8266WiFi.h>
#include <time.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

const char* ssid = "genoray-3f-1";
const char* password = "genoray12345";

const char* ntpServer = "pool.ntp.org";
int timeZone = 9;
int summerTime = 0;

#define NUM_RETRY_WIFI 100
#define NUM_RETRY_GET_NTP 100

// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char daysOfTheWeek[7][12] = {"일", "월", "화", "수", "목", "금", "토"};
// char hourAfterNoon[2][15] = {"Ante Meridiem", "Post Meridiem"};
char hourAfterNoon[2][15] = {"오전", "오후"};
int Hour, AmPm, Min, Sec, Temp;
int Year, Mon, Day;
char* wDay;
char* wAmPm;

int loadRTC() {
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
  Serial.printf("저장 시간: %d년 %d월 %d일(%s) ", Year+2000, Mon, Day, wDay);
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

void setup() {

  //  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  Serial.begin(115200);
  Serial.setDebugOutput(true);

  rtc.begin();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (connectingWiFi()) {
    if (get_NTP()) {
      printLocalTime();
      setRTC();
    }
  }
  else {
    loadRTC();
  }
  WiFi.disconnect(true); //disconnect WiFi as it's no longer needed
  WiFi.mode(WIFI_OFF);
}

// the loop function runs over and over again forever
void loop() {
  //  printLocalTime();
  loadRTC();
  delay(1000);
}
