//
// Created by Thinkpad on 2017/9/23.
//

#include <fstream>
#include <cstdio>
#include <list>
#include <ifaddrs.h>
#include <lang/errors.h>
#include <lang/lang-os.h>
#include <lang/lang-strings.h>
#include <algorithm>

namespace lang {
    namespace errors {
        string _error_strings[] = {
#define XX(num, name, str) #str,
                _ERR_MAP(XX)
#undef XX
        };
    }

    namespace strings {
        string sprintf(const char * fmt, ...) {
            int len;
            va_list arg, copy;
            va_start(arg, fmt);
            va_copy(copy, arg);
            len = vsnprintf(NULL, 0, fmt, copy);
            va_end(copy);
            string res;
            res.resize(len+1);
            vsnprintf(&res[0], len+1, fmt, arg);
            res.resize(len);
            va_end(arg);
            return res;
        }
    }

    namespace os {

        using namespace std;
        using namespace lang;

        os::Mutex  _m;
        os::Mutex  _exclusive;
        list<function<void(void)>> _exitfns;

        void exclusively(function<void(void)> fn) {
            GUARD_BEGIN(_exclusive);
                fn();
            GUARD_END();
        }
        void _doexit() {
            for (auto &fn : _exitfns) {
                fn();
            }
        }
        void atexit(function<void(void)> fn) {
            GUARD_BEGIN(_m);
            if (_exitfns.empty()) {
                ::atexit(_doexit);
            }
            _exitfns.push_back(fn);
            GUARD_END();
        }
        void mkdirp(const string path, bool isfile) {
            string cmd = "mkdir -p \"" + path + "\"";
            if (isfile) {
                auto pos = cmd.find_last_of("/");
                if ((int)pos >= (int)cmd.length() || pos == string::npos) {
                    return;
                }
                cmd = cmd.substr(pos);
            }
            system(cmd.c_str());
        }
        std::string exec(const char* cmd) {
            std::array<char, 128> buffer;
            std::string result;
            std::shared_ptr<FILE> pipe(popen(cmd, "r"), pclose);
            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }
            while (!feof(pipe.get())) {
                if (fgets(buffer.data(), 128, pipe.get())) {
                    result += buffer.data();
                }
            }
            return result;
        }

        bool fexists(string name) {
            FILE *fsrc = fopen(name.c_str(), "rb+");
            if (!fsrc) {
                return false;
            }
            fclose(fsrc);
            return true;
        }

        string fread(const string &name) {
            std::ifstream in(name);
            stringstream sstr;
            sstr << in.rdbuf();
            return sstr.str();
        }
        void fwrite(const string &name, const string &data) {
            std::ofstream dst(name);
            dst << data;
        }
        bool fcopy(string dst, string src, bool ovwr) {
            bool touch = false;
            FILE *fsrc = NULL, *fbak = NULL;
            do {
                if (fexists(dst) && !ovwr) {
                    break;
                }
                if (!(fsrc = fopen(src.c_str(), "rb"))) {
                    break;
                }
                if (!(fbak = fopen(dst.c_str(), "wb"))) {
                    break;
                }
                char buf[1024];
                while (1) {
                    size_t n = fread(buf, 1, sizeof(buf), fsrc);
                    if (n <= 0 || n != fwrite(buf, 1, n, fbak)) {
                        break;
                    }
                }
                touch = true;
            } while(0);
            if (fsrc) {
                fclose(fsrc);
            }
            if (fbak) {
                fclose(fbak);
            }
            return touch;
        }

        string getmac() {

#ifndef CONFIG_IFACE
#define CONFIG_IFACE  eth0
#endif
#define _xstr(a) _str(a)
#define _str(a) #a
            string ifaceName = _xstr(CONFIG_IFACE);
#undef _str
#undef _xstr

            /*
             * eth0      Link encap:Ethernet  HWaddr 9C:65:F9:21:F2:B4
             */
            std::stringstream ss2(lang::os::exec("ifconfig"));
            string line;
            string reEthHWaddr("^" + ifaceName + "\\s+Link\\s+encap:\\s*Ethernet\\s+HWaddr\\s+(.*)\\s*$");
            while (std::getline(ss2, line)) {
                if (strings::re_match(line, reEthHWaddr)) {
                    return strings::normaliz(strings::re_search(line, reEthHWaddr));
                }
            }
            return "";
        }

        string getcpuid() {
            //Serial          : 00000000f32b1b5b
            string line;
            string reCpuid("^Serial\\s+:\\s*(\\w*)\\s*$");
            string cpuid;
            std::ifstream infile("/proc/cpuinfo");
            while (std::getline(infile, line)) {
                if (strings::re_match(line, reCpuid)) {
                    cpuid = strings::re_search(line, reCpuid);
                }
            }
            return cpuid;
        }
        string getip() {
            struct ifaddrs *addrs;
            struct ifaddrs *itor;
            getifaddrs(&addrs);

            string result;

            for (itor = addrs; itor; itor = itor->ifa_next) {
                if (itor->ifa_addr && itor->ifa_addr->sa_family == AF_INET) {
                    struct sockaddr_in *pAddr = (struct sockaddr_in *)itor->ifa_addr;
                    string ip = inet_ntoa(pAddr->sin_addr);
                    if (ip.find("127.") != 0) {
                        if (result.size()) {
                            result += ",";
                        }
                        result += ip;
                        if (result.find(",") > 0) {
                            break;
                        }
                    }
                }
            }
            freeifaddrs(addrs);
            return result;
        }
    }
}