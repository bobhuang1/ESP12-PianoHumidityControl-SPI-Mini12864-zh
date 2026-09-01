#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include "WeatherApiWeather.h"

WeatherApiWeather::WeatherApiWeather() {
}

uint8_t WeatherApiWeather::updateWeather(WeatherApiCurrentData *data, WeatherApiForecastData *forecastData, String apiKey, String location, String language, uint8_t maxForecasts) {
	this->maxForecasts = maxForecasts;
	this->currentFinished = 0;
	return doUpdate(data, forecastData, "https://api.weatherapi.com/v1/forecast.json?key=" + apiKey + "&q=" + location + "&days=" + maxForecasts + "&lang=" + language);
}

uint8_t WeatherApiWeather::doUpdate(WeatherApiCurrentData *data, WeatherApiForecastData *forecastData, String url) {
	if (WiFi.status() != WL_CONNECTED) return 0;
	this->currentForecast = 0;
	this->data = data;
	this->forecastData = forecastData;
	JsonStreamingParser parser;
	parser.setListener(this);
	Serial.printf("Getting url: %s\n", url.c_str());

	// NOTE: setInsecure() skips TLS certificate validation. This trades a
	// (small, in-transit-only) MITM risk for not having to track and update a
	// pinned certificate/fingerprint every time WeatherAPI.com renews its TLS
	// certificate - a hardcoded fingerprint approach is what this library
	// used against its previous, since-shut-down weather provider, and it
	// silently broke every time that provider rotated its certificate. If you
	// need certificate validation, use client.setTrustAnchors() with
	// WeatherAPI.com's current root CA instead.
	WiFiClientSecure client;
	client.setInsecure();

	HTTPClient http;
	http.begin(client, url);

	int httpCode = http.GET();
	Serial.printf("[HTTP] GET... code: %d\n", httpCode);

	if (httpCode == HTTP_CODE_OK) {
		WiFiClient *stream = http.getStreamPtr();
		bool isBody = false;
		char c;
		int size;
		unsigned long lastByteAt = millis();
		const unsigned long readTimeoutMs = 15000UL;

		while (http.connected()) {
			while ((size = stream->available()) > 0) {
				c = stream->read();
				lastByteAt = millis();
				if (c == '{' || c == '[') isBody = true;
				if (isBody) parser.parse(c);
			}
			if (millis() - lastByteAt > readTimeoutMs) {
				Serial.println("Timed out waiting for response body.");
				break;
			}
		}
	} else {
		Serial.printf("[HTTP] GET failed: %s\n", http.errorToString(httpCode).c_str());
	}

	http.end();
	this->data = nullptr;
	this->forecastData = nullptr;
	return currentForecast;
}

void WeatherApiWeather::whitespace(char c) {
}

void WeatherApiWeather::startDocument() {
}

void WeatherApiWeather::key(String key) {
	currentKey = String(key);
}

void WeatherApiWeather::value(String value) {
	if (currentFinished == 0) {
		if (currentKey == "temp_c") {
			this->data->temp_c = value.toFloat();
		}
		if (currentKey == "temp_f") {
			this->data->temp_f = value.toFloat();
		}
		if (currentKey == "text") {
			this->data->text = value;
		}
		if (currentKey == "code") {
			this->data->code = value;
			this->data->iconMeteoCon = getMeteoconIcon(value);
		}
		if (currentKey == "wind_kph") {
			this->data->wind_kph = value.toInt();
		}
		if (currentKey == "wind_dir") {
			this->data->wind_dir = value;
		}
		if (currentKey == "humidity") {
			this->data->humidity = value.toInt();
			currentFinished = 1;
			currentForecast = 0;
		}
	}

	if (currentFinished == 1 && currentForecast < maxForecasts) {
		if (currentKey == "date") {
			forecastData[currentForecast].date = value;
		}
		if (currentKey == "date_epoch") {
			forecastData[currentForecast].date_epoch = value.toInt();
		}
		if (currentKey == "maxtemp_c") {
			forecastData[currentForecast].maxtemp_c = value.toFloat();
		}
		if (currentKey == "mintemp_c") {
			forecastData[currentForecast].mintemp_c = value.toFloat();
		}
		if (currentKey == "totalprecip_mm") {
			forecastData[currentForecast].totalprecip_mm = value.toFloat();
		}
		if (currentKey == "avghumidity") {
			forecastData[currentForecast].avghumidity = value.toInt();
		}
		if (currentKey == "text") {
			forecastData[currentForecast].text = value;
		}
		if (currentKey == "code") {
			forecastData[currentForecast].code = value;
			forecastData[currentForecast].iconMeteoCon = getMeteoconIcon(value);
			currentForecast++;
		}
	}
}

