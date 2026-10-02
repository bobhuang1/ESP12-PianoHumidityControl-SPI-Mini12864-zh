#include "WiFiMultiConnect.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

const char CompileDate[] = __DATE__ " " __TIME__;
const char DefaultNtpServer[] = "pool.ntp.org";

bool connectWiFi(const char* const ssids[], const char* const passwords[], uint8_t count, int maxTriesPerNetwork, unsigned long totalTimeoutMs) {
	if (count == 0) return false;
	const unsigned long startedAt = millis();
	WiFi.persistent(false);
	WiFi.mode(WIFI_STA);
	WiFi.disconnect();

	uint8_t preferredIndex = 0;
	int scannedCount = WiFi.scanNetworks();
	for (int i = 0; i < scannedCount; ++i) {
		for (uint8_t j = 0; j < count; ++j) {
			if (WiFi.SSID(i) == String(ssids[j])) {
				preferredIndex = j;
				break;
			}
		}
	}

	WiFi.persistent(true);
	WiFi.begin(ssids[preferredIndex], passwords[preferredIndex]);

	uint8_t currentIndex = preferredIndex;
	while (WiFi.status() != WL_CONNECTED) {
		for (int attempt = 0; attempt < maxTriesPerNetwork && WiFi.status() != WL_CONNECTED; ++attempt) {
			if (totalTimeoutMs > 0 && millis() - startedAt >= totalTimeoutMs) return false;
			delay(1000);
		}
		if (WiFi.status() == WL_CONNECTED) break;

		currentIndex = (currentIndex + 1) % count;
		WiFi.begin(ssids[currentIndex], passwords[currentIndex]);
	}
	return true;
}

bool connectWiFiWithManager(const char* apName, uint16_t configPortalTimeoutSeconds) {
	WiFi.persistent(true);
	WiFiManager wifiManager;
	wifiManager.setConfigPortalTimeout(configPortalTimeoutSeconds);
	return wifiManager.autoConnect(apName);
}
