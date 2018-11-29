//
// Created by Thinkpad on 2017/11/3.
//

#include <conf/config.h>
#include "conf-etc.h"

namespace conf {
    //refer: https://www.ibm.com/developerworks/aix/library/au-unix-getopt.html
    void Config::init(int argc, char **argv) {

        string server, loglevel, web;
        list<string> mods;
        while (1) {
            static struct option long_options[] = {
                    {"log",    optional_argument, 0, 'l'},
                    {"ns",     optional_argument, 0, 'n'},
                    {"conf",   optional_argument, 0, 'c'},
                    {"mod",    optional_argument, 0, 'm'},
                    {"web",    optional_argument, 0, 'w'},
                    {"passwd", optional_argument, 0, 'p'},
                    {"help",   no_argument, NULL,    'h'},
            };

            int c = getopt_long(argc, argv, "", long_options, NULL);
            if (c == -1)
                break;

            switch (c) {
                case 'l':
                    loglevel = string(optarg ? optarg : argv[optind]);
                    break;
                case 'n':
                    server = string(optarg ? optarg : argv[optind]);
                    break;
                case 'm':
                    mods.push_back(string(optarg ? optarg : argv[optind]));
                    break;
                case 'w':
                    web = string(optarg ? optarg : argv[optind]);
                    break;
                case 'p':
                    _local.system.password = string(optarg ? optarg : argv[optind]);
                    break;
                case 'c':
                    _confhome = string(optarg ? optarg : argv[optind]);
                    if (*_confhome.end() != '/') {
                        _confhome += "/";
                    }
                    break;
                case 'h':
                case '?':
                    fprintf(stderr, "Usage: %s\n"
                                    "\t[--log=LEVEL]         set log level to LEVEL (debug|info|warn|error|fatal)\n"
                                    "\t[--ns=SERVER]         using SERVER as lorawan network service\n"
                                    "\t[--conf=DIRECTORY]    loading conf from DIRECTORY\n"
                                    "\t[--help]              print usage\n",
                            argv[0]);
                    exit(-1);
                default:
                    break;
            }
        }

        _load();

        if (server.size()) {
            _local.networkservice.external.url = server;
            _local.networkservice.runtype = "external";
        }
        if (loglevel.size()) {
            _local.system.loglevel = loglevel;
        }
        if (web.size()) {
            _local.system.weblisten = atoi(web.c_str());
        }
        if (!mods.empty()) {
            _local.system.mod1 = mods.front();
            mods.pop_front();
        }
        if (!mods.empty()) {
            _local.system.mod2 = mods.front();
            mods.pop_front();
        }
    }

    void Config::_load() {
        try {
            _raw_local = Json::parse(jsons::normalize(_confhome + _local_conf));
            _local.load_from(jsons::optional(_raw_local, "gateway_conf"));
        } catch (...) {
            std::exception_ptr p = std::current_exception();
            WARNF("cannot load local config! {}", p.__cxa_exception_type()->name());
        }
    }

    void Config::applyChanges(bool reload) {
        INFOF("Apply radio update!");
        if (reload) {
            _load();
        }
        for (auto &l : _listeners) {
            l();
        }
    }
    void Config::reset() {
        changePasswd("");
    }
    void Config::changePasswd(const string passwd) {
        _local.system.password = passwd;
        save_local();
    }
    ErrStatus Config::save_local() {
        Json updated{{"gateway_conf", _local.get()}};
        lang::jsons::merge(updated, _raw_local);
        _raw_local = updated;
        string text = jsons::serialize(_raw_local, true);
        //etc::wifi_setup(_local.system.wifi);   //TODO wifi setup need further investigation
        if (!_local.system.ulnwk.empty()) {
            etc::ulnwk_change(_local.system.ulnwk);
        }
        io::file((_confhome + _local_conf).c_str()).write(text);
        return ERR_OK;
    }

    void Config::writeVersion(string fw, string hw,string md) {
#define FW "fw"
#define HW "hw"
#define MDL "mdl"
        lang::os::fwrite(string("/tmp/.loraflow_info"),
                         strings::sprintf("{" "\n\"" FW "\":\"%s\",\n " "\"" HW "\":\"%s\",\n " "\"" MDL "\":\"%s\"\n" "}\n",fw.data(),hw.data(),md.data()));
    }

    void Config::writeConnection(bool con) {
#define TITLE "connection"
#define CONN "online"
#define DISCON "offline"

        lang::os::fwrite(string("/tmp/.loraflow_state"),
                         strings::sprintf("{" "\n\"" TITLE "\":\"%s\"\n " "}\n", con ? CONN : DISCON));
    }
}