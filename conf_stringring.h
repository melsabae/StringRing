/*
 * conf_stringring.h
 *
 * Created: 2015-11-27 19:04:09
 *  Author: saba mas0051@uah.edu
 */ 


#ifndef CONF_STRINGRING_H_
#define CONF_STRINGRING_H_

// These options control tuning parameters

// total storage space is ((NUMBER_STRINGS*SIZE_STRINGS) + (sizeof(pointers) * 2) + 3

// add more for more buffering capability
#define NUMBER_STRINGS          5

// this is in bytes
#define SIZE_STRINGS            135 //136 holds the longest string the VN will spit out, plus \r\n and a \0

#define STRING_READY			'$'

#define PRINT_STRINGRING

//#define PUSH_RETURN

#endif /* CONF_STRINGRING_H_ */