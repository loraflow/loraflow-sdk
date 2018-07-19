//
// Created by Thinkpad on 2017/9/20.
//

#ifndef AICAST_BACKHAUL_LANG_JSON_H
#define AICAST_BACKHAUL_LANG_JSON_H

#include <lang/constants.h>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <stdlib.h>
#include <inttypes.h>
#include <parson.h>

#ifdef CONFIG_FIX_CPPSTD
//gcc 4.9.4 for openwrt is missing some std utilities
#if defined(__GNUC__)
#if ((__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__) <= 40940)
namespace std {
    template <typename T>
    string to_string(T val)
    {
        stringstream stream;
        stream << val;
        return stream.str();
    }

    inline int stoi(const string& __str, size_t* __idx = 0, int __base = 10) {
        return 0;
    }

    inline unsigned long long strtoull(const char * s, char ** p, int b) {
        return ::strtoull(s, p, b);
    }

    inline float strtof(const char*s, char ** p) {
        return ::strtof(s, p);
    }

    inline double strtold (const char *s, char **p) {
        return ::strtold(s, p);
    }

    inline long long strtoll(const char *s, char **p, int base) {
        return ::strtoll(s, p, base);
    }
}
#endif
#endif
#endif

#include <json.hpp>
#include <string>
#include <list>

namespace lang {

    using Json = nlohmann::json;

    namespace jsons {

        struct BadJsonException : public std::exception {
            const char *what() const noexcept override {
                return "BadJsonException";
            }
        };

        using std::string;

        Json optional(const Json &opt, string key);
        void foreach(const Json &opt, string key, std::function<void(Json&)> fn);
        string normalize(string file);
        void merge(Json &dst, const Json&src);

        inline string serialize(Json &j, bool pretty=false) {
            return pretty ? j.dump(4) : j.dump();
        }

        template <class T> bool optional(const Json &opt, string key, T &val) {
            auto it = opt.find(key);
            if (it != opt.end()) {
                val = it.value();
                return true;
            } else {
                return false;
            }
        }

        template <class T> T getdefault(const Json &opt, string key, T defval) {
            auto it = opt.find(key);
            if (it != opt.end()) {
                return it.value();
            } else {
                return defval;
            }
        }

        template <class T> bool getlist(const Json &opt, string key, std::list<T> &ll) {
            auto it = opt.find(key);
            if (it != opt.end()) {
                Json j = it.value();
                if (j.is_array()) {
                    for (Json::iterator it = j.begin(); it != j.end(); ++it) {
                        ll.push_back(*it);
                    }
                    return true;
                }
            }
            return false;
        }
    }
}

#endif //AICAST_BACKHAUL_LANG_JSON_H
