#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESPHTTPClient.h>
#include <JsonListener.h>
#include <stdio.h>
#include <time.h>                   // struct timeval
#include <coredecls.h>                  // settimeofday_cb()
#include <Timezone.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <WiFiManager.h>
#include <ESP8266httpUpdate.h>
#include "FS.h"
#include "HeWeatherCurrent.h"
#include "GarfieldCommon.h"

#define CURRENT_VERSION 1
//#define DEBUG
//#define USE_WIFI_MANAGER     // disable to NOT use WiFi manager, enable to use
#define DISPLAY_TYPE 2   // 1-BIG 12864, 2-MINI 12864, 3-New Big BLUE 12864, to use 3, you must change u8x8_d_st7565.c as well!!!, 4- New BLUE 12864-ST7920
#define LANGUAGE_CN  // LANGUAGE_CN or LANGUAGE_EN


// Serial 1300
int serialNumber = -1;
String Location = "Default";
String Token = "Token";
int Resistor = 80000;
bool dummyMode = false;
bool backlightOffMode = false;
bool sendAlarmEmail = false;
String alarmEmailAddress = "Email";
int displayContrast = 128;
int displayMultiplier = 100;
int displayBias = 0;
int displayMinimumLevel = 1;
int displayMaximumLevel = 1023;
int temperatureMultiplier = 100;
int temperatureBias = 0;
int humidityMultiplier = 100;
int humidityBias = 0;
int firmwareversion = 0;
String firmwareBin = "";

// BIN files: 1300.bin

#define DHTTYPE  AM2301       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   2 // 2, -1
#define RELAYPIN 5
#define BACKLIGHTPIN 0 // 2, 0

#define MAXHUMIDITY 50

#if DISPLAY_TYPE == 3
#define BIGBLUE12864
#endif

#ifdef LANGUAGE_CN
const String HEWEATHER_LANGUAGE = "zh"; // zh for Chinese, en for English
#else ifdef LANGUAGE_EN
const String HEWEATHER_LANGUAGE = "en"; // zh for Chinese, en for English
#endif

#ifdef USE_WIFI_MANAGER
const String HEWEATHER_LOCATION = "auto_ip"; // Get location from IP address
#else
const String HEWEATHER_LOCATION = "CN101210202"; // Changxing
#endif

