#include "uCam.h"

uCam::uCam(PinName tx, PinName rx, PinName resetPin, Serial *debug) : _reset(resetPin) {
    _uCam = new BufferedSerial(tx, rx, 512);
    //_uCam->baud(57600);
    _uCam->baud(230400);
//    _uCam->baud(115200);
//    _uCam->baud(460800);
//    _uCam->baud(921600);
    _uCam->attach(callback(this, &uCam::_cmd_rx_callback), Serial::RxIrq);
    _debug = debug;
    _timeout = 5;
    _cmd_buffer_pos = 0;
    _img_buffer_pos = 0;
    _clear_cmd_buffer();
    hard_reset();
}

void uCam::_cmd_rx_callback() {
    _cmd_buffer[_cmd_buffer_pos++] = _uCam->getc();
}

void uCam::_img_rx_callback() {
//    _img_buffer[_img_buffer_pos++] = (unsigned char) _uCam->getc();
//    _it = _img_buffer->insert(_it, _uCam->getc());
//    _img_buffer.push_back(_uCam->getc());
    _img_tmp_buffer[_img_buffer_pos++] = _uCam->getc();
}

void uCam::_rx_callback() {
    if (_serial_picture_data) {
        *(_image_buffer + _img_buffer_pos++) = _uCam->getc();
    } else {
        _cmd_buffer[_cmd_buffer_pos++] = _uCam->getc();
    }
}

void uCam::hard_reset() {
    _reset = 0;
    wait(0.1);
    _reset = 1;
    wait(0.5);
}

void uCam::_clear_cmd_buffer() {
    _cmd_buffer_pos = 0;
    for (int i = 0; i < 12; i++) {
        _cmd_buffer[i] = 0x00;
    }
}

int uCam::get_image_data_size(bool image_data_only) {
    if (image_data_only) {
        return _img_data_size;
    }
    return _img_buffer_pos;
}

vector<unsigned char> *uCam::get_image_data() {
    return &_img_buffer;
}

int uCam::get_number_of_data_pckg() {
    return _number_of_pckg;
}

void uCam::free_buffer() {
    free(_image_buffer);
}

bool uCam::reset() {
    _debug->printf("\n\r-- uCam Reset --");
    _clear_cmd_buffer();
    for (int i = 0; i < 6; i++) {
        _uCam->putc(RESET_CMD[i]);
    }

    wait_ms(5);

    // Something went wrong - return false
    _debug->printf("\n\rNo ACK:\n\r");
    for (int j = 0; j < 12; j++) {
        _debug->printf("0x%02X ", _cmd_buffer[j]);
    }

    return true;
}

bool uCam::set_baud() {
    _debug->printf("\n\r-- uCam Baud --");
    for (int i = 0; i < 6; i++) {
        _uCam->putc(BAUD_1228800[i]);
    }

    wait_ms(5);

//    _debug->printf("\n\rBaud rate response.");
//    for (int j = 0; j < 12; j++) {
//        _debug->printf("0x%02X ", _cmd_buffer[j]);
//    }

    // Check if the response was an ACK
    if (_cmd_buffer[0] == 0xAA && _cmd_buffer[1] == 0x0E && _cmd_buffer[2] == 0x07 && _cmd_buffer[4] == 0x00 && _cmd_buffer[5] == 0x00) {
        // An ACk was received so we can now set the mbed to the new baud rate
        _uCam->baud(1228800);
        // New baud rate confirmed - return true
        _debug->printf("\n\rNew baud rate set!");
        _clear_cmd_buffer();
        baud = true;

        return true;
    } else
        // Something went wrong - return false
        _debug->printf("\n\rShit :/");

    return false;
}

