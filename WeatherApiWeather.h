#pragma once
#include <JsonListener.h>
#include <JsonStreamingParser.h>

// Client for WeatherAPI.com's /v1/forecast.json endpoint - one HTTPS request
// returns both current conditions and a multi-day forecast. Get a free API
// key at https://www.weatherapi.com/ (see README.md for setup and usage).

typedef struct WeatherApiCurrentData {
	float temp_c;
	float temp_f;
	String text;   // condition description, e.g. "Partly cloudy"
	String code;   // condition code, see https://www.weatherapi.com/docs/weather_conditions.json
	uint8_t wind_kph;
	String wind_dir;
	uint8_t humidity;
	// Derived from `code` - see getMeteoconIcon().
	String iconMeteoCon;
} WeatherApiCurrentData;

typedef struct WeatherApiForecastData {
	String date;          // yyyy-MM-dd
	uint32_t date_epoch;
	float maxtemp_c;
	float mintemp_c;
	float totalprecip_mm;
	uint8_t avghumidity;
	uint8_t chanceOfRain;  // 0-100, WeatherAPI.com's daily_chance_of_rain
	float maxwind_kph;
	String text;
	String code;
	String iconMeteoCon;
} WeatherApiForecastData;

class WeatherApiWeather : public JsonListener {
private:
	String currentKey;

	WeatherApiCurrentData *data;
	WeatherApiForecastData *forecastData;
	uint8_t currentForecast;
	uint8_t maxForecasts;

	// Keys of the objects/arrays enclosing the current parse position, e.g.
	// forecast > forecastday > [element] > day > condition. Lets value() tell a day's
	// "code" from an hourly one, which share the same key names.
	static const uint8_t MaxDepth = 10;
	String path[MaxDepth];
	uint8_t depth = 0;
	void push(const String &key);
	String pop();
	bool inside(const char *key) const;

	uint8_t doUpdate(WeatherApiCurrentData *data, WeatherApiForecastData *forecastData, String url);

public:
	WeatherApiWeather();

	// apiKey: your WeatherAPI.com key. location: city name, "lat,lon", zip code, etc.
	// (see https://www.weatherapi.com/docs/#intro-request). language: ISO language
	// code for condition text, e.g. "en", "zh". maxForecasts: size of the
	// forecastData array you're passing in (1-14, WeatherAPI.com free tier
	// currently supports up to 3 days - check your plan's limit).
	uint8_t updateWeather(WeatherApiCurrentData *data, WeatherApiForecastData *forecastData, String apiKey, String location, String language, uint8_t maxForecasts);

	// Maps a WeatherAPI.com condition code to a Meteocons font glyph pair
	// (day-icon, night-icon). Values from the community-maintained mapping
	// used across several ESP8266 weather station projects.
	String getMeteoconIcon(String code);

	static String urlEncode(const String &value);

	virtual void whitespace(char c);
	virtual void startDocument();
	virtual void key(String key);
	virtual void value(String value);
	virtual void endArray();
	virtual void endObject();
	virtual void endDocument();
	virtual void startArray();
	virtual void startObject();
};
