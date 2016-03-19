/*
* stringring.c
*
* Created: 2015-11-27 19:04:09
*  Author: saba mas0051@uah.edu
*/

#include <stdlib.h>
#include "ctl_stringring.h"

// Locations inside of the StringRing buffer
#define SR_FIRST_STRING					(sr->buffer)
#define SR_FINAL_STRING					(sr->finalString)
#define SR_CURRENT_HEAD					(sr->writeHead - sr->sr_headLen)
#define SR_NEXT_HEAD					(SR_CURRENT_HEAD + sr->sr_strlen)
#define SR_NEXT_TAIL					(sr->readTail + sr->sr_strlen)

// Conditionals
#define SR_STRING_FILLED				(sr->sr_headLen >= sr->sr_strlen)
// this technically points to the end of the buffer, which is not out of range, but we are defining it to be so that there is space for \0
#define SR_HEAD_OUT_OF_BOUNDS			(sr->writeHead >= (sr->finalString + sr->sr_strlen))
// tail should never point to beyond finalString, the +1 seems to make it happier - much lower error rate
#define SR_TAIL_OUT_OF_BOUNDS			(sr->readTail > (sr->finalString + 1))
#define SR_HEAD_WILL_CLOBBER_TAIL		(SR_NEXT_HEAD == sr->readTail) || ((sr->readTail == SR_FIRST_STRING) && (SR_CURRENT_HEAD == SR_FINAL_STRING))
#define SR_TAIL_WILL_CLOBBER_HEAD		(SR_NEXT_TAIL == SR_CURRENT_HEAD) || ((SR_CURRENT_HEAD == SR_FIRST_STRING) && (sr->readTail == SR_FINAL_STRING))

// Moves the head to the next string
// Returns a flag for: 1 if data is clobbered, 0 if not clobbered
static inline uint8_t StringRingMoveHeadToNextString(StringRing * const sr)
{
	*(sr->writeHead) = '\0';
	// note for anyone maitaining this: sr_headLen is used in the conditionals, so it should not be modified until after the checks

	if(SR_HEAD_WILL_CLOBBER_TAIL)
	{
		#ifdef SR_CLOBBER_NEWEST
		sr->writeHead = SR_CURRENT_HEAD; // go back to the beginning of this string
		#else
		sr->writeHead = SR_NEXT_HEAD;
		StringRingSeekNextReadableString(sr); // move readtail forward a string
		#endif
		
		sr->sr_headLen = 0;
		return 1;
	}
	else
	{
		sr->writeHead = SR_NEXT_HEAD;
		sr->sr_headLen = 0;

		if(SR_HEAD_OUT_OF_BOUNDS)
		{
			sr->writeHead = SR_FIRST_STRING;
		}
	}
	
	return 0;
}

// Moves the write head forward by 1 character; moves to next string if the string is full
static inline uint8_t StringRingIncrementHead(StringRing * const sr)
{
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
	{
		sr->readTail = SR_FIRST_STRING;
	}
}

// Returns a pointer to a StringRing in a clean state
// STRINGREADY is the first character of the buffered strings; our sensors use '$' by default
StringRing* StringRingCreate(const uint8_t NUMSTRINGS, const uint8_t LENSTRINGS, const char STRINGREADY)
{
	if(NUMSTRINGS <= 1 || LENSTRINGS <= 2 || STRINGREADY == '\0')
	{
		return NULL;
	}
	
	// take the size of the stringring, add the size of the char array, then build that
	StringRing *sr			= calloc(1, sizeof(StringRing) + (LENSTRINGS * NUMSTRINGS));

	if(sr != NULL)
	{
		sr->sr_strlen		= LENSTRINGS;
		sr->stringReady		= STRINGREADY;
		sr->finalString		= &(sr->buffer[(LENSTRINGS * (NUMSTRINGS - 1)) - 1]);
		sr->sr_headLen		= 0;
		sr->writeHead		= SR_FIRST_STRING;
		sr->readTail		= SR_FINAL_STRING;
	}
	
	return sr;
}

// StringRing push adds DATA to the buffer
// StringRing write will add data to the buffer, and if upon receiving '\n', will terminate the string and increment the head.
#ifdef SR_PUSH_RETURN
static inline char StringRingPush(StringRing * const sr, const char DATA)
{
	*(sr->writeHead) = DATA;
	StringRingIncrementHead(sr); // this returns whether or not it clobbered data, handle if you need to
	return DATA;
}

char StringRingWrite(StringRing * const sr, const char DATA)
{
	StringRingPush(sr, DATA);

	if(DATA == '\n')
	{
		StringRingPush(sr, '\0');
		
		if(StringRingMoveHeadToNextString(sr))
		{
			// do something when data is clobbered, if youd like
		}
		
		return 0; // pushed a '\0'
		//return data; // or return what was pushed?
	}

	return DATA;
}
#else
static inline void StringRingPush(StringRing * const sr, const char DATA)
{
	*(sr->writeHead) = DATA;
	StringRingIncrementHead(sr); // this returns whether or not it clobbered data, handle if you need to, be sure to change this function's return type to uint8_t
}

void StringRingWrite(StringRing * const sr, const char DATA)
{
	StringRingPush(sr, DATA);

	if(DATA == '\n')
	{
		StringRingPush(sr, '\0');
		
		if(StringRingMoveHeadToNextString(sr))
		{
			// do something when data is clobbered, if youd like
		}
	}
}
#endif

// Checks to see if the beginning of the string contains the StringReady character
bool IsStringRingReadyForParse(StringRing * const sr)
{
	return (*(sr->readTail) == sr->stringReady);
}

// Makes current string fail the IsStringRingReadyForParse function, like marking an email as having been read
// Moves the tail to the next string if possible; returns whether or not it was successful
bool StringRingSeekNextReadableString(StringRing * const sr)
{
	*(sr->readTail) = '\0';
	
	if(SR_TAIL_WILL_CLOBBER_HEAD)
	{
		return false;
	}
	
	StringRingIncrementTail(sr);
	return true;
}

// A wrapper for free: destroys a StringRing
void StringRingDestroy(StringRing * const sr)
{
	if(sr != NULL)
	{
		free(sr);
	}
}
