#include "BacklightController.h"

void BacklightController::begin(int backlightPin, int lightSensorPin) {
	_backlightPin = backlightPin;
	_lightSensorPin = lightSensorPin;
	pinMode(_backlightPin, OUTPUT);
	for (int i = 0; i < 10; ++i) {
		_lightLevel[i] = analogRead(_lightSensorPin);
	}
}

void BacklightController::update(int biasLevel, int dynamicLevel) {
	for (int i = 0; i < 9; ++i) {
		_lightLevel[i] = _lightLevel[i + 1]; // shift forward
	}
	_lightLevel[9] = analogRead(_lightSensorPin); // 0 = very bright, 200+ = dark

	int lightLevelSum = 0;
	for (int i = 0; i < 10; ++i) {
		lightLevelSum += _lightLevel[i];
	}
	lightLevelSum = (lightLevelSum / 10 - 50 + biasLevel) * dynamicLevel / 100;

	if (lightLevelSum < 10) analogWrite(_backlightPin, 1023);
	else if (lightLevelSum < 15) analogWrite(_backlightPin, 1000);
	else if (lightLevelSum < 20) analogWrite(_backlightPin, 900);
	else if (lightLevelSum < 25) analogWrite(_backlightPin, 800);
	else if (lightLevelSum < 30) analogWrite(_backlightPin, 700);
	else if (lightLevelSum < 35) analogWrite(_backlightPin, 600);
	else if (lightLevelSum < 40) analogWrite(_backlightPin, 500);
	else if (lightLevelSum < 45) analogWrite(_backlightPin, 400);
	else if (lightLevelSum < 50) analogWrite(_backlightPin, 300);
	else if (lightLevelSum < 60) analogWrite(_backlightPin, 200);
	else if (lightLevelSum < 75) analogWrite(_backlightPin, 100);
	else if (lightLevelSum < 100) analogWrite(_backlightPin, 75);
	else if (lightLevelSum < 150) analogWrite(_backlightPin, 50);
	else if (lightLevelSum < 250) analogWrite(_backlightPin, 25);
	else if (lightLevelSum < 400) analogWrite(_backlightPin, 12);
	else if (lightLevelSum < 600) analogWrite(_backlightPin, 6);
	else analogWrite(_backlightPin, 3);
}

void BacklightController::turnOff(int minimumBacklightLevel) {
	analogWrite(_backlightPin, minimumBacklightLevel);
}

void BacklightController::selfTest() {
	const int levels[] = {5, 10, 20, 50, 100, 200, 400, 600, 800, 1000, 1023};
	for (int level : levels) {
		analogWrite(_backlightPin, level);
		delay(level == 1023 ? 500 : 100);
	}
}