#ifdef LANGUAGE_CN
const String WDAY_NAMES[] = { "星期天", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六" };
#else ifdef LANGUAGE_EN
const String WDAY_NAMES[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
#endif

#if (DHTPIN >= 0)
DHT dht(DHTPIN, DHTTYPE);
#endif

HeWeatherCurrentData currentWeather;
HeWeatherCurrent currentWeatherClient;

#if DISPLAY_TYPE == 1
U8G2_ST7565_LM6059_F_4W_SW_SPI display(U8G2_R2, /* clock=*/ 14, /* data=*/ 12, /* cs=*/ 13, /* dc=*/ 15, /* reset=*/ 16); // U8G2_ST7565_LM6059_F_4W_SW_SPI
#endif

#if DISPLAY_TYPE == 2
U8G2_ST7565_64128N_F_4W_SW_SPI display(U8G2_R0, /* clock=*/ 14, /* data=*/ 12, /* cs=*/ 13, /* dc=*/ 15, /* reset=*/ 16); // U8G2_ST7565_64128N_F_4W_SW_SPI
#endif

#if DISPLAY_TYPE == 3
U8G2_ST7565_64128N_F_4W_SW_SPI display(U8G2_R2, /* clock=*/ 14, /* data=*/ 12, /* cs=*/ 13, /* dc=*/ 15, /* reset=*/ 16); // U8G2_ST7565_64128N_F_4W_SW_SPI
#endif

#if DISPLAY_TYPE == 4
U8G2_ST7920_128X64_F_SW_SPI display(U8G2_R2, /* clo  ck=*/ 14 /* A4 */ , /* data=*/ 12 /* A2 */, /* CS=*/ 16 /* A3 */, /* reset=*/ U8X8_PIN_NONE); // 16, U8X8_PIN_NONE
//#define BACKLIGHTPIN 15 // 2, 0
//#define LIGHT_SENSOR   // turn off for ST7565, turn on for ST7920 with BHV1750/GY-30/GY-302 light sensor
//#define LIGHT_SDA_PIN 0  // D3
//#define LIGHT_SCL_PIN  13 // D7
//BH1750 lightMeter(0x23);
#endif

time_t nowTime;
const String degree = String((char)176);
bool readyForWeatherUpdate = false;
long timeSinceLastWUpdate = 0;
float previousTemp = 0;
float previousHumidity = 0;
int lightLevel[10];

#define UPDATE_INTERVAL_SECS 1500


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
  initializeBackLightArray(lightLevel, BACKLIGHTPIN);
  adjustBacklightSub();

#if (DHTPIN >= 0)
  dht.begin();
#endif

  pinMode(RELAYPIN, OUTPUT);
  turnOff();

  listSPIFFSFiles(); // Lists the files so you can see what is in the SPIFFS

  display.begin();
  display.setFontPosTop();
  setContrastSub();

  display.clearBuffer();
  display.drawXBM(31, 0, 66, 64, garfield);
  display.sendBuffer();
  delay(1000);

  drawProgress(String(CompileDate), "Version: " + String(CURRENT_VERSION));
  delay(1000);

  drawProgress("Backlight Level", "Test");
  selfTestBacklight(BACKLIGHTPIN);

#ifdef USE_WIFI_MANAGER
  drawProgress("连接WIFI:", "IBECloc12864-HW");
#else
  drawProgress("连接WIFI中,", "请稍等...");
#endif

  connectWIFI(
#ifdef USE_WIFI_MANAGER
    true
#else
    false
#endif
  );

  if (WiFi.status() != WL_CONNECTED) ESP.restart();

  // Get time from network time service
#ifdef DEBUG
  Serial.println("WIFI Connected");
#endif
  drawProgress("连接WIFI成功,", "正在同步时间...");
  configTime(TZ_SEC, DST_SEC, NTP_SERVER);
  readValueWebSite(serialNumber, Location, Token, Resistor, dummyMode, backlightOffMode, sendAlarmEmail, alarmEmailAddress, displayContrast, displayMultiplier, displayBias, displayMinimumLevel, displayMaximumLevel, temperatureMultiplier, temperatureBias, humidityMultiplier, humidityBias, firmwareversion, firmwareBin);
  if (serialNumber < 0)
  {
    drawProgress("新MAC " + String(WiFi.macAddress()), "序列号: " + String(serialNumber));
    stopApp();
  }
  else if (serialNumber == 0)
  {
    drawProgress("多MAC " + String(WiFi.macAddress()), "找管理员处理");
    stopApp();
  }
  setContrastSub();
  drawProgress("Serial: " + String(serialNumber), "MAC: " + String(WiFi.macAddress()));
  delay(1500);
  Serial.print("MAC: ");
  Serial.println(String(WiFi.macAddress()));
  Serial.print("Serial: ");
  Serial.println(serialNumber);
  Serial.print("Location: ");
  Serial.println(Location);
  Serial.print("Token: ");
  Serial.println(Token);
  Serial.print("Resistor: ");
  Serial.println(Resistor);
  Serial.print("dummyMode: ");
  Serial.println(dummyMode);
  Serial.print("backlightOffMode: ");
  Serial.println(backlightOffMode);
  Serial.print("sendAlarmEmail: ");
  Serial.println(sendAlarmEmail);
  Serial.print("alarmEmailAddress: ");
  Serial.println(alarmEmailAddress);
  Serial.print("displayContrast: ");
  Serial.println(displayContrast);
  Serial.print("displayMultiplier: ");
  Serial.println(displayMultiplier);
  Serial.print("displayBias: ");
  Serial.println(displayBias);
  Serial.print("displayMinimumLevel: ");
  Serial.println(displayMinimumLevel);
  Serial.print("displayMaximumLevel: ");
  Serial.println(displayMaximumLevel);
  Serial.print("temperatureMultiplier: ");
  Serial.println(temperatureMultiplier);
  Serial.print("temperatureBias: ");
  Serial.println(temperatureBias);
  Serial.print("humidityMultiplier: ");
  Serial.println(humidityMultiplier);
  Serial.print("humidityBias: ");
  Serial.println(humidityBias);
  Serial.print("firmwareversion: ");
  Serial.println(firmwareversion);
  Serial.print("CURRENT_VERSION: ");
  Serial.println(CURRENT_VERSION);
  Serial.print("firmwareBin: ");
  Serial.println(SETTINGS_BASE_URL + SETTINGS_OTA_BIN_URL + firmwareBin);
  Serial.println("");
  writeBootWebSite(serialNumber);
  if (firmwareversion > CURRENT_VERSION)
  {
    drawProgress("自动升级中!", "请稍候......");
    Serial.println("Auto upgrade starting...");
    ESPhttpUpdate.rebootOnUpdate(false);
    t_httpUpdate_return ret = ESPhttpUpdate.update(SETTINGS_SERVER, 81, SETTINGS_BASE_URL + SETTINGS_OTA_BIN_URL + firmwareBin);
    Serial.println("Auto upgrade finished.");
    Serial.print("ret "); Serial.println(ret);
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        drawProgress("升级错误!", "重启!");
        delay(2000);
        ESP.restart();
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        drawProgress("无需升级!", "继续启动...");
        delay(1500);
        break;
      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        drawProgress("升级成功!", "重启...");
        delay(2000);
        ESP.restart();
        break;
      default:
        Serial.print("Undefined HTTP_UPDATE Code: "); Serial.println(ret);
        drawProgress("升级错误!", "重启!");
        delay(2000);
        ESP.restart();
    }
  }
  else
  {
    drawProgress("无需自动升级!", "继续启动...");
  }
  drawProgress("同步时间成功,", "正在更新天气数据...");
  updateData(true);
  timeSinceLastWUpdate = millis();
}

