#include "hand.h"

void RxCallback() {
    if (serial_picture_data) {

        image_buffer[img_buffer_pos++] = uCam.getc();

        //pc.printf("0x%02X ", image_buffer[img_buffer_pos++]);
    } else {
        led1 = 1;
        cmd_buffer[cmd_buffer_pos++] = uCam.getc();
        led1 = 0;
    }
}

void HardReset() {
    reset = 0;
    wait(0.1);
    reset = 1;
}

void CleanCMDBuffer() {
    cmd_buffer_pos = 0;
    for (int i = 0; i < 12; i++) {
        cmd_buffer[i] = 0x00;
    }
}

bool Sync() {
    int timeout = 5;

    // Start by Initializing communcation to uCam
    pc.printf("\n\r-- uCam Sync --");
    // This will give 60 attempts to sync with the uCam module
    for (int i = 0; i < 60; i++) {
        // Send out the sync command
        //pc.printf("\n\rSending out SYNC command\n\r");
        for (int j = 0; j < 6; j++) {
           uCam.putc(SYNC[j]);
        }

        // Check if it was an ACK
        if (cmd_buffer[0] == 0xAA && cmd_buffer[1] == 0x0E && cmd_buffer[2] == 0x0D && cmd_buffer[4] == 0x00 && cmd_buffer[5] == 0x00) {
            pc.printf("\n\rACK received");

            if (cmd_buffer[6] == 0xAA && cmd_buffer[7] == 0x0D && cmd_buffer[8] == 0x00 && cmd_buffer[9] == 0x00 && cmd_buffer[10] == 0x00 && cmd_buffer[11] == 0x00) {
                pc.printf("\n\rSYNC received");
                // Send out an ACK as response
                pc.printf("\n\rSending out ACK command");
                for (int j = 0; j<6; j++) {
                    uCam.putc(ACK[j]);
                }
                pc.printf("\n\rSYNC complete after %d attempts", i + 1);

                CleanCMDBuffer();
                return true;
            }
        }

        wait_ms(timeout++);
    }

    CleanCMDBuffer();
    return false;
}

bool GetSnapshot() {
    int image_size = 0;
    serial_picture_data = false;
    img_buffer_pos = 0;

    pc.printf("\n\r-- uCam SNAPSHOT initial --");
    for (int i = 0; i < 6; i++) {
       uCam.putc(JPEG_PICTURE_INIT[i]);
    }

    wait_ms(50);
    // Check if the response was an ACK
    if (cmd_buffer[0] == 0xAA && cmd_buffer[1] == 0x0E && cmd_buffer[2] == 0x01 && cmd_buffer[4] == 0x00 && cmd_buffer[5] == 0x00) {
        // An ACk was received so we can now set the mbed to the new baud rate
        // New baud rate confirmed - return true
        pc.printf("\n\rACK received");
        CleanCMDBuffer();
    } else {
        // Something went wrong - return false
        pc.printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 12; j++) {
            pc.printf("0x%02X ", cmd_buffer[j]);
        }

        return false;
    }

    pc.printf("\n\r-- uCam SET PACKAGE SIZE --");
    for (int i = 0; i < 6; i++) {
       uCam.putc(SET_PACKAGE_SIZE[i]);
    }

    wait_ms(50);
    // Check if the response was an ACK
    if (cmd_buffer[0] == 0xAA && cmd_buffer[1] == 0x0E && cmd_buffer[2] == 0x06 && cmd_buffer[4] == 0x00 && cmd_buffer[5] == 0x00) {
        // An ACk was received so we can now set the mbed to the new baud rate
        // New baud rate confirmed - return true
        pc.printf("\n\rACK received");
        CleanCMDBuffer();
    } else {
        // Something went wrong - return false
        pc.printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 12; j++) {
            pc.printf("0x%02X ", cmd_buffer[j]);
        }

        return false;
    }

    wait_ms(50);
    pc.printf("\n\r-- uCam MAKE SNAPSHOT --");
    for (int i = 0; i < 6; i++) {
       uCam.putc(MAKE_SNAPSHOT[i]);
    }

    wait_ms(50);
    if (cmd_buffer[0] == 0xAA && cmd_buffer[1] == 0x0E && cmd_buffer[2] == 0x05 && cmd_buffer[4] == 0x00 && cmd_buffer[5] == 0x00) {
        pc.printf("\n\rACK received");
        CleanCMDBuffer();
    } else {
        pc.printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 6; j++) {
            pc.printf("0x%02X ", cmd_buffer[j]);
        }

        return false;
    }

    wait_ms(70);
    pc.printf("\n\r-- uCam GET SNAPSHOT --");
    for (int i = 0; i < 6; i++) {
       uCam.putc(GET_SNAPSHOT[i]);
    }

    wait_ms(1);
    if (cmd_buffer[0] == 0xAA && cmd_buffer[1] == 0x0E && cmd_buffer[2] == 0x04 && cmd_buffer[4] == 0x00 && cmd_buffer[5] == 0x00) {
        pc.printf("\n\rACK received");

        image_size = (image_size << 8) | cmd_buffer[11];
        image_size = (image_size << 8) | cmd_buffer[10];
        image_size = (image_size << 8) | cmd_buffer[9];

        pc.printf("\n\r Image size: %d", image_size);
    } else {
        pc.printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 6; j++) {
            pc.printf("0x%02X ", cmd_buffer[j]);
        }

        return false;
    }

    serial_picture_data = true;

    // wait_ms(5);
    //
    // for (int j = 0; j < 12; j++) {
    //     pc.printf("0x%02X ", cmd_buffer[j]);
    // }

    double tmp_number_of_pckgs = (double) image_size / (double) (512 - 6);
    int number_of_pckgs = ceil(image_size / (512 - 6));

    if (tmp_number_of_pckgs > (double) number_of_pckgs) {
        number_of_pckgs++;
    }

    pc.printf("\n\rNumber of packages: %d\n\r", number_of_pckgs);
    int pckg = 0;
    int ack_nr = 0x00;
    int old_pos;
    while(pckg < number_of_pckgs) {
        old_pos = img_buffer_pos;
        for (int i = 0; i < 6; i++) {
            if (i == 4) {
                uCam.putc(ack_nr);

                continue;
            }
            uCam.putc(GENERAL_ACK[i]);
        }
        ack_nr++;
        wait_ms(25);
        pc.printf("\n\rReceived data size: %d\n\r", img_buffer_pos);
        // if (old_pos == img_buffer_pos) {
        //     break;
        // }

        // if (pckg == 0 || pckg == 1 || pckg == 2 || img_buffer_pos > image_size) {
        //
        //     for (int j = old_pos; j < img_buffer_pos; j++) {
        //         pc.printf("0x%02X ", image_buffer[j]);
        //     }
        // }

        pckg++;
        // pc.printf("\n\n\rDATA ID: 0x%02X\n\r", ack_nr++);
        // for (int j = old_pos; j < img_buffer_pos; j++) {
        //     pc.printf("0x%02X ", image_buffer[j]);
        // }
    }

    wait_ms(1);
    for (int i = 0; i < 6; i++) {
        if (i == 4 || i == 5) {
            uCam.putc(0xF0);

            continue;
        }
        uCam.putc(GENERAL_ACK[i]);
    }

    serial_picture_data = false;

    return true;
}

