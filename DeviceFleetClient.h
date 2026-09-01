#pragma once
#include <Arduino.h>

// Every field this device's per-unit config can carry, filled in by readSettings().
struct DeviceFleetSettings {
	int serialNumber = -1;
	String location = "Default";
	String token = "Token";
	int resistor = 80000;
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
	int firmwareVersion = 0;
	String firmwareBin = "";
};

// Client for a small, custom, plain-HTTP device-fleet backend - see this library's README
// for the exact endpoints it expects you to host. Two servers are involved:
//   - The "bootstrap" server: hosts one static file (bootstrapPath) listing the REAL
//     settings server's address - lets you repoint an entire fleet of already-deployed
//     devices at a new settings server without reflashing any of them. If unreachable,
//     falls back to the fallback server/settings you construct this with.
//   - The settings server: serves per-device settings (keyed by MAC address), accepts
//     boot-check-in and sensor-data-logging requests, and hosts OTA firmware binaries.
class DeviceFleetClient {
public:
	DeviceFleetClient(
		const char* bootstrapServer, int bootstrapPort, const char* bootstrapPath,
		const char* fallbackSettingsServer, int fallbackSettingsPort, const char* fallbackBaseUrl);

	// Fetches this device's settings (by MAC address) from the settings server, first
	// re-resolving the settings server's address from the bootstrap server. Leaves
	// `settings` unchanged (at its current/default values) if WiFi isn't connected or the
	// server is unreachable.
	void readSettings(DeviceFleetSettings &settings);

	// Tells the settings server this device just booted (e.g. for fleet-wide uptime/version
	// tracking). No-ops if WiFi isn't connected.
	void writeBootNotification(int serialNumber);

	// Logs a sensor reading to the settings server. No-ops if WiFi isn't connected.
	void writeSensorData(int serialNumber, int insideTemp, int insideHumidity, int outsideTemp, int outsideHumidity, int airQuality = 0);

	// Full OTA bin URL for the firmware named in the most recent readSettings() call -
	// pass this to ESPhttpUpdate.update() together with settingsServer()/settingsPort().
	String firmwareBinUrl(const String &firmwareBin) const;
	String settingsServer() const { return _settingsServer; }
	int settingsPort() const { return _settingsPort; }

private:
	const char* _bootstrapServer;
	int _bootstrapPort;
	const char* _bootstrapPath;
	const char* _fallbackBaseUrl;

	String _settingsServer;
	int _settingsPort;
	String _settingsBaseUrl;
	String _settingsReadSettingUrl = "readsetting.html?mac=";
	String _settingsWriteBootUrl = "writeboot.html?serial=";
	String _settingsWriteDataUrl = "writedata.html?serial=";
	String _settingsOtaBinUrl = "bin/";

	void resolveSettingsServer();
	String readHttpAsString(const String &host, int port, const String &path);
};

// Used on an unrecoverable device-config error (e.g. a MAC address the fleet backend
// doesn't recognize): waits 2 minutes (so the error is visible on-screen) then restarts.
void stopApp();
