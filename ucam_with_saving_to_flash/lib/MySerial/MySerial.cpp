#include "MySerial.h"

int MySerial::getc( int timeout )
{
  // if infinite timeout or we have a character, call base getc()
  if ( timeout == -1 || readable() )
    return Serial::getc();

  // no character yet
  bool has_data = false;
  // count elapsed time
  Timer timer;
  timer.start();
  // loop until we have data or timeout elapses
  while ( !has_data && timer.read_ms() < timeout )
  {
    // wait a short time
    wait_ms(1);
    // check again
    has_data = readable();
  }
  // do we have anything?
  if ( has_data )
    // yes, read it
    return Serial::getc();
  else
    // no, timed out
    return -1;
}

MySerial MySerial::returnSerial() {
    return this->serial_t;
}
