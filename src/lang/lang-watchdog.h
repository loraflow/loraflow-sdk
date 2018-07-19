//
// Created by Thinkpad on 2017/11/23.
//

#ifndef AICAST_BACKHAUL_LANG_WATCHDOG_H
#define AICAST_BACKHAUL_LANG_WATCHDOG_H

#include <thread>

namespace lang {
    namespace watch {

        class Dog {
        protected:
            std::thread _th;
            volatile long _timeout = 0;
        public:
            Dog();
            void after(int seconds);
            void stop();

        protected:
            void _daemon();
        };
    }
}
#endif //AICAST_BACKHAUL_LANG_WATCHDOG_H
