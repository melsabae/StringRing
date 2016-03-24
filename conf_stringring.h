/*
 * conf_stringring.h
 *
 * Created: 2015-11-27 19:04:09
 *  Author: saba mas0051@uah.edu
 */ 

#ifndef CONF_STRINGRING_H_
#define CONF_STRINGRING_H_

// Conditional compile for whether or not the StringRing should return each character that is written to it
//#define SR_PUSH_RETURN

// Determines behavior for when StringRing is operating at capacity: always keep newest strings buffered, or clobber the newest with newer incoming data?
//#define SR_CLOBBER_OLDEST
//#define SR_CLOBBER_NEWEST

// Default buffering capacity
#define SR_DEFAULT_NUMBER_STRINGS			5

// Default string length
#define SR_DEFAULT_SIZE_STRINGS				136

// error conditions
//#ifdef SR_CLOBBER_OLDEST
//#ifdef SR_CLOBBER_NEWEST
//#error "Cannot have both SR_CLOBBER_OLDEST and SR_CLOBBER_NEWEST macros enabled."
//#endif
//#endif
//
//#ifndef SR_CLOBBER_OLDEST
//#ifndef SR_CLOBBER_NEWEST
//#error "Must have SR_CLOBBER_OLDEST xor SR_CLOBBER_NEWEST macro enabled."
//#endif
//#endif

#endif /* CONF_STRINGRING_H_ */
