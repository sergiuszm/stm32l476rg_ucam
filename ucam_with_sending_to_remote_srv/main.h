#ifndef MAIN_H
#define MAIN_H

#include "mbed.h"
#include "FATFileSystem.h"
#include "SDBlockDevice.h"
#include "uCam.h"
#include "rtcx.h"
#include <stdio.h>
#include <errno.h>
#include "BufferedSerial.h"
#include "easy-connect.h"
#include "TCPSocket.h"
#include "http_request.h"
#include <string>
#include <iomanip>
#include <sstream>
#include "TCPSocket.h"
#include <ostream>
#include <iterator>

/* mbed_retarget.h is included after errno.h so symbols are mapped to
 * consistent values for all toolchains */
#include "platform/mbed_retarget.h"

#if defined (CLION)
    #include "mbed_config.h"
#endif

#endif //MAIN_H
