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
#define SR_CURRENT_HEAD					(sr->writeHead - sr->headLen)
#define SR_NEXT_HEAD					(SR_CURRENT_HEAD + sr->strLen)
#define SR_NEXT_TAIL					(sr->readTail + sr->strLen)

// Conditionals
#define SR_HEAD_WILL_CLOBBER_TAIL		((SR_NEXT_HEAD == sr->readTail) || ((sr->readTail == SR_FIRST_STRING) && (SR_CURRENT_HEAD == SR_FINAL_STRING))) // considered the "full" state
#define SR_TAIL_WILL_POINT_TO_HEAD		((SR_NEXT_TAIL == SR_CURRENT_HEAD) || ((sr->readTail == SR_FINAL_STRING) && (SR_CURRENT_HEAD == SR_FIRST_STRING))) // considered the "empty" state

// Moves head to next valid position, based on configuration
static inline void StringRingPushHead(StringRing * const sr, const bool WILLCLOBBER)
{	
	if(WILLCLOBBER)
	{
		if(sr->clobberOld)
		{
			if(SR_CURRENT_HEAD >= SR_FINAL_STRING)
			{
				sr->writeHead = SR_FIRST_STRING;
			}
			else
			{
				sr->writeHead = SR_NEXT_HEAD;
			}
		}
		else
		{
			sr->writeHead = SR_CURRENT_HEAD;
		}
	}
	else
	{
		if(SR_CURRENT_HEAD >= SR_FINAL_STRING)
		{
			sr->writeHead = SR_FIRST_STRING;
		}
		else
		{
			sr->writeHead = SR_NEXT_HEAD;
		}
	}
	
	sr->headLen = 0;
}

// Moves the head to the next string
// Returns 0 if nothing is going to be clobbered, 1 if it clobbered something newer, or -1 if it clobbered something older
static inline int8_t StringRingMoveHeadToNextString(StringRing * const sr)
{
	*(sr->writeHead) = '\0';

	if(SR_HEAD_WILL_CLOBBER_TAIL)
	{
		if(sr->clobberOld)
		{
			StringRingSeekNextReadableString(sr); // push readtail forward a string
			StringRingPushHead(sr, true);
			return -1;
		}
		else
		{
			StringRingPushHead(sr, true);
			return 1;
		}
	}
	
	StringRingPushHead(sr, false);
	return 0;
}

// Moves the write head forward by 1 character
static inline void StringRingIncrementHead(StringRing * const sr)
{
	sr->headLen++;
	sr->writeHead++;
}

// Moves the tail to the next string
static inline void StringRingIncrementTail(StringRing * const sr)
{	
	if(sr->readTail == SR_FINAL_STRING)
	{
		sr->readTail = SR_FIRST_STRING;
	}
	else
	{
		sr->readTail = SR_NEXT_TAIL;
	}
}

// Returns a pointer to a StringRing in a clean state
StringRing* StringRingCreate(const uint8_t NUMSTRINGS, const uint8_t LENSTRINGS, const bool CLOBBEROLD)
{
	if(NUMSTRINGS <= 1 || LENSTRINGS <= 2)
	{
		return NULL;
	}
	
	// take the size of the stringring, add the size of the char array, then build that
	StringRing *sr			= calloc(1, sizeof(StringRing) + (LENSTRINGS * NUMSTRINGS));

	if(sr != NULL)
	{
		sr->clobberOld		= CLOBBEROLD;
		sr->strLen			= LENSTRINGS;
		sr->finalString		= &(sr->buffer[LENSTRINGS * (NUMSTRINGS - 1)]);
		sr->headLen			= 0;
		
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
		StringRingMoveHeadToNextString(sr);
		
		return 0; // pushed a '\0'
		//return data; // or return what was pushed?
	}

	return DATA;
}
#else
static inline void StringRingPush(StringRing * const sr, const char DATA)
{
	*(sr->writeHead) = DATA;
	StringRingIncrementHead(sr);
}

void StringRingWrite(StringRing * const sr, const char DATA)
{
	StringRingPush(sr, DATA);

	if(DATA == '\n')
	{
		StringRingMoveHeadToNextString(sr);
	}
}
#endif

// Checks to see if the beginning of the string is not a null terminator
bool IsStringRingReadyForParse(StringRing * const sr)
{
	return (*(sr->readTail) != '\0');
}

// Makes current string fail the IsStringRingReadyForParse function, like marking an email as having been read
// Moves the tail to the next string if possible; returns whether or not it was successful
bool StringRingSeekNextReadableString(StringRing * const sr)
{
	*(sr->readTail) = '\0';
	
	// explicitly deny this; where you can start reading a string that hasn't been finalized
	// if this happens enough you can probably lower the buffer size
	if(SR_TAIL_WILL_POINT_TO_HEAD)
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
