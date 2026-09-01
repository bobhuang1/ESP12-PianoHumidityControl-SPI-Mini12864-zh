#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

// The "Meteocons" bitmap fonts (u8g2 font format), in two sizes, mapping ASCII letters to
// weather-condition glyphs. Select one with display.setFont(Meteocon21) / Meteocon36.
extern const uint8_t Meteocon21[1750] U8G2_FONT_SECTION("Meteocon21");
extern const uint8_t Meteocon36[3312] U8G2_FONT_SECTION("Meteocon36");

// Expects a 2-character string: the day glyph followed by the night glyph
// (see esp8266-weather-WeatherApi for how such pairs are produced from a
// weather condition code). Picks whichever glyph is appropriate for the
// current local time (day: 6:00-18:00).
String chooseMeteoconChar(String dayNightIconPair);

// Translates N/S/E/W (and the "no sustained wind" placeholder) to Chinese,
// for Chinese-language weather displays.
String translateWindDirectionToChinese(String windDirection);
