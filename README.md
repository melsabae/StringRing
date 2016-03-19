The StringRing's purpose is a ring buffer, containing entire strings. It is intended for use in interrupt handlers. It behaves like an infinite buffer.
This specifically means that the user interface for the StringRing is to write a character to it, read a string from it, check if it is read-ready, and mark-as-read the last read string.
No other operations are required from the application. It will perform its own internal housekeeping and validation using the provided functions.

The readTail pointer inside of the StringRing is the only member element needed to operate the StringRing. It points to an unread string.
Assuming a StringRing pointer named my_sr, you pass my_sr->readTail into a function call, like it were any other K&R string.
The functions write to, check readiness, increment the tail, and destroy a StringRing. They all, except create, take a pointer to a StringRing.
Create returns a pointer to a new StringRing in a clean state. All strings inside of the new StringRing are filled with null terminators upon creation.

The StringRing can be configured to overwrite the oldest data, keeping a running buffer of only the newest strings. See conf_stringring.h.
It can also be configured to hold onto the oldest data and destroy the newest unread string by overwriting it with newer incoming data.
This clobbering is automatic, and the StringRing can flag if it has done so. This is considered operating at or above capacity of the StringRing.
If operating at/above capacity, consider increasing the amount of strings you want it to buffer. Defaults are included in conf_stringring.h.

The parser check function by default checks for '\0' in the string's first element. It flags true if '\0' is not the first element; false if it is the first element.
 
The StringRingWrite function is the one intended for use inside of an interrupt handler. It takes a StringRing pointer and the data to write into the StringRing.
The Write function will assume that an incoming '\n' is the final character of the string.
Write increments the head automatically, and if the data sent is '\n', it writes a null terminator and moves the head to the next string.
Write is a wrapper for the typical push operation. Write implements the additional logic for handling '\n'.
 
Ring buffers usually implement a pop function. One is not provided with the StringRing.
In the event a pop were to be used, it would be possible for the StringRing to overwrite the data in the tail, at the same time as being read from.