bool uCam::sync() {
    int timeout = _timeout;

    // Start by Initializing communcation to uCam
    _debug->printf("\n\r-- uCam Sync --");
    // This will give 60 attempts to sync with the uCam module
    for (int i = 0; i < 60; i++) {
        // Send out the sync command
        //_debug->printf("\n\rSending out SYNC command\n\r");
        for (int j = 0; j < 6; j++) {
            _uCam->putc(SYNC[j]);
        }

        wait_ms(1);
//        for (int j = 0; j < 12; j++) {
//            _cmd_buffer[j] = _uCam->getc();
//        }

        // Check if it was an ACK
        if (_cmd_buffer[0] == 0xAA && _cmd_buffer[1] == 0x0E && _cmd_buffer[2] == 0x0D && _cmd_buffer[4] == 0x00 && _cmd_buffer[5] == 0x00) {
            _debug->printf("\n\rACK received");

            if (_cmd_buffer[6] == 0xAA && _cmd_buffer[7] == 0x0D && _cmd_buffer[8] == 0x00 && _cmd_buffer[9] == 0x00 && _cmd_buffer[10] == 0x00 && _cmd_buffer[11] == 0x00) {
                _debug->printf("\n\rSYNC received");
                // Send out an ACK as response
                _debug->printf("\n\rSending out ACK command");
                for (int j = 0; j<6; j++) {
                    _uCam->putc(ACK[j]);
                }
                _debug->printf("\n\rSYNC complete after %d attempts", i + 1);

                _clear_cmd_buffer();
                synced = true;
                return true;
            }
        }
        else {
            wait_ms(timeout++);
//            timer.start();
//            _debug->printf("\n\rNo ACK:\n\r");
//            for (int j = 0; j < 12; j++) {
//                _debug->printf("0x%02X ", _cmd_buffer[j]);
//            }
//            timer.stop();
//            _debug->printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));
//            timer.reset();
//            wait_ms(10);
        }

//        _debug->printf("\n\rWait: %d", timeout);
    }

    _debug->printf("\n\rSync failed!");

    _clear_cmd_buffer();
    return false;
}

bool uCam::get_snapshot() {
    int image_size = 0;
    _serial_picture_data = false;
    _img_buffer_pos = 0;

    _debug->printf("\n\r-- uCam SNAPSHOT initial --");
    for (int i = 0; i < 6; i++) {
        _uCam->putc(JPEG_PICTURE_INIT[i]);
    }

    wait_ms(50);
    // Check if the response was an ACK
    if (_cmd_buffer[0] == 0xAA && _cmd_buffer[1] == 0x0E && _cmd_buffer[2] == 0x01 && _cmd_buffer[4] == 0x00 && _cmd_buffer[5] == 0x00) {
        _debug->printf("\n\rACK received");
        _clear_cmd_buffer();
    } else {
        // Something went wrong - return false
        _debug->printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 12; j++) {
            _debug->printf("0x%02X ", _cmd_buffer[j]);
        }

        return false;
    }

    _debug->printf("\n\r-- uCam SET PACKAGE SIZE --");
    for (int i = 0; i < 6; i++) {
        _uCam->putc(SET_PACKAGE_SIZE[i]);
    }

    wait_ms(50);
    // Check if the response was an ACK
    if (_cmd_buffer[0] == 0xAA && _cmd_buffer[1] == 0x0E && _cmd_buffer[2] == 0x06 && _cmd_buffer[4] == 0x00 && _cmd_buffer[5] == 0x00) {
        _debug->printf("\n\rACK received");
        _clear_cmd_buffer();
    } else {
        // Something went wrong - return false
        _debug->printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 12; j++) {
            _debug->printf("0x%02X ", _cmd_buffer[j]);
        }

        return false;
    }

    wait_ms(50);
    _debug->printf("\n\r-- uCam MAKE SNAPSHOT --");
    for (int i = 0; i < 6; i++) {
        _uCam->putc(MAKE_SNAPSHOT[i]);
    }

    wait_ms(50);
    // Check if the response was an ACK
    if (_cmd_buffer[0] == 0xAA && _cmd_buffer[1] == 0x0E && _cmd_buffer[2] == 0x05 && _cmd_buffer[4] == 0x00 && _cmd_buffer[5] == 0x00) {
        _debug->printf("\n\rACK received");
        _clear_cmd_buffer();
    } else {
        _debug->printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 6; j++) {
            _debug->printf("0x%02X ", _cmd_buffer[j]);
        }

        return false;
    }

    wait_ms(150);
    _debug->printf("\n\r-- uCam GET SNAPSHOT --");
    for (int i = 0; i < 6; i++) {
        _uCam->putc(GET_SNAPSHOT[i]);
    }

    wait_ms(1);
    // Check if the response was an ACK
    if (_cmd_buffer[0] == 0xAA && _cmd_buffer[1] == 0x0E && _cmd_buffer[2] == 0x04 && _cmd_buffer[4] == 0x00 && _cmd_buffer[5] == 0x00) {
        _debug->printf("\n\rACK received");

        image_size = (image_size << 8) | _cmd_buffer[11];
        image_size = (image_size << 8) | _cmd_buffer[10];
        image_size = (image_size << 8) | _cmd_buffer[9];

        _debug->printf("\n\r Image size: %d", image_size);
        _img_data_size = image_size;
        _clear_cmd_buffer();
    } else {
        _debug->printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 6; j++) {
            _debug->printf("0x%02X ", _cmd_buffer[j]);
        }

        return false;
    }

    _uCam->attach(callback(this, &uCam::_img_rx_callback), Serial::RxIrq);
    _serial_picture_data = true;
    double tmp_number_of_pckgs = (double) image_size / (double) (512 - 6);
    int number_of_pckgs = ceil(image_size / (512 - 6));

    if (tmp_number_of_pckgs > (double) number_of_pckgs) {
        number_of_pckgs++;
    }

