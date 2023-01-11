#include "rtcx.h"


RTCx::RTCx(PinName sda, PinName scl, Serial *debug) : _rtc(sda, scl) {
    _api_key = MBED_CONF_APP_TIMEZONEDB_KEY;
    _api_request_path = MBED_CONF_APP_TIMEZONEDB_TIME_PATH;
    _api_timezone = MBED_CONF_APP_TIMEZONEDB_ZONE;
    _debug = debug;
}

bool RTCx::lost_power() {
    _debug->printf("\n\rReading DS3231 Status Register (Lost Power?)");
    ds3231_cntl_stat_t status = {0x0F, 0};
    if (!_rtc.get_cntl_stat_reg(&status)) {
        _debug->printf("\n\rLost Power: ");
        _debug->printf("%s", (status.status >> 7) ? "True" : "False");
        return (status.status >> 7);
    }

    return false;
}

bool RTCx::setup_time() {
    long api_timestamp = _get_timestamp_from_api();
    if (!api_timestamp) {
        _debug->printf("\n\rError getting timestamp from API!");
        _debug->printf("\n\rInitialization failed!");

        return false;
    }

    ds3231_time_t rtc_time;
    ds3231_calendar_t rtc_calendar;

    const time_t rawTimestamp = (const time_t) api_timestamp;
    struct tm * dt;

    _debug->printf("\n\rTime as a basic string = %s", ctime(&rawTimestamp));

    dt = localtime(&rawTimestamp);
    rtc_calendar.day = (uint32_t) (dt->tm_wday + 1);
    _debug->printf("\n\rDay: %d", (uint32_t) (dt->tm_wday + 1));
    rtc_calendar.date = (uint32_t) dt->tm_mday;
    _debug->printf("\n\rDay of the month: %d", (uint32_t) dt->tm_mday);
    rtc_calendar.month = (uint32_t) (dt->tm_mon + 1);
    _debug->printf("\n\rMonth: %d", (uint32_t) (dt->tm_mon + 1));
    rtc_calendar.year = (uint32_t) abs(dt->tm_year - 100);
    _debug->printf("\n\rYear: %d", (uint32_t) abs(dt->tm_year - 100));
    rtc_time.mode = 0;
    rtc_time.hours = (uint32_t) dt->tm_hour;
    _debug->printf("\n\rHour: %d", (uint32_t) dt->tm_hour);
    rtc_time.minutes = (uint32_t) dt->tm_min;
    _debug->printf("\n\rMinute: %d", (uint32_t) dt->tm_min);
    rtc_time.seconds = (uint32_t) dt->tm_sec;
    _debug->printf("\n\rSecond: %d", (uint32_t) dt->tm_sec);

    if (_rtc.set_time(rtc_time)) {
        _debug->printf("\n\rRTC set_time failed!");
        _debug->printf("\n\rInitialization failed!");

        return false;
    }

    if (_rtc.set_calendar(rtc_calendar)) {
        _debug->printf("\n\rRTC set_calendar failed!");
        _debug->printf("\n\rInitialization failed!");

        return false;
    }

    time_t epoch_time;

    //new epoch time fx
    epoch_time = _rtc.get_epoch();

    _debug->printf("\n\rTime as seconds since January 1, 1970 = %d\n", epoch_time);

    _debug->printf("\n\rTime as a basic string = %s", ctime(&epoch_time));

    char buffer[32];
    strftime(buffer, 32, "%I:%M %p\n", localtime(&epoch_time));
    _debug->printf("\n\rTime as a custom formatted string = %s", buffer);

    return true;
}

void RTCx::set_network(NetworkInterface *network) {
    _network = network;
}

time_t RTCx::get_timestamp() {
    return _rtc.get_epoch();
}

long RTCx::_get_timestamp_from_api() {
    char buffer[255];
    sprintf(buffer, "%s&key=%s&by=zone&zone=%s", _api_request_path.c_str(), _api_key.c_str(), _api_timezone.c_str());

    HttpRequest* get_req = new HttpRequest(_network, HTTP_GET, buffer);

    HttpResponse* get_res = get_req->send();
    if (!get_res) {
        _debug->printf("\n\rHttpRequest failed (error code %d)\n", get_req->get_error());
        delete get_req;

        return -1;
    }

    _debug->printf("\n\r----- HTTP GET response -----\n\r");
    _debug->printf("\n\rStatus: %d - %s\n", get_res->get_status_code(), get_res->get_status_message().c_str());

    if (get_res->get_status_code() != 200) {
        _debug->printf("\n\rWrong status from API: %d", get_res->get_status_code());
        delete get_req;

        return -1;
    }

    Json json(get_res->get_body_as_string().c_str(), get_res->get_body_length());
    if (!json.isValidJson()) {
        _debug->printf("\n\rInvalid JSON");
        delete get_req;

        return -1;
    }

    char timestamp_value[11];
    long timestamp = -1;
    int timestamp_key_index = json.findKeyIndexIn("timestamp", 0);
    if (timestamp_key_index == -1) {
        _debug->printf("\n\r\"timestamp\" does not exist...");

        return -1;
    } else {
        // Find the first child index of key-node "timestamp"
        int timestamp_value_index = json.findChildIndexOf ( timestamp_key_index, -1 );
        if ( timestamp_value_index > 0 ) {
            const char *value_start = json.tokenAddress(timestamp_value_index);
            int value_length = json.tokenLength(timestamp_value_index);
            strncpy(timestamp_value, value_start, value_length);
            timestamp_value[value_length] = 0; // NULL-terminate the string

            //let's print the value.  It should be timestamp
            _debug->printf("\n\rTimestamp from API: %s", timestamp_value);

            timestamp = atol(timestamp_value);
        }
    }

    delete get_req;

    return timestamp;
}