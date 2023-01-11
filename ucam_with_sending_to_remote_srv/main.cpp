#include "main.h"


SDBlockDevice sd(SPI_MOSI, SPI_MISO, SPI_SCK, SPI_CS);
FATFileSystem fs("sd", &sd);
Serial pc(USBTX,USBRX);
uCam cam1 = uCam(MBED_CONF_APP_UCAM_TX, MBED_CONF_APP_UCAM_RX, MBED_CONF_APP_UCAM_RESET, &pc);
uCam cam2 = uCam(MBED_CONF_APP_UCAM_TX_S, MBED_CONF_APP_UCAM_RX_S, MBED_CONF_APP_UCAM_RESET_S, &pc);
RTCx rtc(I2C_SDA, I2C_SCL, &pc);
DigitalIn enable(USER_BUTTON);
TCPSocket socket;
//LowPowerTimeout halfSecondTimeout;
//InterruptIn button(USER_BUTTON);

//int buff_pos = 0;

//void rx_callback() {t
//    buffer[buff_pos++] = pc.getc();
//}

bool expired = false;

void callback(void) {
    expired = true;
}

void return_error(int ret_val) {
    if (ret_val)
        pc.printf("\r Failure. %d\n", ret_val);
    else
        pc.printf("\r done.");
}

bool errno_error(void* ret_val) {
    if (ret_val == NULL) {
        pc.printf("\r Failure. %d \n", errno);

        return true;
    } else {
        pc.printf("\r done.");

        return false;
    }
}

void dump_response(HttpResponse* res) {
    pc.printf("\n\rStatus: %d - %s\n", res->get_status_code(), res->get_status_message().c_str());

    pc.printf("\n\rHeaders:\n");
    for (size_t ix = 0; ix < res->get_headers_length(); ix++) {
        pc.printf("\t\r%s: %s\n", res->get_headers_fields()[ix]->c_str(), res->get_headers_values()[ix]->c_str());
    }
    pc.printf("\n\rBody (%d bytes):\n\n%s\n", res->get_body_length(), res->get_body_as_string().c_str());
}

bool send_data(NetworkInterface *network, const char *image_chunk, int image_chunk_size, bool first_packet = false) {

//    char buffer[] = "http://httpbin.org/put";
//    char buffer[] = "http://10.1.1.63:8080/put";
    char buffer[255];
    char time_buffer[32];
    time_t timestamp = rtc.get_timestamp();
    strftime(time_buffer, 32, "%Y%m%d-%H%M%S", localtime(&timestamp));
    sprintf(buffer, "%s%s/observations/%s?formattag=nucleo-test-data", "http://10.1.1.33:8080/observation-units/", "nucleo01", time_buffer);
    pc.printf("\n\r%s", buffer);

    socket.open(network);
    socket.connect("10.1.1.33", 8080);

    ParsedUrl* _parsed_url;
    HttpRequestBuilder* _request_builder;

    size_t _size;
    _parsed_url = new ParsedUrl(buffer);
    _request_builder = new HttpRequestBuilder(HTTP_PUT, _parsed_url);
    _request_builder->set_header("Content-Type", "text/plain");
//    char request[512];
    char *ptr;
//    ptr = request;
    ptr = _request_builder->build(image_chunk, image_chunk_size, _size);
    pc.printf("\n\r%s", ptr);

//    // Send a simple http request
//    char sbuffer[] = "GET / HTTP/1.1\r\nHost: developer.mbed.org\r\n\r\n";
    int scount = socket.send(ptr, _size);
    pc.printf("\n\rsent %d\r\n", scount);
//
    // Recieve a simple http response and print out the response line
    char rbuffer[64];
    int rcount = socket.recv(rbuffer, sizeof rbuffer);
    pc.printf("\n\rrecv %d [%.*s]\r\n", rcount, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);
    pc.printf("\n\r%s", rbuffer);
//
//    // Close the socket to return its memory and bring down the network interface
    socket.close();
    pc.printf("\n\rDone!");
//    HttpRequest* post_req = new HttpRequest(network, HTTP_PUT, buffer);
//    post_req->set_header("Content-Type", "text/plain");
////                const char body[] = "Test data";
//
////    pc.printf("\n\rLength: %d", oss.tellp());
////    string test = "FF001244";
////    const char * c = test.c_str();
////    const char * c = image_chunk;
//    HttpResponse* post_res = post_req->send(image_chunk, image_chunk_size);
//    pc.printf("\n\rRequest sent!");
//    if (!post_res) {
//        pc.printf("HttpRequest failed (error code %d)\n", post_req->get_error());
//
//        return false;
//    }
//
//    pc.printf("\n\r----- HTTP POST response -----\n\r");
//    dump_response(post_res);
//
//    delete post_req;

    return true;
}

