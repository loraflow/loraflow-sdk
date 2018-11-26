//
// Created by Thinkpad on 2017/9/23.
//

#ifndef AICAST_BACKHAUL_LANG_CONFIG_H
#define AICAST_BACKHAUL_LANG_CONFIG_H

#include <string>
#include <lang/lang-json.h>
#include <lang/lang-file.h>
#include <lang/errors.h>
#include <lang/lang-log.h>
#include <getopt.h>
#include <list>
#include <ipc/lora-ipc.h>
#include <conf/conf-local.h>

namespace conf {

    using namespace std;
    using namespace lang;
    using namespace lang::errors;
    using lang::Json;

    using Listener = std::function<void(void)>;
    class Config {
        string _confhome = "conf/";
        const string _local_conf = "local_conf.json";
        list<Listener>  _listeners;
    public:

        //refer: https://www.ibm.com/developerworks/aix/library/au-unix-getopt.html
        void init(int argc, char **argv);

        void listen(Listener listener) { _listeners.push_back(listener); }

        const Local & c_local() const { return _local; }
        Local & local() { return _local; }

        Json load_resource(string name) { return Json::parse(jsons::normalize(_confhome + name)); }

        void _load();
        ErrStatus save_local();
        void applyChanges(bool reload=false);
        void reset();
        void changePasswd(const string passwd);
        void writeVersion(string fw, string hw,string md);

    protected:

        Local       _local = {};
        Json        _raw_local;

        bool validate_server(string &s) {
            return s.size() > 0;
        }
        bool validate_ssid(string &s) {
            return s.size() > 0;
        }
        bool validate_passwd(string &s) {
            return true;
        }
    };

    extern Config INSTANCE;
}

#endif //AICAST_BACKHAUL_LANG_CONFIG_H