//    _image_buffer = (unsigned char *) malloc(sizeof(unsigned char) * (image_size + number_of_pckgs * 6));
//    _img_buffer = new vector<unsigned char>(image_size + number_of_pckgs * 6);
//    _it = _img_buffer->begin();
//    _img_buffer = new vector<unsigned char>();


    _debug->printf("\n\rCapacity of vector: %ld, size: %ld", _img_buffer.capacity(), _img_buffer.size());
    _debug->printf("\n\rNumber of packages: %d\n\r", number_of_pckgs);
    int pckg = 0;
    int ack_nr = 0x00;
    int old_pos = 0;
    while(pckg < number_of_pckgs) {
        old_pos = _img_buffer_pos;
        for (int i = 0; i < 6; i++) {
            if (i == 4) {
                _uCam->putc(ack_nr);

                continue;
            }
            _uCam->putc(GENERAL_ACK[i]);
        }
        ack_nr++;
        wait_ms(25);
        for (int x = 4; x < _img_buffer_pos-2; x++) {
            _img_buffer.push_back(_img_tmp_buffer[x]);
        }

        _debug->printf("\n\rReceived data size: %d: %d -> %d -> %ld", pckg, _img_buffer_pos, _img_buffer_pos % 512, _img_buffer.size());
        _img_buffer_pos = 0;
        // if (old_pos == img_buffer_pos) {
        //     break;
        // }

//        if (pckg == 0 || pckg == 1 || pckg == 2 || _img_buffer_pos > image_size) {
//
//            for (int j = old_pos; j < _img_buffer_pos; j++) {
//                _debug->printf("0x%02X ", _image_buffer[j]);
//            }
//        }

        pckg++;
        // _debug->printf("\n\n\rDATA ID: 0x%02X\n\r", ack_nr++);
        // for (int j = old_pos; j < img_buffer_pos; j++) {
        //     _debug->printf("0x%02X ", image_buffer[j]);
        // }
    }

    _serial_picture_data = false;

    wait_ms(1);
    for (int i = 0; i < 6; i++) {
        if (i == 4 || i == 5) {
            _uCam->putc(0xF0);

            continue;
        }
        _uCam->putc(GENERAL_ACK[i]);
    }

    wait_ms(5);
    _debug->printf("\n\rReceived data size: %d vs %d\n\r", _img_buffer_pos, (image_size + number_of_pckgs * 6));
    _uCam->attach(callback(this, &uCam::_rx_callback), Serial::RxIrq);

//    vector<unsigned char>::iterator it;
//    for (it = _img_buffer.begin(); it < _img_buffer.end(); it++) {
//        _debug->printf("%02x", *it);
//    }

    return true;
}

