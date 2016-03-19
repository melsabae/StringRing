/*
 * stringring.h
 *
 * Created: 2015-11-27 19:04:09
 *  Author: saba mas0051@uah.edu
 */ 

#ifndef CTL_STRINGRING_H_
#define CTL_STRINGRING_H_

#include <stdint-gcc.h>
#include <stdbool.h>
#include "conf_stringring.h"

/*
The StringRing's purpose is a ring buffer, containing entire strings. It is intended for use in interrupt handlers. It behaves like an infinite buffer.
This specifically means that the user interface for the StringRing is to write a character to it, read a string from it, check if it is read-ready, and mark-as-read the last read string.
No other operations are required from the application. It will perform its own internal housekeeping and validation using the provided functions.

The readTail pointer inside of the StringRing is the only member element needed to operate the StringRing. It points to an unread string.
Assuming a StringRing pointer named my_sr, you pass my_sr->readTail into a function call, like it were any other K&R string.
The functions below create, write to, check readiness, increment the tail, and destroy a StringRing. They all, except create, take a pointer to a StringRing.
Create returns a pointer to a new StringRing in a clean state. All strings inside of the new StringRing are filled with null terminators.

The StringRing can be configured to overwrite the oldest data, keeping a running buffer of only the newest strings. See conf_stringring.h.
It can also be configured to hold onto the oldest data and destroy the newest unread string by overwriting it with newer incoming data.
This clobbering is automatic, and the StringRing can flag if it has done so. This is considered operating at or above capacity of the StringRing.
If operating at/above capacity, consider increasing the amount of strings you want it to buffer. Defaults are included in conf_stringring.h.

The parser check function checks to see if the first character behind the readTail pointer is a '\0'. If so, it returns false; true if it was not '\0'.

The StringRingWrite function is the one intended for use inside of an interrupt handler. It takes a StringRing pointer and the data to write into the StringRing.
The Write function will assume that an incoming '\n' is the final character of the string.
Write increments the head automatically, and if the data sent is '\n', it moves to the next string.
Write is a wrapper for the typical push operation. Write implement the additional logic for handling '\n'.

Ring buffers usually implement a pop function. One is not provided with the StringRing.
In the event a pop were to be used, it would be possible for the StringRing to overwrite the data in the tail, at the same time as being read from.
*/

typedef struct
{	
	// this member IS the user interface to this data structure
	char		*readTail; // points to last unprocessed string
	
	// these assist 'housekeeping' activity inside the buffer
	uint8_t		sr_headLen; // string length of string under construction
	uint8_t		sr_strlen; // string length of each of this buffer's strings
	char		*writeHead; // ringbuffer head, used to write incoming characters
	char		*finalString; // points to the final string in the buffer
	char		buffer[];
} StringRing;

// Returns the address of a new StringRing in a clean state
StringRing* StringRingCreate(const uint8_t NUMSTRINGS, const uint8_t LENSTRINGS);

// SR_PUSH_RETURN is #defined in conf_stringring.h
#ifdef SR_PUSH_RETURN
// Write a character to a StringRing, and returns the value written. Increments head automatically.
char StringRingWrite(StringRing * const sr, const char data);
#else
// Write a character to a StringRing. Increments head automatically.
void StringRingWrite(StringRing * const sr, const char data);
#endif

// Checks if the string under readTail is useable.
bool IsStringRingReadyForParse(StringRing * const sr);

// This function 'clips' the 'string ready' character from the string pointed to by readTail
// Increments the tail to the next unread string, if possible.
// Returns false if it did not increment. Subsequent checks for 'readiness' will fail for this string.
// Returns true for successful increment.
bool StringRingSeekNextReadableString(StringRing * const sr);

// Destroys the StringRing
void StringRingDestroy(StringRing * const sr);

#endif
