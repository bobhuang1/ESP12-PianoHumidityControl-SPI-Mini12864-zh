#include "DeviceFleetClient.h"
#include <ESP8266WiFi.h>

// Spoofs a real browser's User-Agent/Accept headers - some shared-hosting backends behind
// a WAF reject requests that don't look like they came from a browser. Harmless but a bit
// unusual; drop this if your own backend doesn't need it.
static const char USER_AGENT[] =
	"\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/85.0.4183.121 Safari/537.36 Edg/85.0.564.63"
	"\r\nAccept-Language: en-US,en;q=0.9"
	"\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
	"\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n";

static bool parseBool01(const String &input) {
	String trimmed = input;
	trimmed.trim();
	return trimmed == "1";
}

DeviceFleetClient::DeviceFleetClient(
	const char* bootstrapServer, int bootstrapPort, const char* bootstrapPath,
	const char* fallbackSettingsServer, int fallbackSettingsPort, const char* fallbackBaseUrl)
	: _bootstrapServer(bootstrapServer), _bootstrapPort(bootstrapPort), _bootstrapPath(bootstrapPath),
	  _fallbackBaseUrl(fallbackBaseUrl),
	  _settingsServer(fallbackSettingsServer), _settingsPort(fallbackSettingsPort), _settingsBaseUrl(fallbackBaseUrl) {
}

String DeviceFleetClient::readHttpAsString(const String &host, int port, const String &path) {
	if (WiFi.status() != WL_CONNECTED) return "";

	WiFiClient client;
	int retryCounter = 0;
	while (!client.connect(host, port)) {
		delay(1000);
		if (++retryCounter > 10) {
			client.stop();
			return "";
		}
	}

	client.print(String("GET ") + path + " HTTP/1.0\r\nHost: " + host + USER_AGENT);

	unsigned long timeout = millis();
	while (client.available() == 0) {
		if (millis() - timeout > 30000) {
			client.stop();
			return "";
		}
	}

	client.setTimeout(30000);
	String statusLine = client.available() ? client.readStringUntil('\n') : "";

	// Matches " 200 " generically rather than a specific HTTP version string - the
	// original checked for "1.1 200" even though it sends an HTTP/1.0 request, which only
	// worked because most servers respond 1.1 regardless of what the client asked for.
	if (statusLine.indexOf(" 200 ") < 0) {
		client.stop();
		return "";
	}

	String body;
	while (client.available()) {
		body += client.readString();
	}
	body.replace("\r", "");
	int headerEnd = body.indexOf("\n\n");
	body = body.substring(headerEnd);
	body.trim();

	client.stop();
	return body;
}

void DeviceFleetClient::resolveSettingsServer() {
	String response = readHttpAsString(_bootstrapServer, _bootstrapPort, _bootstrapPath);
	if (response.length() == 0) return; // keep fallback values

	// Expected format: one value per line - server, port, baseUrl, readSettingUrl,
	// readTimerUrl (unused, kept for backward file-format compatibility), writeBootUrl,
	// writeDataUrl, otaBinUrl.
	String fields[8];
	int previousEnd = 0;
	for (int i = 0; i < 8; ++i) {
		int end = response.indexOf('\n', previousEnd + 1);
		String field = (i == 0) ? response.substring(previousEnd, end) : response.substring(previousEnd + 1, end);
		field.replace("\n", "");
		field.trim();
		fields[i] = field;
		previousEnd = end;
	}

	_settingsServer = fields[0];
	_settingsPort = fields[1].toInt();
	_settingsBaseUrl = fields[2];
	_settingsReadSettingUrl = fields[3];
	// fields[4] (readTimerUrl) intentionally unused
	_settingsWriteBootUrl = fields[5];
	_settingsWriteDataUrl = fields[6];
	_settingsOtaBinUrl = fields[7];
}

void DeviceFleetClient::readSettings(DeviceFleetSettings &settings) {
	if (WiFi.status() != WL_CONNECTED) return;

	resolveSettingsServer();

	String macAddress = WiFi.macAddress();
	String url = _settingsBaseUrl + _settingsReadSettingUrl + macAddress;
	String response = readHttpAsString(_settingsServer, _settingsPort, url);

	const int FIELD_COUNT = 20;
	String previousLine;
	int previousEnd = 0;
	for (int i = 0; i < FIELD_COUNT; ++i) {
		int end = response.indexOf('\n', previousEnd + 1);
		String field = (i == 0) ? response.substring(previousEnd, end) : response.substring(previousEnd + 1, end);
		field.replace("\n", "");
		field.trim();
		previousEnd = end;

		switch (i) {
			case 0: settings.serialNumber = field.toInt(); break;
			case 1: settings.location = field; break;
			case 2: settings.token = field; break;
			case 3: settings.resistor = field.toInt(); break;
			case 4: settings.displayContrast = field.toInt(); break;
			case 5: settings.displayBias = field.toInt(); break;
			case 6: settings.dummyMode = parseBool01(field); break;
			case 7: settings.backlightOffMode = parseBool01(field); break;
			case 8: settings.sendAlarmEmail = parseBool01(field); break;
			case 9: settings.alarmEmailAddress = field; break;
			case 10: settings.displayMultiplier = field.toInt(); break;
			case 11: settings.displayMinimumLevel = field.toInt(); break;
			case 12: settings.displayMaximumLevel = field.toInt(); break;
			case 13: settings.temperatureMultiplier = field.toInt(); break;
			case 14: settings.temperatureBias = field.toInt(); break;
			case 15: settings.humidityMultiplier = field.toInt(); break;
			case 16: settings.humidityBias = field.toInt(); break;
			// index 17 intentionally unused (reserved in the field layout)
			case 18: settings.firmwareVersion = field.toInt(); break;
			case 19: settings.firmwareBin = field; break;
		}
	}
}

void DeviceFleetClient::writeBootNotification(int serialNumber) {
	if (WiFi.status() != WL_CONNECTED) return;
	String url = _settingsBaseUrl + _settingsWriteBootUrl + String(serialNumber);
	readHttpAsString(_settingsServer, _settingsPort, url);
}

void DeviceFleetClient::writeSensorData(int serialNumber, int insideTemp, int insideHumidity, int outsideTemp, int outsideHumidity, int airQuality) {
	if (WiFi.status() != WL_CONNECTED) return;
	String url = _settingsBaseUrl + _settingsWriteDataUrl + String(serialNumber) +
		"&int=" + String(insideTemp) + "&inh=" + String(insideHumidity) +
		"&outt=" + String(outsideTemp) + "&outh=" + String(outsideHumidity) + "&air=" + String(airQuality);
	readHttpAsString(_settingsServer, _settingsPort, url);
}

String DeviceFleetClient::firmwareBinUrl(const String &firmwareBin) const {
	return _settingsBaseUrl + _settingsOtaBinUrl + firmwareBin;
}

void stopApp() {
	delay(120000);
	ESP.restart();
}
