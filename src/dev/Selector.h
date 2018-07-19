//
// Created by Thinkpad on 2017/9/19.
//

#ifndef AICAST_BACKHAUL_SELECTOR_H
#define AICAST_BACKHAUL_SELECTOR_H


#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/param.h>

namespace haul {
    class Selector {
        fd_set   input;
        int      maxfd;
        struct timeval timeout;
    public:
        void reset() {
            FD_ZERO(&input);
            maxfd = 0;
        }
        void add(int fd) {
            FD_SET (fd, &input);
            maxfd = MAX(fd, maxfd);
        }
        int read(int mills=0) { //
            timeout.tv_sec = mills / 1000;
            timeout.tv_usec = (mills - timeout.tv_sec * 1000) * 1000;
            return select(maxfd + 1, &input, NULL, NULL, &timeout);
        }
        bool test(int fd) {
            return FD_ISSET(fd, &input);
        }
    };
}


#endif //AICAST_BACKHAUL_SELECTOR_H
