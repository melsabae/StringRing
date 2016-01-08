/*
 * stringring.h
 *
 * Created: 2015-11-27 19:04:09
 *  Author: saba mas0051@uah.edu
 */ 

#include <stdint-gcc.h>
#include <stdbool.h>
#include "conf_stringring.h"

// For most RingBuffer implementations, you will see an N-1 amount of storage, where N is the number of datum. This holds true here.
// Each string in the StringRing is atomic; it is intentional that the application reads the entire string or none of it.
// The application should only be concerned with the pointers, writeHead and readTail. A character is written to the writeHead. An entire string is read from readTail.
// The logic for writeHead is automatic. Writing to it is done with StringRingWrite.
// When StringRingWrite receives a linefeed, \n, it will null terminate the string and move on to the next string.

// StringRingPop is currently not being used. It returns a pointer to the last unread string, then increments the tail. This can cause an edge case wherein the writeHead can be placed
// on the same row as the tail, potentially clobbering data. The current method of using the readTail is to use your stringring->readTail pointer to parse, then call StringRingSeekNextReadableString.

// This implementation restricts the movement of its head and tail pointers. Once an entire string has been written to the StringRing, these 2 pointers will not point to the same row.
// writeHead and readTail will never be on the same row of the 2D array (except for after creation), unless StringRingPop is used, and even then under certain circumstances.
// In the event the StringRing is full, the default behavior is to clobber the entire string the writeHead points to and overwrite it with new data.
// This can be alleviated by slowing the frequency of incoming data, and/or modifying the NUMBER_STRINGS in conf_stringring.h to a higher value.

typedef struct
{
	uint_fast8_t headCol; // column index of string that writeHead is working on
	uint_fast8_t headRow; // row index of string that writeHead is working on
	uint_fast8_t tailRow; // row index of current tail of unread data
	
	// stored for faster lookups instead of calculating
	uint_fast8_t numRows;
	uint_fast8_t numCols;
	
	char stringReady; // a tag to see if the string is okay to read
	char *writeHead; // ringbuffer head, used to write incoming characters
	char *readTail; // points to last unprocessed string
	char **buffer; // the buffer
} StringRing;

#ifdef PUSH_RETURN
// Write a single character to a string in the StringRing, and returns the value written. Increments head automatically.
char StringRingWrite(StringRing *sr, const char data);
#endif

#ifndef PUSH_RETURN
//Write a character. Increments head automatically.
void StringRingWrite(StringRing *sr, const char data);
#endif

// Increments the tail to the next unread string, if possible.
uint8_t StringRingSeekNextReadableString(StringRing *sr);

// Increments the head to the next string, if possible.
void StringRingSeekNextWritableString(StringRing *sr);

// Returns the address of the current tail string; removes the entire string by incrementing the tail.
//char* StringRingPop(StringRing **sr);

// Returns the current character under the head pointer, without moving the head pointer forward.
char StringRingPeekHead(StringRing *sr);

// Returns the first character behind the tail pointer, without moving the tail pointer forward.
char StringRingPeekTail(StringRing *sr);

// Checks to see if there are any strings available for buffering
//bool IsStringRingEmpty(StringRing **sr);

// Checks to see if the string under readTail contains the character used to tag data, and if so, returns true.
// This character is stored in the StringRing as stringReady
bool IsStringRingReadyForParse(StringRing *sr);

// Returns the address of a new StringRing in a clean state
StringRing* StringRingCreate(uint_least8_t, uint_least8_t, char);

// Cleans all the dynamic allocation in a stringring
void StringRingDestroy(StringRing *sr);
