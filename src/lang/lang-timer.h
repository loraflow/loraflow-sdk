//
// Created by Thinkpad on 2017/9/21.
//

#ifndef AICAST_BACKHAUL_LANG_TIMER_H
#define AICAST_BACKHAUL_LANG_TIMER_H

#include <cstdint>
#include <lang/lang-os.h>
#include <list>
#include <sys/param.h>
#include <chrono>
#include <condition_variable>

namespace lang {

    namespace timers {
        using Callback = std::function<int()>;

        class Timer {
            friend class Driver;
            const uint32_t _at;
            const Callback _fn;
            Timer(uint32_t at, Callback fn):_at(at),_fn(fn) {
            }
            friend void after(uint32_t mills, Callback fn);
            friend void at(uint32_t mills, Callback fn);
        };
        class Driver {
            using Timers = std::list<Timer>;
            static const int eps = 1;
            Timers      _timers;
            os::Mutex   _m;
            condition_variable _cv;
            std::thread _th;
            void run();
            void _wait();
            void _fire();
        public:
            static Driver driver;
            Driver() {
                _th = std::thread(&Driver::run, this);
            }
            void add(Timer t);
        };

        inline void after(uint32_t mills, Callback fn) {
            Driver::driver.add(Timer(os::mills()+mills, fn));
        }
    }
}
#endif //AICAST_BACKHAUL_LANG_TIMER_H
