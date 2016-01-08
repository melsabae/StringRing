/*
 * stringring.c
 *
 * Created: 2015-11-27 19:04:09
 *  Author: saba mas0051@uah.edu
 */ 
 
#include <stdlib.h>
#include <string.h>
#include "stringring.h"

// Moves the write head forward by 1 character, and to the next line if need be
static inline void StringRingIncrementHead(StringRing *sr)
{
	//increment the head
	sr->headCol = (sr->headCol + 1) % sr->numCols;

	// if column wrapped, move the row too
	if(sr->headCol == 0)
	{
		sr->headRow = (sr->headRow + 1) % (sr->numRows);
	}

	sr->writeHead = &sr->buffer[sr->headRow][sr->headCol];
}

//Moves the StringRing head down a row
static inline void StringRingMoveHeadToNextString(StringRing *sr)
{
	// if moving the head down or around by 1 row will place it on the same row as the current readTail
	// then reset the string head is pointing to, move all the way back to the beginning, and it can be overwritten
	// if this is the case, we are operating at capacity and the data is coming in too quickly
	if(((sr->headRow + 1) == sr->tailRow) || (((sr->headRow + 1) >= sr->numRows) && (sr->tailRow == 0)))
	{
		sr->writeHead = &sr->buffer[sr->headRow][0]; // reset position in string for overwriting
		//memset(sr->writeHead, 0, SIZE_STRINGS - 1); // nullify all data in head string
		return;
	}

	// if the head would not be clobbering the tail, then move the head forward by a row
	sr->headCol = (sr->numCols - 1); // put the column at the end of the string, so the increment head function sets it to 0
	StringRingIncrementHead(sr); // this will move the head down a row
}

// Moves the tail forward to the next string
// Strings in the StringRing are atomic, you read all or nothing
// This enforces that my StringRing will not be trying to read data that hasn't fully transmitted
static inline uint8_t StringRingIncrementTail(StringRing *sr)
{
	// If the moving the readTail down will collide with the writeHead, clip the STRING_READY off of the string and do nothing else
	// So the string isn't parsed again

	if(((sr->tailRow + 1) == sr->headRow) || (((sr->tailRow + 1) >= sr->numRows) && (sr->headRow == 0)))
	{
		return 1;
	}

	sr->tailRow = (sr->tailRow + 1) % sr->numRows;
	sr->readTail = &sr->buffer[sr->tailRow][0];
	return 0;
}

#ifdef PUSH_RETURN
static inline char StringRingPush(StringRing *sr, const char data)
{
	StringRingPush(sr, data);
	StringRingIncrementHead(sr);
	return data;
}

char StringRingWrite(StringRing *sr, const char data)
{
	StringRingPush(sr, data);
	StringRingIncrementHead(sr);

	if(data == '\n')
	{
		StringRingPush(sr, '\0');
		StringRingSeekNextWritableString(sr);
	}

	return data;
}
#endif

#ifndef PUSH_RETURN
static inline void StringRingPush(StringRing *sr, const char data)
{
	*sr->writeHead = data;
	StringRingIncrementHead(sr);
}

void StringRingWrite(StringRing *sr, const char data)
{
	StringRingPush(sr, data);

	if(data == '\n')
	{
		StringRingPush(sr, '\0');
		StringRingSeekNextWritableString(sr);
	}
}
#endif

uint8_t StringRingSeekNextReadableString(StringRing *sr)
{
	*sr->readTail = '\0';
	//memset(sr->readTail, 0, sr->numCols -1);
	return StringRingIncrementTail(sr);
}

void StringRingSeekNextWritableString(StringRing *sr)
{
	StringRingMoveHeadToNextString(sr);
}

//char* StringRingPop(StringRing *sr)
//{
//static char *data;
//data = sr->readTail;
//StringRingIncrementTail(sr);
//return data;
//}

char StringRingPeekHead(StringRing *sr)
{
	return *sr->writeHead;
}

//bool IsStringRingEmpty(StringRing *sr)
//{
	//// if the head of the StringRing is on the first string
	//// then check to see if the tailRow is the final row
	//if(sr->headRow == 0)
	//{
		//return (sr->tailRow == (sr->numRows - 1));
	//}
	//// else check to see if the StringRing head row is the one just after the tail's row
	//else
	//{
		//return ((sr->headRow - 1) == sr->tailRow);
	//}
//}

bool IsStringRingReadyForParse(StringRing *sr)
{
	return (*sr->readTail == sr->stringReady);
}

// not sure this won't create a memory leak, but even with 3 MB of input the microcontroller still chugs along just fine
StringRing* StringRingCreate(const uint_least8_t rows, const uint_least8_t columns, const char stringReady)
{
	StringRing *sr = (StringRing *) malloc(sizeof(StringRing));

	sr->numRows = rows;
	sr->numCols = columns;
	sr->headRow = 0;
	sr->headCol = 0;
	sr->tailRow = 0;
	sr->stringReady = stringReady;

	sr->buffer = malloc(rows * sizeof(char *));
	
	if(sr->buffer == NULL)
	{
		return NULL;
	}

	for(uint_least8_t i = 0; i < rows; i++)
	{
		sr->buffer[i] = calloc(columns, 1);
		
		if(sr->buffer[i] == NULL)
		{
			return NULL;
		}
	}

	sr->writeHead = &sr->buffer[sr->headRow][sr->headCol];
	sr->readTail = &sr->buffer[sr->tailRow][0];

	return sr;
}

void StringRingDestroy(StringRing *sr)
{
	for(uint_least8_t i = 0; i < sr->numRows; i++)
	{
		free(sr->buffer[i]);
	}

	free(sr->buffer);
	free(sr);
}
