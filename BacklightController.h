#pragma once
#include <Arduino.h>

// Auto-dims a PWM backlight pin based on a photoresistor wired to an analog pin (A0 on
// most ESP8266 boards, the only ADC pin available), using a rolling 10-sample average to
// smooth out noise.
class BacklightController {
public:
	// Call once in setup(). Seeds the rolling average from the current light level so the
	// first update() isn't skewed by startup transients.
	void begin(int backlightPin, int lightSensorPin = A0);

	// Call every loop() iteration (or on a timer). biasLevel shifts the light-level
	// threshold (try 50-200 to tune for your specific photoresistor/enclosure);
	// dynamicLevel scales the result (100 = unscaled).
	void update(int biasLevel = 0, int dynamicLevel = 100);

	// Forces the backlight to a fixed minimum level, bypassing the sensor (e.g. for a
	// scheduled "lights out" period).
	void turnOff(int minimumBacklightLevel);

	// Ramps the backlight through its full range once, for a visual power-on self-test.
	void selfTest();

private:
	int _backlightPin = -1;
	int _lightSensorPin = A0;
	int _lightLevel[10] = {0};
};
