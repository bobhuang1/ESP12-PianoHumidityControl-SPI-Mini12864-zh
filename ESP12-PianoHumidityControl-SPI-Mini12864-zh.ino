#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h> // was <ESPHTTPClient.h>, a header that no longer exists in any current core
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
#include "WeatherApiWeather.h"
#include "StringHelpers.h"
#include "BacklightController.h"
#include "WiFiMultiConnect.h"
#include "WeatherDisplayHelpers.h"
#include "DeviceFleetClient.h"
#include "BootSplashBitmap.h"

#define CURRENT_VERSION 4
//#define DEBUG
//#define USE_WIFI_MANAGER     // disable to NOT use WiFi manager, enable to use
#define DISPLAY_TYPE 2   // 1-BIG 12864, 2-MINI 12864, 3-New Big BLUE 12864, to use 3, you must change u8x8_d_st7565.c as well!!!, 4- New BLUE 12864-ST7920
#define LANGUAGE_CN  // LANGUAGE_CN or LANGUAGE_EN

// Fill in your own SSID/password pairs (or better, use USE_WIFI_MANAGER above
// instead of hardcoding any of this). Never commit real WiFi credentials.
const char* const WIFI_SSIDS[] = {"YOUR_SSID_1", "YOUR_SSID_2", "YOUR_SSID_3"};
const char* const WIFI_PASSWORDS[] = {"YOUR_PASSWORD_1", "YOUR_PASSWORD_2", "YOUR_PASSWORD_3"};

const String WEATHERAPI_APP_ID = "YOUR_WEATHERAPI_COM_KEY"; // https://www.weatherapi.com/

// See DeviceFleetClient's README for what these servers need to implement - replace with
// your own device-fleet backend, or remove the fleet-client calls below if you don't need one.
DeviceFleetClient fleet(
	"your-bootstrap-server.example.com", 80, "/iot.txt",
	"your-settings-server.example.com", 81, "/IOT/");
DeviceFleetSettings settings;

// BIN files: 1300.bin

#define DHTTYPE  AM2301       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   2 // 2, -1
#define RELAYPIN 5
#define BACKLIGHTPIN 0 // 2, 0

#define MAXHUMIDITY 50
#define SENSOR_TIMEOUT_MS (5UL * 60UL * 1000UL) // no valid reading this long -> relay OFF (fail safe)
#define WIFI_TIMEOUT_MS   (60UL * 1000UL)        // give up on WiFi at boot and run offline
#define HYSTERESIS 5  // relay switches OFF only this far below MAXHUMIDITY (45%) -
                      // prevents chattering when readings hover at the threshold

#if DISPLAY_TYPE == 3
#define BIGBLUE12864
#endif

#ifdef LANGUAGE_CN
const String WEATHERAPI_LANGUAGE = "zh"; // zh for Chinese, en for English
#else ifdef LANGUAGE_EN // NOTE: '#else ifdef' is not valid preprocessor; use plain '#else'
const String WEATHERAPI_LANGUAGE = "en"; // zh for Chinese, en for English
#endif

#ifdef USE_WIFI_MANAGER
const String WEATHERAPI_LOCATION = "auto:ip"; // WeatherAPI.com: resolve location from the request's IP address
#else
const String WEATHERAPI_LOCATION = "YOUR_CITY"; // e.g. "London", "New York", or "lat,lon" - see WeatherAPI.com docs
#endif

