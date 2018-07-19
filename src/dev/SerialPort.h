//
// Created by Thinkpad on 2017/9/19.
//

#ifndef AICAST_BACKHAUL_SERIALPORT_H
#define AICAST_BACKHAUL_SERIALPORT_H

#include <fcntl.h>  /* File Control Definitions          */
#include <termios.h>/* POSIX Terminal Control Definitions*/
#include <unistd.h> /* UNIX Standard Definitions         */
#include <errno.h>  /* ERROR Number Definitions          */
#include <lang/lang-log.h>
#include <dev/Selector.h>

namespace haul {

    using std::string;

#define CONFIG_BAUDRATE  B115200 //B256000

    //
    //Refer
    //  https://www.cmrr.umn.edu/~strupp/serial.html
    //  https://en.wikibooks.org/wiki/Serial_Programming/Serial_Linux
    //
    class SerialPort {
        int         _fd;
        termios     _bak;
    public:
        const string    name;
        SerialPort(string name):name(name) {
            _fd = open(name.c_str(), O_RDWR | O_NOCTTY);
            if (_fd < 0) {
                FATALF("faild open serial port {}", name);
            }
            tcgetattr(_fd, &_bak);
            termios options = _bak;
            cfsetispeed(&options, CONFIG_BAUDRATE);
            cfsetospeed(&options, CONFIG_BAUDRATE);
            options.c_cflag &= ~PARENB;      //no parity
            options.c_cflag &= ~CSTOPB;      //stopbits 1
            options.c_cflag &= ~CSIZE;       //databits 8
            options.c_cflag |=  CS8;
            options.c_cflag &= ~CRTSCTS;     //hw-flowctrl CTS/RTS off
            options.c_cflag |= CREAD | CLOCAL;   //turn on receiver
            options.c_iflag &= ~(IXON | IXOFF | IXANY);  //XON/XOFF off
            options.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC|IMAXBEL|BRKINT|IGNBRK);  //disable all maps, breaks, ...
            options.c_oflag &= ~(OPOST);  //output raw mode
            options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);   //NON Cannonical mode
            tcsetattr(_fd,TCSANOW,&options);  //apply settings immediately
            fcntl(_fd, F_SETFL, O_NONBLOCK);
        }
        int read(void *buff, int size) {
            errno = 0;
            return ::read(_fd, buff, size);
        }
        int write(void *data, int size) {
            return ::write(_fd, data, size);
        }
        ~SerialPort() {
            tcsetattr(_fd,TCSANOW,&_bak);
            close(_fd);
        }
        inline int getfd() {
            return _fd;
        }
    };
}


#endif //AICAST_BACKHAUL_SERIALPORT_H