void WeatherApiWeather::endArray() {
}

void WeatherApiWeather::startObject() {
	currentParent = currentKey;
}

void WeatherApiWeather::endObject() {
}

void WeatherApiWeather::endDocument() {
}

void WeatherApiWeather::startArray() {
}

String WeatherApiWeather::getMeteoconIcon(String code) {
	if (code == "1000") { return "B2"; }      // Sunny / Clear
	if (code == "1003") { return "HI"; }      // Partly cloudy
	if (code == "1006") { return "N5"; }      // Cloudy
	if (code == "1009") { return "Y%"; }      // Overcast
	if (code == "1030") { return "M9"; }      // Mist
	if (code == "1063") { return "Q7"; }      // Patchy rain possible
	if (code == "1066") { return "U\""; }     // Patchy snow possible
	if (code == "1069") { return "V\""; }     // Patchy sleet possible
	if (code == "1072") { return "Q7"; }      // Patchy freezing drizzle possible
	if (code == "1087") { return "P6"; }      // Thundery outbreaks possible
	if (code == "1114") { return "U\""; }     // Blowing snow
	if (code == "1117") { return "S9"; }      // Blizzard
	if (code == "1135") { return "JK"; }      // Fog
	if (code == "1147") { return "JK"; }      // Freezing fog
	if (code == "1150") { return "Q7"; }      // Patchy light drizzle
	if (code == "1153") { return "Q7"; }      // Light drizzle
	if (code == "1168") { return "Q7"; }      // Freezing drizzle
	if (code == "1171") { return "R8"; }      // Heavy freezing drizzle
	if (code == "1180") { return "Q7"; }      // Patchy light rain
	if (code == "1183") { return "Q7"; }      // Light rain
	if (code == "1186") { return "R8"; }      // Moderate rain at times
	if (code == "1189") { return "R8"; }      // Moderate rain
	if (code == "1192") { return "X$"; }      // Heavy rain at times
	if (code == "1195") { return "X$"; }      // Heavy rain
	if (code == "1198") { return "Q7"; }      // Light freezing rain
	if (code == "1201") { return "R8"; }      // Moderate or heavy freezing rain
	if (code == "1204") { return "Q7"; }      // Light sleet
	if (code == "1207") { return "R8"; }      // Moderate or heavy sleet
	if (code == "1210") { return "U\""; }     // Patchy light snow
	if (code == "1213") { return "U\""; }     // Light snow
	if (code == "1216") { return "W#"; }      // Patchy moderate snow
	if (code == "1219") { return "W#"; }      // Moderate snow
	if (code == "1222") { return "W#"; }      // Patchy heavy snow
	if (code == "1225") { return "W#"; }      // Heavy snow
	if (code == "1237") { return "GG"; }      // Ice pellets
	if (code == "1240") { return "Q7"; }      // Light rain shower
	if (code == "1243") { return "R8"; }      // Moderate or heavy rain shower
	if (code == "1246") { return "Q7"; }      // Torrential rain shower
	if (code == "1249") { return "Q7"; }      // Light sleet showers
	if (code == "1252") { return "R8"; }      // Moderate or heavy sleet showers
	if (code == "1255") { return "U\""; }     // Light snow showers
	if (code == "1258") { return "W#"; }      // Moderate or heavy snow showers
	if (code == "1261") { return "GG"; }      // Light showers of ice pellets
	if (code == "1264") { return "GG"; }      // Moderate or heavy showers of ice pellets
	if (code == "1273") { return "Z&"; }      // Patchy light rain with thunder
	if (code == "1276") { return "Z&"; }      // Moderate or heavy rain with thunder
	if (code == "1279") { return "U\""; }     // Patchy light snow with thunder
	if (code == "1282") { return "W#"; }      // Moderate or heavy snow with thunder
	return "))"; // Nothing matched: N/A
}
