# StringRing
Data structure for serial communication buffering using K&amp;R C-strings

This data structure works as a ring buffer, using C style (K&R) strings. It maintains its own pointers, the only thing needed by a user is to push a character onto it, and when the string is terminated (I have made it to terminate itself on a '\n'), it increments to the next available string in the buffer.

The internal tail pointer, tailRow, is the last unread string. The user software must manually call the function provided to move to the next unread string.

The StringRing will not overwrite older data, it will instead clobber new data. It will not increment the tailRow pointer to a line that has not been terminated.

The purpose of this was to buffer entire strings of data from an external source in a hardware project. The serial stream triggered an interrupt, and the interrupt handler passed the incoming character to the StringRing. My application would go on to do other things while it waited. The StringReady character was how I chose to provide a true/false value for whether or not a string was ready to be parsed. After using a string, the SeekNextReadableString function would 'clip' the string[0] element, so that it would not be parsed a second time, then try to increment to the next available string.
