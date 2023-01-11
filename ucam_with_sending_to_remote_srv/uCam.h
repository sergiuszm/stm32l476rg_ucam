#ifndef UCAM_H
#define UCAM_H

#include "mbed.h"
#include "BufferedSerial.h"
#include <vector>

const unsigned char SYNC[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
const unsigned char ACK[] = {0xAA, 0x0E, 0x0D, 0x00, 0xF5, 0x00};

const unsigned char BAUD_57600[] = {0xAA, 0x07, 0x1F, 0x01, 0x00, 0x00};
const unsigned char BAUD_115200[] = {0xAA, 0x07, 0x1F, 0x00, 0x00, 0x00};
const unsigned char BAUD_230400[] = {0xAA, 0x07, 0x07, 0x01, 0x00, 0x00};
const unsigned char BAUD_737280[] = {0xAA, 0x07, 0x00, 0x04, 0x00, 0x00};
const unsigned char BAUD_921600[] = {0xAA, 0x07, 0x01, 0x01, 0x00, 0x00};
const unsigned char BAUD_1228800[] = {0xAA, 0x07, 0x02, 0x00, 0x00, 0x00};

const unsigned char RAW_PICTURE_INIT[] = {0xAA, 0x01, 0x00, 0x06, 0x03, 0x03};
const unsigned char SET_PACKAGE_SIZE[] = {0xAA, 0x06, 0x08, 0x00, 0x02, 0x00};
const unsigned char GET_RAW_PICTURE[] = {0xAA, 0x04, 0x02, 0x00, 0x00, 0x00};
const unsigned char RAW_PICTURE_ACK[] = {0xAA, 0x0E, 0x0A, 0x00, 0x00, 0x00};

const unsigned char JPEG_PICTURE_INIT[] = {0xAA, 0x01, 0x00, 0x07, 0x03, 0x03};
const unsigned char MAKE_SNAPSHOT[] = {0xAA, 0x05, 0x00, 0x00, 0x00, 0x00};
const unsigned char GET_SNAPSHOT[] = {0xAA, 0x04, 0x01, 0x00, 0x00, 0x00};

const unsigned char GENERAL_ACK[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};

const unsigned char RESET_CMD[] = {0xAA, 0x08, 0x01, 0x00, 0x00, 0xFF};

class uCam {
public:
    // define which pins are used
    uCam(PinName tx, PinName rx, PinName resetPin, Serial *debug);

    // Methods
    bool sync();
    bool get_snapshot();
    int get_image_data_size(bool image_data_only);
    vector<unsigned char> *get_image_data();
    int get_number_of_data_pckg();
    void hard_reset();
    bool set_baud();
    void free_buffer();
    bool reset();
    bool set_sleep_timeout();

    bool baud = false;
    bool synced = false;

private:
    BufferedSerial *_uCam;    // uCam TX and RX
    DigitalOut _reset;
    Serial *_debug;
    int _timeout;
    unsigned char _cmd_buffer[12];
//    unsigned char _image_buffer[10000];
    unsigned char *_image_buffer;
    vector<unsigned char> _img_buffer;
    unsigned char _img_tmp_buffer[512];
    vector<unsigned char>::iterator _it;
    int _cmd_buffer_pos;
    int _img_buffer_pos;
    int _img_data_size;
    bool _serial_picture_data = false;
    int _number_of_pckg;

    void _rx_callback();
    void _clear_cmd_buffer();
    void _cmd_rx_callback();
    void _img_rx_callback();
};

#endif //UCAM_H
