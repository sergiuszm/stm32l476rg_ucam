#include "mbed.h"

// Set up some memory to hold the commands
const unsigned char SYNC[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
const unsigned char ACK[] = {0xAA, 0x0E, 0x0D, 0x00, 0xF5, 0x00};
const unsigned char BAUD_115200[] = {0xAA, 0x07, 0x01, 0x0F, 0x00, 0x00};
const unsigned char BAUD_737280[] = {0xAA, 0x07, 0x00, 0x04, 0x00, 0x00};
const unsigned char BAUD_921600[] = {0xAA, 0x07, 0x01, 0x01, 0x00, 0x00};
const unsigned char BAUD_1228800[] = {0xAA, 0x07, 0x02, 0x00, 0x00, 0x00};

class uCamIII {
public:
    // define which pins are used
    uCamIII(PinName tx, PinName rx, PinName resetPin);

    // Methods
    bool Sync();
    int SetBaud(int baud);
    int Initial(unsigned char COLOUR, unsigned char RES);
    int GetPicture(unsigned char *data);
    int GetResponse(unsigned char type, unsigned char command);
    void HardReset();
    void _RxCallback();

private:
    Serial _uCam;    // uCam TX and RX
    DigitalOut _reset;
    int _timeout;
    unsigned char _buffer[12];
    unsigned char _image_buffer[80 * 60];
    unsigned int _buffer_pos;
};
