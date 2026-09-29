#pragma once
#include <Arduino.h>

// Returns a pointer to command's internal C-string buffer. The pointer is only valid as
// long as `command` itself lives and isn't modified/reassigned - use it immediately
// (e.g. passed straight into a display.print()/getStrWidth() call) rather than storing it.
// Returns the String's internal buffer, valid until the String is modified/destroyed.
// Takes the String by const reference (an earlier by-value version returned a pointer
// into a temporary - a dangling-pointer trap) and returns const char*, which is what
// every u8g2 print/drawStr/getStrWidth call site wants.
const char* string2char(const String &command);

// Zero-pads a 0-9 value to two digits, e.g. for HH:MM:SS display (5 -> "05", 12 -> "12").
String intToTwoDigitString(int value);
