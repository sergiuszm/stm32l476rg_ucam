#include "uCamIII.h"

uCamIII::uCamIII(PinName tx, PinName rx, PinName resetPin) : _uCam(tx,rx), _reset(resetPin) {
    _uCam.baud(115200);
    _uCam.attach(callback(this, &uCamIII::_RxCallback), Serial::RxIrq);
    _timeout = 1;
    _buffer_pos = 0;
    for (int i = 0; i < 12; i++) {
        _buffer[i] = 0x00;
    }
    HardReset();
}

void uCamIII::_RxCallback() {
    // printf("\n\rWLAZLEM!\n\r");
    _buffer[_buffer_pos] = _uCam.getc();
    _buffer_pos++;
}

void uCamIII::HardReset() {
    _reset = 0;
    wait(0.1);
    _reset = 1;
    wait(0.1);
}

bool uCamIII::Sync() {
    // Start by Initializing communcation to uCam
    printf("\n\r-- uCam Initailse --");
    Timer timer;
    int difference = 0;
    for (int i = 0; i < 60; i++) {
        // Send out the sync command
        // timer.start();
        // printf("\n\rSending out SYNC command\n\r");
        // timer.stop();
        difference++;
        for (int j = 0; j < 6; j++) {
           _uCam.putc(SYNC[j]);
        }

        wait_ms(1);
        // Check if it was an ACK
        if (_buffer[0] == 0xAA && _buffer[1] == 0x0E && _buffer[2] == 0x0D && _buffer[4] == 0x00 && _buffer[5] == 0x00) {
            printf("\n\rACK received");

            if (_buffer[6] == 0xAA && _buffer[7] == 0x0D && _buffer[8] == 0x00 && _buffer[9] == 0x00 && _buffer[10] == 0x00 && _buffer[11] == 0x00) {
                printf("\n\rSYNC received");
                // Send out an ACK as response
                printf("\n\rSending out ACK command");
                for (int j = 0; j<6; j++) {
                    _uCam.putc(ACK[j]);
                }
                printf("\n\rSYNC complete after %d attempts",i+1);

                return true;
            } else {
                printf("\n\rNo SYNC received - trying again...");
            }
        } else {
            // difference = timer.read_ms();
            // timer.reset();
            // printf("\n\rNo ACK received - trying again.. | %d ", difference);
            // for (int j = 0; j < 12; j++) {
            //     printf("0x%02X ", _buffer[j]);
            // }
        }

        wait_ms(_timeout++);

    }

    return false;
}
