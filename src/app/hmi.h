//
// Created by Thinkpad on 2017/11/14.
//

#ifndef AICAST_BACKHAUL_HMI_H
#define AICAST_BACKHAUL_HMI_H

#include <vector>
#include <conf/conf-etc.h>
#include <lang/lang-timer.h>
#include <lang/lang-log.h>

namespace haul {
    namespace hmi {
        using namespace std;
        using namespace conf;

        enum Event : int {
            EVT_LOAD,
            EVT_CONNECTING,
            EVT_CONNECTED,
            EVT_DISCCONNECTED,
        };
        class Button {
            etc::GPIO  _btn;
        public:
            Button(int pin):_btn(pin) {
                _btn.enable(etc::GPIO::Mode::Input);
            }
            int read() {
                return _btn.read();
            }
        };

        class Flash {
            etc::GPIO  _pin;
            vector<int>  _duty;
            bool       _on = false;
            bool       _repeat = false;
            int        _idx = 0;
        public:
            Flash(int pin):_pin(pin) {
                _pin.enable(etc::GPIO::Mode::Output);
            }

            void flash(vector<int> duty, bool repeat=false) {
                reset();
                _duty = duty;
                _repeat = repeat;
                _toggle();
            }
            void flash(int dura) {
                flash(vector<int>{dura});
            }
            void reset() {
                _pin.write(0);
                _idx = 0;
                _on = false;
                _repeat = false;
                _duty.clear();
            }
            void turn(bool on) {
                _on = on;
                _pin.write(on);
            }

        protected:
            void _toggle() {
                if (_idx >= (int)_duty.size() && _repeat) {
                    _on = false;
                    _idx = 0;
                }
                if (_idx >= (int)_duty.size()) {
                    _pin.write(0);
                } else {
                    _on = !_on;
                    _pin.write(_on ? 1 : 0);
                    lang::timers::after(_duty[_idx++], [this]() -> int{
                        _toggle();
                        return 0;
                    });
                }
            }
        };

        //Human Machine Interface
        class HMI {
            Button topButton;
            Flash  beep;
            Flash  ledRed;
            Flash  ledGreen;
            Flash  ledBlue;
            std::thread _thd;
            Json   info;
            os::Mutex _m;
            unsigned _sinceMills = 0;
            unsigned _finishVerify = 0;
            string   _privkey;
            string   _pubkey;
            bool     _runsigner = false;
        public:
            HMI();
            void start() {
                _thd = std::thread(&HMI::button_daemon, this);
            }

            void dispatch(Event e);
            void chkuuid(const char*uuid);
            void signer();
            void V();
            bool isSignerMode() {
                return _runsigner;
            }

        protected:
            void button_daemon();
        };

        HMI &_hmi();
    }
}
#endif //AICAST_BACKHAUL_HMI_H
