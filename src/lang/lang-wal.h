//
// Created by Thinkpad on 2017/10/18.
//

#ifndef AICAST_BACKHAUL_LANG_WAL_H
#define AICAST_BACKHAUL_LANG_WAL_H

#include <lang/lang-os.h>
#include "lang-timer.h"

namespace lang {
    namespace wal {

#define ONEDAY_SECONDS      (24*3600)
#define FLUSHDELAY_MILLS    (10*1000)
#define MAXRECENT            100

        class Daily {
            const string    _dir;
            const string    _name;
            const string    _header;
            std::mutex      _m;
            std::ofstream * _current = nullptr;
            string          _curpath;
            list<string>    _recent;
            time_t          _nextroll = 0;
        public:
            Daily(string dir, string name, string header): _dir(dir),_name(name),_header(header) {
                lang::os::atexit([this](){ _close(); });
                lang::timers::after(FLUSHDELAY_MILLS, [this]() -> int {
                    std::lock_guard<std::mutex> guard(_m);
                    if (_current) {
                        _current->flush();
                        if (_current->fail() || !os::fexists(_curpath)) {
                            WARNF("WAL truncated");
                            _close();
                        }
                    }
                    return FLUSHDELAY_MILLS;
                });
            }

            const string &current() const {
                return _curpath;
            }

            void truncate() {
                if (!_curpath.empty()) {
                    _close();
                    ::remove(_curpath.c_str());
                }
            }

            const list<string>& recent() const {
                return _recent;
            }

            friend Daily& operator<<(Daily& os, const std::string& j) {
                os._append(j);
                return os;
            }
            friend Daily& operator<<(Daily& os, const char *& j) {
                os._append(j);
                return os;
            }
        protected:

            void _close() {
                if (_current) {
                    delete _current;
                    _current = nullptr;
                }
            }
            template<class T> void _append(const T &val) {
                char timestamp[60];
                time_t now = time(nullptr);
                tm ti = *localtime(&now);
                std::lock_guard<std::mutex> guard(_m);
                if (!_current || (long)(_nextroll - now) <= 0) {
                    _close();
                    ::strftime(timestamp, sizeof(timestamp), "%Y%m%d-%H", &ti);
                    _curpath = _dir + "/" + _name + timestamp;
                    bool freshly = !os::fexists(_curpath);
                    _current = new std::ofstream;
                    _current->open(_curpath, std::ios_base::app);
                    _nextroll = now + (ti.tm_hour*3600+ti.tm_min*60+ti.tm_sec) + ONEDAY_SECONDS;
                    if (freshly) {
                        *_current << _header << "\n";
                    }
                }
                ::strftime(timestamp, sizeof(timestamp), "%FT%TZ",&ti);  //timestamp: 2017-10-19T10:53:55.556+00:00
                std::stringstream  line;
                line << timestamp << "," << val;
                string lstr = line.str();
                *_current << lstr << "\n";
                _recent.push_front(lstr);
                if (_recent.size() > MAXRECENT) {
                    _recent.pop_back();
                }
            }
            //https://stackoverflow.com/a/43690698
        };
    }
}

#endif //AICAST_BACKHAUL_LANG_WAL_H
