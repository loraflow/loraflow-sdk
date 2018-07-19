//
// Created by Thinkpad on 2017/9/5.
//

#ifndef AICAST_BACKHAUL_LANG_H
#define AICAST_BACKHAUL_LANG_H

#include <lang/constants.h>
#include <cstdint>
#include <functional>
#include <mutex>
#include <iostream>
#include <cstdlib>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>
#include <sys/time.h>
#include <unistd.h>
#include <cassert> // assert
#include <thread>
#include <csignal>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define NUMELMNT(_a) (sizeof(_a)/sizeof(_a[0]))
#define NUMCHAR(_a) (sizeof(_a) - sizeof(""))
#define INRANGE(_x, _a, _b) ((_x) >= (_a) && (_x) <= (_b))
#define ASSERT(e) assert(e)

#ifndef MAX
#define MAX(a, b)  ((a) >= (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a, b)  ((a) <= (b) ? (a) : (b))
#endif

#define __DATA_ALIGNED__      __attribute__ ((aligned (4)))

using namespace std;

namespace lang {

    namespace os {

        using timepoint = std::chrono::high_resolution_clock::time_point;

        void exclusively(function<void(void)> fn);

        inline timepoint now() {
            return std::chrono::high_resolution_clock::now();
        }
        inline int64_t mills(void) {
            //return now().time_since_epoch().count();
            return chrono::duration_cast<chrono::milliseconds >(
                    chrono::system_clock::now().time_since_epoch()
            ).count();
        }
        inline time_t micros(void) {
            struct timeval tm;
            gettimeofday(&tm, NULL);
            return tm.tv_sec*1000000 + tm.tv_usec;
        }

        inline time_t epoch() {
            return time(0);
        }

        inline void sleep_ms(int mills) {
            std::this_thread::sleep_for(std::chrono::milliseconds(mills));
        }

        inline string version() {
#ifndef ASZVER
#define ASZVER 0.0.0
#endif
#define _xstr(a) _str(a)
#define _str(a) #a
            return _xstr(ASZVER);
#undef _str
#undef _xstr
        }

        inline void enable_int(void (*handler)(int)) {
            struct sigaction sigIntHandler;
            sigIntHandler.sa_handler = handler;
            sigemptyset(&sigIntHandler.sa_mask);
            sigIntHandler.sa_flags = 0;
            sigaction(SIGINT, &sigIntHandler, NULL);
        }

        inline void handle_pipe() {
            signal(SIGPIPE, SIG_IGN);
        }

        string getip();
        string getmac();
        string getcpuid();
        bool fexists(string name);
        bool fcopy(string dst, string src, bool ovwr);
        string fread(const string &name);
        void fwrite(const string &name, const string &data);

        using Mutex = std::mutex;

        void atexit(std::function<void(void)>);

        string exec(const char* cmd);
        void mkdirp(const string path, bool isfile=false);
        void reboot();
    }

    namespace net {
        sockaddr_in resolve_host(const char* hostname);
    }

#define GET_MILLS()         lang::os::mills()
#define GET_MICROS()        lang::os::micros()
#define GUARD_BEGIN(_m)     { std::lock_guard<std::mutex> _guard(_m)
#define GUARD_END()         }
}



#endif //AICAST_BACKHAUL_LANG_H
