//
// Created by Thinkpad on 2017/11/23.
//

#include <lang/lang-watchdog.h>
#include <thread>
#include "lang-os.h"
#include "lang-log.h"

namespace lang {
    namespace watch {

        Dog::Dog() {
            _th = std::thread(&Dog::_daemon, this);
        }
        void Dog::after(int seconds) {
            _timeout = ::time(nullptr) + seconds;
        }

        void Dog::stop() {
            _timeout = 0;
        }

        void Dog::_daemon() {
            int count = 0;
            for (;;) {
                if (_timeout && (int)(::time(nullptr) - _timeout) > 0) {
                    count++;
                } else {
                    count = 0;
                }
                if (count >= 3) {
                    WARNF("watchdog timeout!");
                    exit(-1);
                }
                os::sleep_ms(500);
            }
        }
    }
}