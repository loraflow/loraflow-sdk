//
// Created by Thinkpad on 2017/11/9.
//

#ifndef AICAST_BACKHAUL_CONF_ETC_H
#define AICAST_BACKHAUL_CONF_ETC_H

#include <list>
#include <regex>
#include <dirent.h>
#include <lang/lang-strings.h>

namespace conf {

    using std::string;
    using namespace lang;
    
    struct etc {
        enum Section {Unknown, Device, IfaceAP, IfaceSta, EtherIface};
        enum Type {Skip, Mode, APSSID, StaSSID, StaPasswd, Encryption, EtherProto, EtherIpaddr, EtherNetmask, EtherIfname, EtherGateway, EtherDns};
        enum GPIOMode {Input, Output};
        struct WifiAP {
            string      name;
            string      encryption;
            string      quality;
        public:
            bool valid() {
                return !name.empty() && !quality.empty() && !encryption.empty();
            }
        };
        struct WifiConfig {
            string   mode;      //ap or sta
            string   encryption;  //psk2, ...
            string   apname;    //ap's name
            string   stassid;   //sta's ssid
            string   stapasswd; //sta's passwd

            bool is_station() const {
                return mode == "sta";
            }
            bool is_ap() const {
                return mode == "ap";
            }
        };

        struct EtherConfig {
            string proto;  //static or dhcp
            string ipaddr;
            string netmask;
            string gateway;
            string dns;

            bool is_static() const {
                return proto == "static";
            }
            bool is_dhcp() const {
                return proto == "dhcp";
            }
        };

        class GPIO {
            int pin;
        public:
            enum class Mode {Input, Output};
            GPIO(int pin):pin(pin) {

            }
            void enable(Mode mode);
            int read();
            void write(int val);
        };

        static WifiConfig wifi_query();
        static void wifi_setup(const WifiConfig &wifi);
        static void wifi_reset();
        static vector<WifiAP> wifi_scan();
        static std::list<string> ulnwk_list();
        static void ulnwk_change(string netw);

        static EtherConfig ether_query();
        static void ether_setup(const EtherConfig &cfg);
        static void ether_reset();

    private:
        static void _wifi_iterate(std::istream &infile, std::function<void(Type, string &, string)> fn);
        static void _ether_iterate(std::istream &infile, std::function<void(Type, string &, string&, string)> fn);
    };
}

#endif //AICAST_BACKHAUL_CONF_ETC_H