bool make_photo(uCam *cam, NetworkInterface *network) {
    if (cam->synced || cam->sync()) {
        if (!cam->baud) {
            cam->set_baud();
        }
        time_t timestamp = rtc.get_timestamp();
        char file_path[20];
        sprintf(file_path, "/sd/%d.jpeg", timestamp);
        if (!cam->get_snapshot()) {
            return false;
        }

        Timer timer;
        ostringstream oss;

//        pc.printf("\n\rOpening a new file, %s: \n\r", file_path);
//        FILE* fd = fopen(file_path, "wb+");
//        if (!errno_error(fd)) {
        if (1) {
            timer.start();
//            socket.open(network);
//            socket.connect("10.1.1.33", 8080);

            pc.printf("\n\rHERE?");

            char buffer[255];
            char time_buffer[32];
            char *ptr;

            strftime(time_buffer, 32, "%Y%m%d-%H%M%S", localtime(&timestamp));
            sprintf(buffer, "%s%s/observations/%s?formattag=nucleo-test-data",
                    "http://10.1.1.33:8080/observation-units/", "nucleo01", time_buffer);

            ParsedUrl *_parsed_url;
            HttpRequestBuilder *_request_builder;
            size_t _size = 0;
            _parsed_url = new ParsedUrl(buffer);
            _request_builder = new HttpRequestBuilder(HTTP_PUT, _parsed_url);
            _request_builder->set_header("Content-Type", "text/plain");
            int scount;
//            int image_data_size = cam->get_image_data_size(false);
            vector<unsigned char> *image_data = cam->get_image_data();
//            int i = 0, pckg_nr = 1;
//            int img_buff_pos = 0;
//            unsigned char img_buffer[512];
//            string img_string;
//            int string_size = 0;
            bool first_chunk = true;

//            oss << hex << setfill('0') << setw(2);

            pc.printf("\n\r");
            for (int i = 0; i < image_data->size(); i++) {
//                oss << hex << setw(2) << setfill('0') << +image_data->at(i);
                oss << +static_cast<char>(image_data->at(i));
                if (i < 10) {
                    pc.printf("\n\rBIN? %c", image_data->at(i));
                    pc.printf("\n\rBIN2? %c", oss.str().c_str());
                }
                if (oss.tellp() != 0 && oss.tellp() % 1800 == 0 || i == image_data->size() - 1) {
                    if (first_chunk) {
                        ptr = _request_builder->build(oss.str().c_str(), (2 * image_data->size()), _size);
//                        _size -= (2 * image_data->size());
//                        _size += oss.tellp();
//                        _size -= 8;
                        _size = strlen(ptr);
//                        scount = socket.send(ptr, _size);
                        pc.printf("\n\r%s", ptr);
                        pc.printf("\n\r%d", _size);
                        pc.printf("\n\r%d", strlen(ptr));
//                        pc.printf("\n\r%s", oss.str().c_str());
                        first_chunk = false;
                    } else {
                        _size = oss.tellp();
//                        scount = socket.send(oss.str().c_str(), _size);
                    }

//                    if (i == image_data->size() - 1) {
//                        pc.printf("%s", oss.str().c_str());
//                    }
//                    pc.printf("\n\r%s", oss.str().c_str());
//                    pc.printf("\n\rsent %d\r\n", scount);
                    oss.str("");
                    oss.clear();
                }
                //std::string test{ image_data->begin(), image_data->end() };
                //copy(image_data->begin(), image_data->end(), ostream_iterator<char>(oss));
            //for (it = image_data->begin(); it < image_data->end(); it++) {
//                if (first_chunk) {
//                    ptr = _request_builder->build(oss.str().c_str(), 2 * image_data->size(), _size);
//                    _size -= (2 * image_data->size());
//                    _size += oss.tellp();
//                    scount = socket.send(ptr, _size);
//                    first_chunk = false;
//                    continue;
//                }
            }

//            char rbuffer[250];
//            int rcount = socket.recv(rbuffer, sizeof rbuffer);
//            pc.printf("\n\rrecv %d [%.*s]\r\n", rcount, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);
//            pc.printf("\n\r%s", rbuffer);
//
//            // Close the socket to return its memory and bring down the network interface
//            socket.close();
            pc.printf("\n\rDone!");

//            for (int j = 0; j < image_data_size - 2; j++) {
//
//                if (string_size > 0 && string_size % 506 == 0) {
//
//                }
//
//                if (i == 512) {
//                    if (first_chunk) {
//                        ptr = _request_builder->build(oss.str().c_str(), 2 * cam->get_image_data_size(true), _size);
////                        pc.printf("\n\r%s", ptr);
//
////                        pc.printf("\n\rString size: %d", string_size);
//
//                        _size -= (2 * cam->get_image_data_size(true));
//                        _size += oss.tellp();
//                        scount = socket.send(ptr, _size);
////                        pc.printf("\n\rSize: %d", _size);
//                        first_chunk = false;
//                    } else {
//                        _size = oss.tellp();
//                        scount = socket.send(oss.str().c_str(), _size);
////                        pc.printf("\n\rSize: %d", _size);
//                    }
//
////                    pc.printf("\n\r%s", oss.str().c_str());
//
//                    pc.printf("\n\rsent %d\r\n", scount);
//                    oss.str("");
//                    oss.clear();
//                    pckg_nr++;
//                    i = 0;
//                }
//
//                if (i < 4) {
//                    i++;
//                    continue;
//                }
//
//                if (i > 509) {
//                    i++;
//                    continue;
//                }
//
//                oss << hex << setw(2) << setfill('0') << +image_data[j];
////                img_buffer[i] = image_data[j];
////                img_string += image_data[j];
////                fprintf(fd, "%c", image_data[j]);
//                string_size++;
//                i++;
//            }
//
//            if (i > 0) {
//                _size = oss.tellp();
////                pc.printf("\n\rSize: %d", _size);
//                scount = socket.send(oss.str().c_str(), _size);
//
////                pc.printf("\n\r%s", oss.str().c_str());
//
//                pc.printf("\n\rsent %d\r\n", scount);
//            }
        }
//
//        char rbuffer[250];
//        int rcount = socket.recv(rbuffer, sizeof rbuffer);
//        pc.printf("\n\rrecv %d [%.*s]\r\n", rcount, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);
//        pc.printf("\n\r%s", rbuffer);
////
////    // Close the socket to return its memory and bring down the network interface
//        socket.close();
//        pc.printf("\n\rDone!");

        timer.stop();
        cam->free_buffer();
//            pc.printf("\n\rI vs image_size: %d -> %d", img_buff_pos, cam->get_image_data_size(true));
//            fwrite(img_buffer, sizeof(unsigned char), cam->get_image_data_size(true), fd);
//            pc.printf("\n\r%s", oss.str().c_str());


        pc.printf("\n\r[TIME OSS]: %f s", (timer.read_ms() / 1000.0));

        pc.printf("\n\rClosing file.\r");
//            fclose(fd);
//            cam->reset();

        return true;
//        }
    }

    return false;
}

