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
	String text;
	String code;
	String iconMeteoCon;
} WeatherApiForecastData;

class WeatherApiWeather : public JsonListener {
private:
	String currentKey;
	String currentParent;
	WeatherApiCurrentData *data;
	WeatherApiForecastData *forecastData;
	uint8_t currentForecast;
	uint8_t maxForecasts;
	uint8_t currentFinished = 0;
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
