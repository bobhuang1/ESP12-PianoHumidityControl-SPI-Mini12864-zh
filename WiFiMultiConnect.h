#pragma once
#include <Arduino.h>

// Connects using the first of `ssids` that a network scan finds nearby, falling back
// through the rest of the list (in order, wrapping around) if the preferred network's
// connection attempt doesn't succeed within maxTriesPerNetwork seconds. Blocks until
// connected. ssids/passwords must be the same length, given in `count`.
void connectWiFi(const char* const ssids[], const char* const passwords[], uint8_t count, int maxTriesPerNetwork = 30);

// Puts up a WiFiManager captive-portal config access point named `apName` instead of using
// a hardcoded credential list - the user connects to it with a phone and enters their real
// WiFi credentials, which WiFiManager then stores in flash for subsequent boots. Blocks
// until connected or the portal times out.
void connectWiFiWithManager(const char* apName, uint16_t configPortalTimeoutSeconds = 600);

// Build timestamp (when this sketch was last compiled), handy for an "about"/boot screen.
extern const char CompileDate[];

// Default NTP server for configTime(). Override by passing your own to configTime() instead.
extern const char DefaultNtpServer[];

// Helpers for configTime()'s TZ_SEC/DST_SEC parameters, e.g.:
//   configTime(TZ_SEC_FOR(8), DST_SEC_FOR(0), DefaultNtpServer); // UTC+8, no DST
#define TZ_SEC_FOR(utcOffsetHours) ((long)(utcOffsetHours) * 3600L)
#define DST_SEC_FOR(dstOffsetMinutes) ((long)(dstOffsetMinutes) * 60L)