int main() {
    pc.baud(921600);
    pc.printf("\n\rWaiting for user action!");
    while(1) {
        pc.printf("\n\rWaiting for user action!");
        if (!enable.read()) {
            break;
        }
        wait(1);
    }

    NetworkInterface *network;// = easy_connect(true);
//    if (!network) {
//        pc.printf("\n\rCannot connect to the network, see serial output");
//
//        return -1;
//    }

//    halfSecondTimeout.attach(callback, 3.5f);

//    button.fall(wake);
//
//    //Ensure that we only continue the program flow here after the timeout expired.
//    while(!expired) {
//        deepsleep();
//    }

//    pc.attach(rx_callback, Serial::RxIrq);

    Timer timer;
    Timer sub_timer;
    timer.start();

    pc.printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));

//    int i = 0;
//    while(enable.read()) {
////        if (buff_pos > 0) {
////            pc.printf("\n\rGot %d, %d", buff_pos, buff_pos % 512);
////        }
////        wait_ms(50);
//    }
//
//    pc.printf("\n\rFinally got: %d, %d", buff_pos, buff_pos % 512);

    if (rtc.lost_power()) {
        rtc.set_network(network);
        rtc.setup_time();
    }

    pc.printf("\n\rTime as seconds since January 1, 1970 = %ld", (long) rtc.get_timestamp());
    pc.printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));

    int i = 0;
    int j = 0;
    sub_timer.start();
    while (i < 1) {
        sub_timer.reset();
        if (make_photo(&cam1, network)) {
            j++;
        }
        pc.printf("\n\r[SUB TIME CAM1]: %f s", (sub_timer.read_ms() / 1000.0));
        sub_timer.reset();
        if (make_photo(&cam2, network)) {
            j++;
        }
        pc.printf("\n\r[SUB TIME CAM2]: %f s", (sub_timer.read_ms() / 1000.0));
        i++;
    }

    pc.printf("\n\rSuccesses: %d", j);
    pc.printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));

