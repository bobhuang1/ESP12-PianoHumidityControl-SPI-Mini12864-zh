#include "StringHelpers.h"

const char* string2char(const String &command) {
	// String::c_str() is always safe to call, including on an empty String (it returns a
	// valid pointer to a null terminator, never null) - earlier copies of this function
	// had a bug where the empty-string case fell through with no return statement at all
	// (undefined behavior). No special-casing needed. The parameter is now a const
	// reference, so the returned pointer points into the caller's String, not a temporary.
	return command.c_str();
}

String intToTwoDigitString(int value) {
	return value > 9 ? String(value) : "0" + String(value);
}