bool GetRawPicture() {
    pc.printf("\n\r-- uCam RAW Picture initial --");
    for (int i = 0; i < 6; i++) {
       uCam.putc(RAW_PICTURE_INIT[i]);
    }

    wait_ms(50);
    // Check if the response was an ACK
    if (cmd_buffer[0] == 0xAA && cmd_buffer[1] == 0x0E && cmd_buffer[2] == 0x01 && cmd_buffer[4] == 0x00 && cmd_buffer[5] == 0x00) {
        // An ACk was received so we can now set the mbed to the new baud rate
        // New baud rate confirmed - return true
        pc.printf("\n\rACK received");
        CleanCMDBuffer();
        serial_picture_data = true;
    } else {
        // Something went wrong - return false
        pc.printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 12; j++) {
            pc.printf("0x%02X ", cmd_buffer[j]);
        }

        return false;
    }

    pc.printf("\n\r-- uCam GET RAW Picture --");
    for (int i = 0; i < 6; i++) {
       uCam.putc(GET_RAW_PICTURE[i]);
    }
    //
    // for (int j = 0; j < 6; j++) {
    //     pc.printf("0x%02X ", image_buffer[j]);
    // }

    wait_ms(50);
    if (image_buffer[0] == 0xAA && image_buffer[1] == 0x0E && image_buffer[2] == 0x04 && image_buffer[4] == 0x00 && image_buffer[5] == 0x00) {
        pc.printf("\n\rACK received");
    } else {
        pc.printf("\n\rNo ACK:\n\r");
        for (int j = 0; j < 6; j++) {
            pc.printf("0x%02X ", image_buffer[j]);
        }

        return false;
    }

    wait_ms(500);
    // wait(1);
    // //
    // for (int j = 0; j < img_buffer_pos; j++) {
    //     pc.printf("0x%02X ", image_buffer[j]);
    // }

    // while (img_buffer_pos < image_buffer_size) {
    //     wait(0.5);
    //     pc.printf("\n\rIMG Buffer pos: %d, IMG Buffer size: %d", img_buffer_pos, image_buffer_size);
    // }
    //
    // if (image_buffer[image_buffer_size-6] == 0xAA && image_buffer[image_buffer_size-5] == 0x0A && image_buffer[image_buffer_size-4] == 0x02) {
    //     pc.printf("\n\rImage downloaded");
    // } else {
    //     pc.printf("\n\rWrong data received:\n\r");
    //     for (int j = image_buffer_size - 6; j < image_buffer_size; j++) {
    //         pc.printf("0x%02X ", image_buffer[j]);
    //     }
    //
    //     return false;
    // }
    //
    // pc.printf("\n\r-- uCam DATA ACK --");
    // for (int i = 0; i < 6; i++) {
    //    uCam.putc(RAW_PICTURE_ACK[i]);
    // }

    return true;
}

