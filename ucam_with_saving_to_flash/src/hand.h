#include "mbed.h"
#include "math.h"
#include "mbed_stats.h"

// #include "FATFileSystem.h"
// #include "SDFileSystem.h"
// #include "uCamIII.h"

#ifdef TARGET_NUCLEO_L476RG
    #define TX_PIN PC_10
    #define RX_PIN PC_11
    #define RESET_PIN PH_1
#elif TARGET_NUCLEO_F767ZI
    #define TX_PIN PD_5
    #define RX_PIN PD_6
    #define RESET_PIN PG_0
#endif

const unsigned char SYNC[] = {0xAA, 0x0D, 0x00, 0x00, 0x00, 0x00};
const unsigned char ACK[] = {0xAA, 0x0E, 0x0D, 0x00, 0xF5, 0x00};

const unsigned char BAUD_115200[] = {0xAA, 0x07, 0x01, 0x0F, 0x00, 0x00};
const unsigned char BAUD_737280[] = {0xAA, 0x07, 0x00, 0x04, 0x00, 0x00};
const unsigned char BAUD_921600[] = {0xAA, 0x07, 0x01, 0x01, 0x00, 0x00};
const unsigned char BAUD_1228800[] = {0xAA, 0x07, 0x02, 0x00, 0x00, 0x00};

const unsigned char RAW_PICTURE_INIT[] = {0xAA, 0x01, 0x00, 0x06, 0x03, 0x03};
const unsigned char SET_PACKAGE_SIZE[] = {0xAA, 0x06, 0x08, 0x00, 0x02, 0x00};
const unsigned char GET_RAW_PICTURE[] = {0xAA, 0x04, 0x02, 0x00, 0x00, 0x00};
const unsigned char RAW_PICTURE_ACK[] = {0xAA, 0x0E, 0x0A, 0x00, 0x00, 0x00};

const unsigned char JPEG_PICTURE_INIT[] = {0xAA, 0x01, 0x00, 0x07, 0x07, 0x07};
const unsigned char MAKE_SNAPSHOT[] = {0xAA, 0x05, 0x00, 0x00, 0x00, 0x00};
const unsigned char GET_SNAPSHOT[] = {0xAA, 0x04, 0x01, 0x00, 0x00, 0x00};

const unsigned char GENERAL_ACK[] = {0xAA, 0x0E, 0x00, 0x00, 0x00, 0x00};


const int image_resolution = 160*128*2;
const int image_buffer_size = 50000;

int cmd_buffer_pos = 0;
int img_buffer_pos = 0;
unsigned char cmd_buffer[12] = {};
Serial uCam (TX_PIN, RX_PIN, 512);
Serial pc(USBTX,USBRX);
DigitalOut reset(RESET_PIN);
// unsigned char *po = new unsigned char[100];
unsigned char image_buffer[image_buffer_size];
bool serial_picture_data = false;

DigitalOut led1(LED1);
