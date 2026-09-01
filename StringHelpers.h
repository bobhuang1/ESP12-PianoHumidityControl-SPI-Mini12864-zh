#pragma once
#include <Arduino.h>

// Returns a pointer to command's internal C-string buffer. The pointer is only valid as
// long as `command` itself lives and isn't modified/reassigned - use it immediately
// (e.g. passed straight into a display.print()/getStrWidth() call) rather than storing it.
char* string2char(String command);

// Zero-pads a 0-9 value to two digits, e.g. for HH:MM:SS display (5 -> "05", 12 -> "12").
String intToTwoDigitString(int value);
