#include "mbed.h"

class MySerial: public Serial
{
public:
  MySerial(PinName tx, PinName rx, const char *name = NULL) : Serial(tx, rx, name) {};
  // reads a character, waiting for up to timeout ms
  // returns -1 in case of timeout
  // timeout: timeout in ms, -1 for infinite
  int getc( int timeout = -1 );
  MySerial returnSerial(); 
};
