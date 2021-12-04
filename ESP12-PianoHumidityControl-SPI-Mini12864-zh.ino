#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <JsonListener.h>
#include <stdio.h>
#include <time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()
#include <Timezone.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <TimeLib.h>
#include "HeWeatherCurrent.h"
#include "WeatherFonts.h"

#define CURRENT_VERSION 4
//#define DEBUG
#define LANGUAGE_CN  // LANGUAGE_CN or LANGUAGE_EN

const char* WIFI_SSID[] = {"ibehome", "ibetest", "ibehomen", "TYCP", "Tenda_301"};
const char* WIFI_PWD[] = {"tianwanggaidihu", "tianwanggaidihu", "tianwanggaidihu", "5107458970", "5107458970"};
#define numWIFIs (sizeof(WIFI_SSID)/sizeof(char *))
#define WIFI_TRY 30

#define TZ              8       // (utc+) TZ in hours
#define DST_MN          0      // use 60mn for summer time in some countries
#define TZ_MN           ((TZ)*60)
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
#define UPDATE_INTERVAL_SECS  (20 * 60) // Update every 20 minutes

const String HEWEATHER_APP_ID = "d72b42bcfc994cfe9099eddc9647c6f2";


// Serial 1300

#define DHTTYPE  AM2301       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   2
#define RELAYPIN 5
#define BACKLIGHT 4

U8G2_ST7565_64128N_F_4W_SW_SPI display(U8G2_R0, /* clock=*/ 14, /* data=*/ 12, /* cs=*/ 13, /* dc=*/ 15, /* reset=*/ 16); // U8G2_ST7565_64128N_F_4W_SW_SPI, u8x8_d_st7565.c U8X8_C(0x060)

#define MAXHUMIDITY 50

#ifdef LANGUAGE_CN
const String HEWEATHER_LANGUAGE = "zh"; // zh for Chinese, en for English
#else ifdef LANGUAGE_EN
const String HEWEATHER_LANGUAGE = "en"; // zh for Chinese, en for English
#endif

const String HEWEATHER_LOCATION = "CN101210202"; // Changxing