void setContrastSub() {
  if (displayContrast > 0)
  {
    display.setContrast(displayContrast);
    Serial.print("Set displayContrast to: ");
    Serial.println(displayContrast);
    Serial.println();
  }
}

void adjustBacklightSub() {
  adjustBacklight(lightLevel, BACKLIGHTPIN, displayBias, displayMultiplier);
}

void loop() {

  display.firstPage();
  do {
    drawLocal();
  } while ( display.nextPage() );

  if (millis() - timeSinceLastWUpdate > (1000 * UPDATE_INTERVAL_SECS)) {
    readyForWeatherUpdate = true;
    timeSinceLastWUpdate = millis();
  }

#if (DHTPIN >= 0)
  if (dht.read())
  {
    float fltHumidity = dht.readHumidity() * humidityMultiplier / 100 + humidityBias;
    float fltCTemp = dht.readTemperature() * temperatureMultiplier / 100 + temperatureBias;
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
#endif

  if (readyForWeatherUpdate) {
    updateData(false);
  }
}

void updateData(bool isInitialBoot) {
  nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);
  if (isInitialBoot)
  {
    drawProgress("正在更新...", "本地天气实况...");
  }
  currentWeatherClient.updateCurrent(&currentWeather, HEWEATHER_APP_ID, HEWEATHER_LOCATION, HEWEATHER_LANGUAGE);
  if (!isInitialBoot)
  {
    writeDataWebSite(serialNumber, previousTemp, previousHumidity, currentWeather.tmp, currentWeather.hum, 0);
  }
  readyForWeatherUpdate = false;
}

void drawProgress(String labelLine1, String labelLine2) {
  display.clearBuffer();
  display.enableUTF8Print();
  display.setFont(u8g2_font_wqy12_t_gb2312); // u8g2_font_wqy12_t_gb2312, u8g2_font_helvB08_tf
  int stringWidth = 1;
  if (labelLine1 != "")
  {
    stringWidth = display.getUTF8Width(string2char(labelLine1));
    display.setCursor((128 - stringWidth) / 2, 13);
    display.print(labelLine1);
  }
  if (labelLine2 != "")
  {
    stringWidth = display.getUTF8Width(string2char(labelLine2));
    display.setCursor((128 - stringWidth) / 2, 36);
    display.print(labelLine2);
  }
  display.disableUTF8Print();
  display.sendBuffer();
}

void drawLocal() {
  nowTime = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&nowTime);
  char buff[20];

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
    display.drawStr(98, 17, string2char(chooseMeteocon(currentWeather.iconMeteoCon)));
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


