/*
* stringring.c
*
* Created: 2015-11-27 19:04:09
*  Author: saba mas0051@uah.edu
*/

#include <stdlib.h>
#include "ctl_stringring.h"

// Special Conditions
#define SR_STRING_FILLED		(sr->sr_headLen >= sr->sr_strlen) // leave 1 space for the null terminator
#define SR_HEAD_OUT_OF_BOUNDS	(sr->writeHead >= sr->finalString + sr->sr_strlen)
#define SR_TAIL_OUT_OF_BOUNDS	(sr->readTail >= sr->finalString + sr->sr_strlen)

// Locations inside of the StringRing buffer
#define SR_FIRST_STRING			(sr->buffer)
#define SR_FINAL_STRING			(sr->finalString)
#define SR_CURRENT_HEAD			(sr->writeHead - sr->sr_headLen)
#define SR_NEXT_HEAD			(SR_CURRENT_HEAD + sr->sr_strlen)
#define SR_NEXT_TAIL			(sr->readTail + sr->sr_strlen)

// Moves the head to the next string
// Returns a flag for: if data is clobbered; it is also assumed that data _will_ be clobbered when running at capacity
static inline uint8_t StringRingMoveHeadToNextString(StringRing * const sr)
{
	*(sr->writeHead) = '\0';

	// if head is about to point to where tail is currently pointing
	if((SR_NEXT_HEAD == sr->readTail) || ((sr->readTail == SR_FIRST_STRING) && (SR_CURRENT_HEAD == SR_FINAL_STRING)))
	{
		#ifdef SR_CLOBBER_NEWEST
		sr->writeHead = SR_CURRENT_HEAD; // go back to the beginning of this string
		#else
		sr->writeHead = SR_NEXT_HEAD; // move head to current read tail so we overwrite oldest data
		StringRingSeekNextReadableString(sr); // then move readtail forward a string
		#endif
	
		sr->sr_headLen = 0;
		return 1; // signal that data was clobbered
	}
	else // if head will not be pointing to where the tail is, just move forward to the next string
	{
		sr->writeHead = SR_NEXT_HEAD;
		sr->sr_headLen = 0;
	}

	if(SR_HEAD_OUT_OF_BOUNDS)
	//if(SR_CURRENT_HEAD >= SR_FINAL_STRING) // if beyond the bounds of the ring buffer
	{
		sr->writeHead = SR_FIRST_STRING;
	}
	
	return 0; // nothing was clobbered
}

// Moves the write head forward by 1 character, increments count, then moves to next string if full
static inline uint8_t StringRingIncrementHead(StringRing * const sr)
{
	// increment the head and the count of characters in this string
	sr->sr_headLen++;
	sr->writeHead++;

	if(SR_STRING_FILLED)
	{
		return StringRingMoveHeadToNextString(sr); // this will terminate the current string before moving to the next
	}
	
	return 0;
}

// Moves the tail to the next string
static inline void StringRingIncrementTail(StringRing * const sr)
{
	sr->readTail = SR_NEXT_TAIL;
	
	if(SR_TAIL_OUT_OF_BOUNDS)
	//if(sr->readTail > SR_FINAL_STRING) // if readTail went beyond the ring buffer's length
	{
		sr->readTail = SR_FIRST_STRING; // go back to the beginning of the ring buffer
	}
}

StringRing* StringRingCreate(const uint8_t NUMSTRINGS, const uint8_t LENSTRINGS, const char STRINGREADY)
{
	if(NUMSTRINGS <= 1 || LENSTRINGS <= 2 || STRINGREADY == '\0') // you want a different data structure
	{
		return NULL;
	}
	
	// take the size of the stringring, add the size of the char array, then build that in memory
	StringRing *sr		= calloc(1, sizeof(StringRing) + (LENSTRINGS * NUMSTRINGS));

	if(sr == NULL)
	{
		return NULL;
	}

	sr->sr_strlen		= LENSTRINGS;
	sr->stringReady		= STRINGREADY;
	sr->finalString		= &(sr->buffer[(LENSTRINGS * (NUMSTRINGS - 1)) - 1]);
	sr->sr_headLen		= 0;
	
	// set pointers to sane values
	sr->writeHead		= SR_FIRST_STRING;
	sr->readTail		= SR_FINAL_STRING;
	
	return sr;
}

#ifdef SR_PUSH_RETURN
static inline char StringRingPush(StringRing * const sr, const char data)
{
	*(sr->writeHead) = data;
	StringRingIncrementHead(sr); // this returns whether or not it clobbered data, handle if you need to
	return data;
}

char StringRingWrite(StringRing * const sr, const char data)
{
	StringRingPush(sr, data);

	if(data == '\n')
	{
		StringRingPush(sr, '\0');
		
		if(StringRingMoveHeadToNextString(sr))
		{
			// do something when data is clobbered, if youd like
		}
		
		return 0; // pushed a '\0'
		//return data; // or return what was pushed?
	}

	return data;
}
#else
static inline void StringRingPush(StringRing * const sr, const char data)
{
	*(sr->writeHead) = data;
	StringRingIncrementHead(sr); // this returns whether or not it clobbered data, handle if you need to, be sure to change this function's return type to uint8_t
}

void StringRingWrite(StringRing * const sr, const char data)
{
	StringRingPush(sr, data);

	if(data == '\n')
	{
		StringRingPush(sr, '\0');
		
		if(StringRingMoveHeadToNextString(sr))
		{
			 // do something when data is clobbered, if youd like
		}
	}
}
#endif

bool IsStringRingReadyForParse(StringRing * const sr)
{
	return (*(sr->readTail) == sr->stringReady);
}

// Moves the tail to the next string, returns whether or not it was successful
bool StringRingSeekNextReadableString(StringRing * const sr)
{
	// make current string fail the IsStringRingReadyForParse function, like marking an email as having been read
	*(sr->readTail) = '\0';
	
	// if readTail being incremented would point to the string under construction
	if((SR_NEXT_TAIL == SR_CURRENT_HEAD) || ((SR_CURRENT_HEAD == SR_FIRST_STRING) && (sr->readTail == SR_FINAL_STRING)))
	{
		return false;
	}
	else
	{
		StringRingIncrementTail(sr);
		return true;
	}
}

void StringRingDestroy(StringRing * const sr)
{
	if(sr != NULL) // let's not try to free a null pointer
	{
		free(sr);
	}
}