#ifdef LANGUAGE_CN
const String WDAY_NAMES[] = { "星期天", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六" };
#else ifdef LANGUAGE_EN
const String WDAY_NAMES[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
#endif

DHT dht(DHTPIN, DHTTYPE);

HeWeatherCurrentData currentWeather;
HeWeatherCurrent currentWeatherClient;

time_t nowTime;
const String degree = String((char)176);
bool readyForWeatherUpdate = false;
long timeSinceLastWUpdate = 0;
float previousTemp = 0;
float previousHumidity = 0;


void connectWiFi()
{
  if (WiFi.status() == WL_CONNECTED) return;
  int intPreferredWIFI = 0;
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  int n = WiFi.scanNetworks();
  if (n == 0)
  {
  }
  else
  {
    for (int i = 0; i < n; ++i)
    {
      for (int j = 0; j < numWIFIs; j++)
      {
        if (strcmp(WIFI_SSID[j], string2char(WiFi.SSID(i))) == 0)
        {
          intPreferredWIFI = j;
          break;
        }
      }
    }
  }

  WiFi.persistent(true);
  WiFi.begin(WIFI_SSID[intPreferredWIFI], WIFI_PWD[intPreferredWIFI]);
  int WIFIcounter = intPreferredWIFI;
  while (WiFi.status() != WL_CONNECTED) {
    int counter = 0;
    while (counter < WIFI_TRY && WiFi.status() != WL_CONNECTED)
    {
      if (WiFi.status() == WL_CONNECTED) break;
      delay(500);
      if (WiFi.status() == WL_CONNECTED) break;
      counter++;
    }
    if (WiFi.status() == WL_CONNECTED) break;
    WIFIcounter++;
    if (WIFIcounter >= numWIFIs) WIFIcounter = 0;
    WiFi.begin(WIFI_SSID[WIFIcounter], WIFI_PWD[WIFIcounter]);
  }
  return;
}

void turnOff() {
  digitalWrite(RELAYPIN, HIGH);
}

void turnOn() {
  digitalWrite(RELAYPIN, LOW);
}

void setup() {
  delay(100);
  Serial.begin(115200);
#ifdef DEBUG
  Serial.println("Begin");
#endif

  dht.begin();

  pinMode(BACKLIGHT, OUTPUT);
  analogWrite(BACKLIGHT, 255);

  pinMode(RELAYPIN, OUTPUT);
  turnOff();

  turnOn();
  delay(2000);
  turnOff();

  display.begin();
  display.setFontPosTop();
  display.setContrast(128);
  display.clearBuffer();
  display.drawXBM(31, 0, 66, 64, garfield);
  display.sendBuffer();
  updateData(true);
}

void loop() {
  display.firstPage();
  do {
    drawLocalSystem();
  } while ( display.nextPage() );

  if (millis() - timeSinceLastWUpdate > (1000 * UPDATE_INTERVAL_SECS)) {
    readyForWeatherUpdate = true;
    timeSinceLastWUpdate = millis();
  }

  if (dht.read())
  {
    float fltHumidity = dht.readHumidity() * 100 / 100 + 0;
    float fltCTemp = dht.readTemperature() * 100 / 100 + 0;
#ifdef DEBUG
    Serial.print("Humidity: ");
    Serial.println(fltHumidity);
    Serial.print("CTemp: ");
    Serial.println(fltCTemp);
#endif
    if (isnan(fltCTemp) || isnan(fltHumidity))
    {
    }
    else
    {
      previousTemp = fltCTemp;
      if (fltHumidity <= 100)
      {
        previousHumidity = fltHumidity;
      }
      else
      {
        previousHumidity = 100;
      }
      if (previousHumidity > MAXHUMIDITY)
      {
        turnOn();
      }
      else
      {
        turnOff();
      }
    }
  }
  DebugShowSystemTime();
  if (readyForWeatherUpdate) {
    updateData(false);
  }
  delay(1000);
}

void updateData(bool isInitialBoot) {
  // Connect WiFi
  connectWiFi();
  if (WiFi.status() == WL_CONNECTED) 
  {
    configTime(TZ_SEC, DST_SEC, "pool.ntp.org");
    long beginTime = millis();
    int inteval = 30;
    if (isInitialBoot) inteval = 5;
    while(time(nullptr) <= 100000) {
      if ((millis() - beginTime) > inteval * 1000) 
      {
        return; // it more than inteval seconds return
      }
      delay(100);
    }
  }
  currentWeatherClient.updateCurrent(&currentWeather, HEWEATHER_APP_ID, HEWEATHER_LOCATION, HEWEATHER_LANGUAGE);
  readyForWeatherUpdate = false;
  return;
}

void drawLocalSystem() {
  nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);
  char buff[20];
  if (timeInfo->tm_hour >= 0 && timeInfo->tm_hour < 6)
  {
    analogWrite(BACKLIGHT, 32);
  }
  else if (timeInfo->tm_hour >= 6 && timeInfo->tm_hour < 8)
  {
    analogWrite(BACKLIGHT, 128);
  }
  else if (timeInfo->tm_hour >= 8 && timeInfo->tm_hour < 17)
  {
    analogWrite(BACKLIGHT, 255);
  }
  else if (timeInfo->tm_hour >= 17 && timeInfo->tm_hour < 22)
  {
    analogWrite(BACKLIGHT, 128);
  }
  else if (timeInfo->tm_hour >= 22 && timeInfo->tm_hour <= 23)
  {
    analogWrite(BACKLIGHT, 64);
  }
  display.enableUTF8Print();
  display.setFont(u8g2_font_wqy12_t_gb2312); // u8g2_font_wqy12_t_gb2312, u8g2_font_helvB08_tf
  String stringText = String(timeInfo->tm_year + 1900) + "年" + String(timeInfo->tm_mon + 1) + "月" + String(timeInfo->tm_mday) + "日 " + WDAY_NAMES[timeInfo->tm_wday].c_str();
  int stringWidth = display.getUTF8Width(string2char(stringText));
  display.setCursor((128 - stringWidth) / 2, 1);
  display.print(stringText);
  stringWidth = display.getUTF8Width(string2char(String(currentWeather.cond_txt)));
  display.setCursor((128 - stringWidth) / 2, 40);
  display.print(String(currentWeather.cond_txt));
  String WindDirectionAndSpeed = windDirectionTranslate(currentWeather.wind_dir) + String(currentWeather.wind_sc) + "级";
  stringWidth = display.getUTF8Width(string2char(WindDirectionAndSpeed));
  display.setCursor((128 - stringWidth) / 2, 54);
  display.print(WindDirectionAndSpeed);
  display.disableUTF8Print();
  display.setFont(u8g2_font_helvR24_tn); // u8g2_font_inb21_ mf, u8g2_font_helvR24_tn
  //  sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);
  sprintf_P(buff, PSTR("%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min);
  stringWidth = display.getStrWidth(buff);
  display.drawStr((128 - 30 - stringWidth) / 2, 11, buff);
  display.setFont(Meteocon21);
  if (previousHumidity > MAXHUMIDITY)
  {
    display.drawStr(98, 17, string2char("'"));
  }
  else
  {
    display.drawStr(98, 17, string2char(chooseMeteoconSystem(currentWeather.iconMeteoCon)));
  }
  display.setFont(u8g2_font_helvR08_tf);
  String temp = String(currentWeather.tmp) + degree + "C";
  display.drawStr(0, 53, string2char(temp));
  display.setFont(u8g2_font_helvR08_tf);
  stringWidth = display.getStrWidth(string2char((String(currentWeather.hum) + "%")));
  display.drawStr(127 - stringWidth, 53, string2char((String(currentWeather.hum) + "%")));
  display.setFont(u8g2_font_helvB08_tf);
  if (previousTemp != 0 && previousHumidity != 0)
  {
    display.drawStr(0, 39, string2char(String(previousTemp, 0) + degree + "C"));
  }
  else
  {
    //    display.drawStr(0, 39, string2char("..."));
  }

  if (previousTemp != 0 && previousHumidity != 0)
  {
    String thisTempHumidity = String(previousHumidity, 0) + "%";
    int stringWidth = display.getStrWidth(string2char(thisTempHumidity));
    display.drawStr(128 - stringWidth, 39, string2char(thisTempHumidity));
  }
  else
  {
    //    int stringWidth = display.getStrWidth(string2char("..."));
    //    display.drawStr(128 - stringWidth, 39, string2char("..."));
  }
  display.drawHLine(0, 51, 128);
}

String chooseMeteoconSystem(String stringInput) {
  time_t nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);
  if (timeInfo->tm_hour > 6 && timeInfo->tm_hour < 18)
  {
    return stringInput.substring(0, 1);
  }
  else
  {
    return stringInput.substring(1, 2);
  }
}

String windDirectionTranslate(String stringInput) {
  String stringReturn = stringInput;
  stringReturn.replace("N", "北");
  stringReturn.replace("S", "南");
  stringReturn.replace("E", "东");
  stringReturn.replace("W", "西");
  stringReturn.replace("无持续", "无");
  return stringReturn;
}

char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}

String intToString(int intInput) {
  if (intInput > 9)
  {
    return String(intInput);
  }
  else
  {
    return "0" + String(intInput);
  }
}

void DebugShowSystemTime() {
  time_t nowTime = time(nullptr);
  struct tm * timeInfo;
  timeInfo = localtime (&nowTime);
#ifdef DEBUG  
  Serial.print("SYS ");
  Serial.print(timeInfo->tm_year - 100);    // 00-99
  Serial.print('-');
  Serial.print(intToString(timeInfo->tm_mon + 1));   // 01-12
  Serial.print('-');
  Serial.print(intToString(timeInfo->tm_mday));     // 01-31
  Serial.print(' ');
  Serial.print(intToString(timeInfo->tm_hour));    // 00-23
  Serial.print(':');
  Serial.print(intToString(timeInfo->tm_min));  // 00-59
  Serial.print(':');
  Serial.print(intToString(timeInfo->tm_sec));  // 00-59
  Serial.println();
#endif  
}