#ifdef LANGUAGE_CN
const String WDAY_NAMES[] = { "星期天", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六" };
#else ifdef LANGUAGE_EN // NOTE: '#else ifdef' is not valid preprocessor; use plain '#else'
const String WDAY_NAMES[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
#endif

#if (DHTPIN >= 0)
DHT dht(DHTPIN, DHTTYPE);
#endif

WeatherApiCurrentData currentWeather;
WeatherApiForecastData weatherForecastUnused[1]; // this sketch only displays current conditions
WeatherApiWeather weatherClient;

#if DISPLAY_TYPE == 1
U8G2_ST7565_LM6059_F_4W_SW_SPI display(U8G2_R2, /* clock=*/ 14, /* data=*/ 12, /* cs=*/ 13, /* dc=*/ 15, /* reset=*/ 16); // U8G2_ST7565_LM6059_F_4W_SW_SPI
#endif

#if DISPLAY_TYPE == 2
U8G2_ST7565_64128N_F_4W_SW_SPI display(U8G2_R0, /* clock=*/ 14, /* data=*/ 12, /* cs=*/ 13, /* dc=*/ 15, /* reset=*/ 16); // U8G2_ST7565_64128N_F_4W_SW_SPI, u8x8_d_st7565.c U8X8_C(0x060)
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
unsigned long lastValidReadingMs = 0;   // millis() of the last good DHT reading
bool sensorFault = false;               // relay forced OFF because the sensor went silent
bool fleetOk = false;                   // settings came from the fleet server and the device is registered
BacklightController backlight;

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
  backlight.begin(BACKLIGHTPIN);
  adjustBacklightSub();

#if (DHTPIN >= 0)
  dht.begin();
#endif

  pinMode(RELAYPIN, OUTPUT);
  turnOff();

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
  backlight.selfTest();

  // Humidity control must never depend on the network: WiFi, the weather API and the
  // fleet server are extras. Without them the device keeps controlling the relay with the
  // compiled-in defaults.
#ifdef USE_WIFI_MANAGER
  drawProgress("连接WIFI:", "ESP8266-Setup");
  bool wifiOk = connectWiFiWithManager("ESP8266-Setup", 180);
#else
  drawProgress("连接WIFI中,", "请稍等...");
  bool wifiOk = connectWiFi(WIFI_SSIDS, WIFI_PASSWORDS, 3, 30, WIFI_TIMEOUT_MS);
#endif

  if (!wifiOk)
  {
    drawProgress("WIFI连接失败", "离线运行湿度控制");
    delay(2000);
    lastValidReadingMs = millis();
    return;
  }

  // Get time from network time service
#ifdef DEBUG
  Serial.println("WIFI Connected");
#endif
  drawProgress("连接WIFI成功,", "正在同步时间...");
  configTime(TZ_SEC_FOR(8), DST_SEC_FOR(0), DefaultNtpServer);
  // An unreachable server leaves the defaults in place (readSettings returns false). An
  // unregistered device shows why, then still runs the humidity control; it just skips
  // fleet logging and OTA.
  fleetOk = fleet.readSettings(settings);
  if (fleetOk && settings.serialNumber < 0)
  {
    drawProgress("新MAC " + String(WiFi.macAddress()), "序列号: " + String(settings.serialNumber));
    delay(5000);
    fleetOk = false;
  }
  else if (fleetOk && settings.serialNumber == 0)
  {
    drawProgress("多MAC " + String(WiFi.macAddress()), "找管理员处理");
    delay(5000);
    fleetOk = false;
  }
  setContrastSub();
  drawProgress("Serial: " + String(settings.serialNumber), "MAC: " + String(WiFi.macAddress()));
  delay(1500);
  Serial.print("MAC: ");
  Serial.println(String(WiFi.macAddress()));
  Serial.print("Serial: ");
  Serial.println(settings.serialNumber);
  Serial.print("Location: ");
  Serial.println(settings.location);
  Serial.print("Token: ");
  Serial.println(settings.token);
  Serial.print("Resistor: ");
  Serial.println(settings.resistor);
  Serial.print("dummyMode: ");
  Serial.println(settings.dummyMode);
  Serial.print("backlightOffMode: ");
  Serial.println(settings.backlightOffMode);
  Serial.print("sendAlarmEmail: ");
  Serial.println(settings.sendAlarmEmail);
  Serial.print("alarmEmailAddress: ");
  Serial.println(settings.alarmEmailAddress);
  Serial.print("displayContrast: ");
  Serial.println(settings.displayContrast);
  Serial.print("displayMultiplier: ");
  Serial.println(settings.displayMultiplier);
  Serial.print("displayBias: ");
  Serial.println(settings.displayBias);
  Serial.print("displayMinimumLevel: ");
  Serial.println(settings.displayMinimumLevel);
  Serial.print("displayMaximumLevel: ");
  Serial.println(settings.displayMaximumLevel);
  Serial.print("temperatureMultiplier: ");
  Serial.println(settings.temperatureMultiplier);
  Serial.print("temperatureBias: ");
  Serial.println(settings.temperatureBias);
  Serial.print("humidityMultiplier: ");
  Serial.println(settings.humidityMultiplier);
  Serial.print("humidityBias: ");
  Serial.println(settings.humidityBias);
  Serial.print("firmwareVersion: ");
  Serial.println(settings.firmwareVersion);
  Serial.print("CURRENT_VERSION: ");
  Serial.println(CURRENT_VERSION);
  Serial.print("firmwareBin: ");
  Serial.println(fleet.firmwareBinUrl(settings.firmwareBin));
  Serial.println("");
  if (fleetOk) fleet.writeBootNotification(settings.serialNumber);
  if (fleetOk && settings.firmwareVersion > CURRENT_VERSION)
  {
    drawProgress("自动升级中!", "请稍候......");
    Serial.println("Auto upgrade starting...");
    ESPhttpUpdate.rebootOnUpdate(false);
    // ESP8266 core 3.x removed the update(host, port, path) overload - use the
    // URL form with an explicit (plain-HTTP) client, as the core requires.
    WiFiClient otaClient;
    String otaUrl = "http://" + fleet.settingsServer() + ":" + String(fleet.settingsPort()) + fleet.firmwareBinUrl(settings.firmwareBin);
    t_httpUpdate_return ret = ESPhttpUpdate.update(otaClient, otaUrl);
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
  lastValidReadingMs = millis();
}

void setContrastSub() {
  if (settings.displayContrast > 0)
  {
    display.setContrast(settings.displayContrast);
    Serial.print("Set displayContrast to: ");
    Serial.println(settings.displayContrast);
    Serial.println();
  }
}

void adjustBacklightSub() {
  backlight.update(settings.displayBias, settings.displayMultiplier);
}

void loop() {

  adjustBacklightSub(); // auto-dim from the photoresistor (previously only ran once in setup())

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
    float fltHumidity = dht.readHumidity() * settings.humidityMultiplier / 100 + settings.humidityBias;
    float fltCTemp = dht.readTemperature() * settings.temperatureMultiplier / 100 + settings.temperatureBias;
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
      lastValidReadingMs = millis();
      sensorFault = false;
      previousTemp = fltCTemp;
      if (fltHumidity <= 100)
      {
        previousHumidity = fltHumidity;
      }
      else
      {
        previousHumidity = 100;
      }
      // Relay hysteresis: ON at/above MAXHUMIDITY, OFF only after humidity falls
      // below MAXHUMIDITY - HYSTERESIS. Inside the band the relay keeps its
      // previous state, so sensor noise at the threshold cannot chatter it.
      if (previousHumidity > MAXHUMIDITY)
      {
        turnOn();
      }
      else if (previousHumidity <= MAXHUMIDITY - HYSTERESIS)
      {
        turnOff();
      }
    }
  }
  // Fail safe: if the sensor has not produced a valid reading for SENSOR_TIMEOUT_MS
  // (unplugged, corroded, dead), switch the relay OFF instead of leaving a heater or
  // dehumidifier running inside the piano on the last known value.
  if (millis() - lastValidReadingMs > SENSOR_TIMEOUT_MS)
  {
    if (!sensorFault)
    {
      Serial.println("Humidity sensor silent - relay forced OFF");
    }
    sensorFault = true;
    turnOff();
    previousTemp = 0;
    previousHumidity = 0;
  }
#endif

  if (readyForWeatherUpdate && WiFi.status() == WL_CONNECTED) {
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
  weatherClient.updateWeather(&currentWeather, weatherForecastUnused, WEATHERAPI_APP_ID, WEATHERAPI_LOCATION, WEATHERAPI_LANGUAGE, 1);
  if (!isInitialBoot && fleetOk)
  {
    fleet.writeSensorData(settings.serialNumber, previousTemp, previousHumidity, (int)currentWeather.temp_c, currentWeather.humidity, 0);
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
  stringWidth = display.getUTF8Width(string2char(String(currentWeather.text)));
  display.setCursor((128 - stringWidth) / 2, 40);
  display.print(String(currentWeather.text));
  String WindDirectionAndSpeed = translateWindDirectionToChinese(currentWeather.wind_dir) + String(currentWeather.wind_kph) + "km/h";
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
    display.drawStr(98, 17, string2char(chooseMeteoconChar(currentWeather.iconMeteoCon)));
  }


  display.setFont(u8g2_font_helvR08_tf);
  String temp = String(currentWeather.temp_c, 0) + degree + "C";
  display.drawStr(0, 53, string2char(temp));

  display.setFont(u8g2_font_helvR08_tf);
  stringWidth = display.getStrWidth(string2char((String(currentWeather.humidity) + "%")));
  display.drawStr(127 - stringWidth, 53, string2char((String(currentWeather.humidity) + "%")));

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