//    while(1) {
////        printf("\n\rWaiting for user action!");
//        if (!enable.read()) {
//            timer.reset();
////            cam.hard_reset();
//            if (cam1.sync()) {
//                if (!baud) {
//                    cam1.set_baud();
//                    baud = true;
//                }
//                cam1.get_snapshot();
//                pc.printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));
//            }
//        }
//        wait(1);
//    }


//    if (cam1.sync()) {
//        if (!baud) {
//            cam1.set_baud();
//            baud = true;
//        }
//        time_t timestamp = rtc.get_timestamp();
//        char file_path[20];
//        sprintf(file_path, "/sd/%d.raw", timestamp);
//        cam1.get_snapshot();
//        pc.printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));
////
//        pc.printf("\n\rOpening a new file, %s: \n\r", file_path);
//        FILE* fd = fopen(file_path, "wb+");
//        if (!errno_error(fd)) {
//            int i = 0;
//            int pckg_nr = 1;
////        pc.printf("\n\rPackage number: %d\n\r", pckg_nr);
//            write_timer.start();
//            for (int j = 0; j < cam1.get_image_data_size() - 2; j++) {
//                if (i == 512) {
////                fprintf(fd, "%02X ", buffer);
////                printf("\n\r%02X ", buffer);
////                byte_nr = 0;
//                    pckg_nr++;
//                    i = 0;
////                pc.printf("\n\rPackage number: %d\n\r", pckg_nr);
////                pc.printf("\n\r[512KB TIME]: %f s", (write_timer.read_ms() / 1000.0));
//                    write_timer.reset();
//                }
//
//                if (i < 4) {
//                    i++;
//                    continue;
//                }
//
//                if (i > 509) {
//                    i++;
//                    continue;
//                }
//
////            sprintf(buf_ptr, "%02X", cam1.get_image_data()[j]);
////            buffer[byte_nr++] = cam1.get_image_data()[j];
//
//                fprintf(fd, "%02X ", cam1.get_image_data()[j]);
////            pc.printf("%02X ", cam1.get_image_data()[j]);
//                i++;
//            }
//            pc.printf("\n\rClosing file.\r");
//            fclose(fd);
//            cam1.free_buffer();
//        }
//        pc.printf("\n\r[TIME]: %f s", (timer.read_ms() / 1000.0));
//    }

//    int error = 0;
//    printf("\n\rWelcome to the filesystem example.");
//
//    printf("\n\rOpening a new file, numbers.txt.\n\r");
//    FILE* fd = fopen("/sd/numbers.txt", "w+");
//    errno_error(fd);
//
//    for (int i = 0; i < 20; i++){
//        printf("Writing decimal numbers to a file (%d/20)\r", i);
//        fprintf(fd, "%d\n", i);
//    }
//    printf("\n\rWriting decimal numbers to a file (20/20) done.");
//
//    printf("\n\rClosing file.\r");
//    fclose(fd);
//    printf(" done.");
//
//    printf("\n\rRe-opening file read-only.");
//    fd = fopen("/sd/numbers.txt", "r");
//    errno_error(fd);
//
//    printf("\n\rDumping file to screen.\n\r");
//    char buff[16] = {0};
//    while (!feof(fd)){
//        int size = fread(&buff[0], 1, 15, fd);
//        fwrite(&buff[0], 1, size, stdout);
//    }
//    printf("\n\rEOF.");
//
//    printf("\n\rClosing file.\r");
//    fclose(fd);
//    printf(" done.");
//
//    printf("\n\rOpening root directory.");
//    DIR* dir = opendir("/sd/");
//    errno_error(fd);
//
//    struct dirent* de;
//    printf("\n\rPrinting all filenames:\n\r");
//    while((de = readdir(dir)) != NULL){
//        printf("  %s\n\r", &(de->d_name)[0]);
//    }
//
//    printf("\n\rCloseing root directory. ");
//    error = closedir(dir);
//    return_error(error);
//    printf("\n\rFilesystem Demo complete.\n\r");
//
//    while (true) {}

}
