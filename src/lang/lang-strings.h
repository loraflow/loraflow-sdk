//
// Created by Thinkpad on 2017/9/19.
//

#ifndef AICAST_BACKHAUL_LANG_STRINGS_H
#define AICAST_BACKHAUL_LANG_STRINGS_H

#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <queue>
#include <regex>
#include <lang/lang-os.h>
#include <algorithm>

namespace lang {
    namespace strings {
        using std::string;

        /**
         * Find string 'str' in data buffer 'buf' of length 'len'.
         *
         * @return size_t offset or len if not found.
         */
        inline size_t offset(const char* buf, size_t len, const string str)
        {
            return std::search(buf, buf + len, str.c_str(), str.c_str() + str.length()) - buf;
        }

        //inline string re_search(string &line, std::regex &ex, int group=1) {
        //    std::smatch m;
        //    if (std::regex_search(line, m, ex)) {
        //        return m[group];
        //    }
        //    return "";
        //}
        inline string re_search(const string &line, string ex, int group=1) {
            std::smatch m;
            std::regex re(ex);
            if (std::regex_search(line, m, re)) {
                return m[group];
            }
            return "";
        }

        inline bool re_match(string &line, string ex) {
            std::smatch m;
            std::regex re(ex);
            return std::regex_search(line, m, re);
        }
        inline bool re_match(string &line, std::regex &ex) {
            std::smatch m;
            return std::regex_search(line, m, ex);
        }
        inline string normaliz(string &&s) {
            s.erase(std::remove_if(s.begin(), s.end(), [](char c) { return !isalnum(c); }), s.end());
            std::transform(s.begin(), s.end(), s.begin(), ::tolower);
            return s;
        }

        inline string ltrim(const string &s, const string &sub) {
            if (s.compare(0, sub.size(), sub)) {
                return s;
            } else {
                return s.substr(sub.size());
            }
        }
        inline bool endwith(const string &s, const string &suffix) {
            const int n = s.size() - suffix.size();
            return n >= 0 && 0 == s.compare(n, s.size(), suffix);
        }

        inline int str2hex(const string &s, uint8_t *bin, int size, bool invert=false) {
#define BETW(_c, _a, _b)  ((_c)>= (_a) && (_c)<= (_b))
#define HEX1(_c)  (BETW(_c, '0', '9') ? ((_c) - '0') : 10+(BETW(_c, 'A','F') ? (_c)-'A' : (_c)-'a'))
            if ((int)s.size() > size * 2) {
                return 0;
            }
            int L = MIN((int)s.size()/2, size);
            for (int i=0; i<L; i++) {
                uint8_t h = HEX1((char)(s[2*i]));
                uint8_t l = HEX1((char)(s[2*i+1]));
                if (h > 15 || l > 15) {
                    return 0;
                }
                bin[invert ? L - i - 1 : i] = (h << 4) + l;
            }
            return L;
        }

        inline string hex2str(const uint8_t *bin, int size, bool invert=false) {
            string str;
            str.resize(size*2);
            for (int i=0; i<size; i++) {
                uint8_t h = bin[i] >> 4;
                uint8_t l = bin[i] & 0xf;
                int base = 2 * (invert ? size-i-1 : i);
                str[base+0] = (char)((h < 10 ? '0' : 'a'-10) + h);
                str[base+1] = (char)((l < 10 ? '0' : 'a'-10) + l);
            }
            return str;
        }

        inline std::string split(string s, string by, string &right) {
            int pos = s.find(by);
            string left;
            if (pos >= 0) {
                left = s.substr(0, pos);
                right = s.substr(pos+by.size());
            } else {
                left = s;
                right = "";
            }
            return left;
        }

        template <typename T>
        std::queue<T> split(string optarg, bool(*fn)(string&, T&), string del=",") {
            std::queue<T> results;
            for (string rem=""; optarg.size() > 0; optarg = rem) {
                string sub = strings::split(optarg, del, rem);
                T u;
                if (fn(sub, u)) {
                    results.push(u);
                }
                optarg = rem;
            }
            return results;
        }

        string sprintf(const char * fmt, ...);
    }
}

#endif //AICAST_BACKHAUL_LANG_STRINGS_H