bool SetBaud() {
    for (int i = 0; i < 6; i++) {
       uCam.putc(BAUD_921600[i]);
    }

    wait_ms(5);

    pc.printf("\n\rBaud rate response :/\n\r");
    for (int j = 0; j < 12; j++) {
        pc.printf("0x%02X ", cmd_buffer[j]);
    }

    // Check if the response was an ACK
    if (cmd_buffer[0] == 0xAA && cmd_buffer[1] == 0x0E && cmd_buffer[2] == 0x07 && cmd_buffer[4] == 0x00 && cmd_buffer[5] == 0x00) {
        // An ACk was received so we can now set the mbed to the new baud rate
        uCam.baud(921600);
        // New baud rate confirmed - return true
        pc.printf("\n\rNew baud rate set!");

        return true;
    } else
        // Something went wrong - return false
        pc.printf("\n\rShit :/");

    return false;
}

int main() {
    pc.baud(9600);

    mbed_stats_heap_t heap_stats;
    mbed_stats_heap_get(&heap_stats);

    printf("Current heap: %lu\r\n", heap_stats.current_size);
    printf("Max heap size: %lu\r\n", heap_stats.max_size);

    uCam.baud(230400);
    uCam.attach(&RxCallback, Serial::RxIrq);

    // uCamIII cam(TX_PIN, RX_PIN, RESET_PIN);

    Timer timer;
    timer.start();

    pc.printf("\n\r[TIME]: %d", timer.read_ms());
    //
    // if (cam.Sync()) {
    //     printf("\n\rTest finished!");
    // } else {
    //     printf("\n\rTest failed!");
    // }

    for (int i = 0; i < 1; i++) {
        CleanCMDBuffer();
        HardReset();
        // wait_ms(50);

        led1 = 0;
        // pc.printf("\n\rUSB TX: %d, USB RX: %d", USBTX, USBRX);
        // pc.printf("\n\rTX: %d, RX: %d", TX_PIN, RX_PIN);

        if (Sync()) {
    //         //SetBaud();
    //         //GetRawPicture();
            GetSnapshot();
            pc.printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));

            mbed_stats_heap_get(&heap_stats);
            printf("Current heap: %lu\r\n", heap_stats.current_size);
            printf("Max heap size: %lu\r\n", heap_stats.max_size);
    //
    //         // wait(2);
    //         // GetSnapshot();
    //         //pc.printf("\n\r -- IDE DALEJ? --\n\r");
    //
    //         ;
    //
    //         // pc.printf("\n\rReceived data size: %d", img_buffer_pos);
    //         // wait_ms(500);
    //         // pc.printf("\n\rReceived data size: %d", img_buffer_pos);
    //         // wait_ms(500);
    //         // pc.printf("\n\rReceived data size: %d", img_buffer_pos);
    //         // wait_ms(500);
    //         // pc.printf("\n\rReceived data size: %d\n\r", img_buffer_pos);
    //         //
    //         int i = 0;
    //         int pckg_nr = 1;
    //         // pc.printf("\n\rPackage number: %d\n\r", pckg_nr);
    //
    //
            pc.printf("\n\n\n\n\r");
            for (int j = 0; j < img_buffer_pos-2; j++) {
                if (i == 512) {
                    pckg_nr++;
                    i = 0;
                    //pc.printf("\n\rPackage number: %d\n\r", pckg_nr);
                }

                if (i < 4) {
                    i++;
                    continue;
                }

                if (i > 509) {
                    i++;
                    continue;
                }

                pc.printf("%02X ", image_buffer[j]);
                i++;
            }
    //
    //         // if (GetRawPicture()) {
    //         //     wait(5);
    //         //     pc.printf("\n\rPICTURE: \n\r");
    //         //     for (int j = 0; j < image_buffer_size; j++) {
    //         //         pc.printf("0x%02X ", image_buffer[j]);
    //         //     }
    //         // }
    //     } else {
    //         for (int j = 0; j < 12; j++) {
    //             pc.printf("0x%02X ", cmd_buffer[j]);
    //         }
        }
    }
}
