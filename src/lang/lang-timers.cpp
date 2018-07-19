//
// Created by Thinkpad on 2017/11/14.
//
#include <lang/lang-timer.h>

namespace lang {
    namespace timers {

        void Driver::run() {
            for (;;) {
                _wait();
                _fire();
            }
        }
        void Driver::_wait() {
            const auto exp = std::chrono::microseconds(100);;
            auto pred = [this](){
                return !_timers.empty() &&
                       (int)(_timers.front()._at - os::mills()) <= eps;
            };
            std::unique_lock<std::mutex> lk(_m);
            while (!_cv.wait_for(lk, exp, pred)) {}
        }

        void Driver::_fire() {
            list<Callback> fires;
            GUARD_BEGIN(_m);
            uint32_t now = os::mills();
            while (!_timers.empty()) {
                auto& f = _timers.front();
                if ((int)(f._at - now) > eps) {
                    break;
                }
                fires.push_back(f._fn);
                _timers.pop_front();
            }
            GUARD_END();

            for (auto c : fires) {
                int t = c();
                if (t>0) {
                    add(Timer(os::mills()+t, c));
                }
            }
        }

        void Driver::add(Timer t) {
            GUARD_BEGIN(_m);
            if (_timers.empty()) {
                _timers.push_back(t);
            } else {
                Timers::iterator it;
                uint32_t last = t._at;
                for (it = _timers.begin(); it != _timers.end(); ++it) {
                    int delta = it->_at - last;
                    if (delta >= 0) {
                        break;
                    }
                }
                _timers.insert(it, t);
            }
            GUARD_END();
            _cv.notify_one();
        }

    }
}