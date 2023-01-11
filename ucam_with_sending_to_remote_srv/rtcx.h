#ifndef RTCX_H
#define RTCX_H

#include "mbed.h"
#include "ds3231.h"
#include "string"
#include "Json.h"
#include "TCPSocket.h"
#include "http_request.h"

#if defined (CLION)
    #include "mbed_config.h"
#endif

class RTCx {
public:
    RTCx(PinName sda, PinName scl, Serial *debug);
    time_t get_timestamp();
    bool lost_power();
    bool setup_time();
    void set_network(NetworkInterface *network);

private:
    Ds3231 _rtc;
    Serial *_debug;
    long _timestamp;
    string _api_timezone;
    string _api_key;
    string _api_request_path;
    NetworkInterface *_network;

    long _get_timestamp_from_api();
};

#endif //RTCX_H
