//
// Created by Thinkpad on 2017/11/9.
//

#include <conf/conf-etc.h>
#include <fstream>

#define ETC_WIFI_FILE   "/etc/config/wireless"
#define ETC_NETWORK_FILE   "/etc/config/network"
#define ETC_3G_DIR  "/etc/chatscripts"
#define ETC_3G_CURSOR  "3g.chat"
#define ETC_WIFI_SCAN  "iwinfo ra0 scan"
#define ETC_IFACE_ETH  "ETH"

namespace conf {

    using WifiConfig = etc::WifiConfig;
    using WifiAP = etc::WifiAP;
    using EtherConfig = etc::EtherConfig;

    EtherConfig etc::ether_query() {
        EtherConfig cfg;
        std::ifstream infile(ETC_NETWORK_FILE);
        _ether_iterate(infile, [&cfg](Type typ, string &line, string &iface, string word) {
            if (iface == ETC_IFACE_ETH) {
                switch (typ) {
                    case EtherProto:
                        cfg.proto = word;
                        break;
                    case EtherIpaddr:
                        cfg.ipaddr = word;
                        break;
                    case EtherNetmask:
                        cfg.netmask = word;
                        break;
                    case EtherGateway:
                        cfg.gateway = word;
                        break;
                    case EtherDns:
                        cfg.dns = word;
                        break;
                    default:
                        break;
                }
            }
        });
        return cfg;
    }

    void etc::ether_setup(const EtherConfig &cfg) {
        std::stringstream output;
        std::ifstream infile(ETC_NETWORK_FILE);
        bool protochange = false;
        _ether_iterate(infile, [&cfg, &output, &protochange](Type typ, string &line, string &iface, string word) {
            bool touch = false;
            if (iface == ETC_IFACE_ETH) {
                switch (typ) {
                    case EtherProto:
                        protochange = cfg.proto != word;
                        if (protochange || cfg.is_static()) {
                            output << "        option proto '" << cfg.proto << "'" << std::endl;
                            if (cfg.is_static()) {
                                output << "        option ipaddr '" << cfg.ipaddr << "'" << std::endl;
                                output << "        option netmask '" << cfg.netmask << "'" << std::endl;
                                output << "        option gateway '" << cfg.gateway << "'" << std::endl;
                                output << "        option dns '" << cfg.dns << "'" << std::endl;
                            }
                            touch = true;
                        }
                        break;
                    case EtherIpaddr:
                    case EtherNetmask:
                    case EtherGateway:
                    case EtherDns:
                        touch = true;
                        break;
                    default:
                        break;
                }
            }
            if (!touch) {
                output << line << std::endl;
            }
        });
        if (protochange || cfg.is_static()) {
            std::ofstream dst(ETC_NETWORK_FILE);
            dst << output.str();
        }
    }

    WifiConfig etc::wifi_query() {
        WifiConfig wifi;
        std::ifstream infile(ETC_WIFI_FILE);
        _wifi_iterate(infile, [&wifi](Type typ, string &line, string word) {
            switch (typ) {
                case Mode:
                    wifi.mode = word;
                    break;
                case APSSID:
                    wifi.apname = word;
                    break;
                case StaSSID:
                    wifi.stassid = word;
                    break;
                case StaPasswd:
                    wifi.stapasswd = word;
                    break;
                case Encryption:
                    wifi.encryption = word;
                    break;
                default:
                    break;
            }
        });
        return wifi;
    }

    void etc::wifi_reset() {
        auto wifi = wifi_query();
        wifi.mode = "ap";
        wifi_setup(wifi);
    }

    void etc::ether_reset() {
        EtherConfig cfg;
        cfg.proto = "static";
        cfg.ipaddr = "192.168.99.1";
        cfg.netmask = "255.255.255.0";
        cfg.gateway = "192.168.99.1";
        cfg.dns = "114.114.114.114";
        ether_setup(cfg);
    }

    static string translate_encryption(string raw) {
        //none for an open network, wep for WEP, psk for WPA-PSK, or psk2 for WPA2-PSK
        if (raw == "none") {
            return "none";
        }
        if (raw == "WPA PSK") {
            return "psk";
        }
        if (raw == "WPA2 PSK") {
            return "psk2";
        }
        if (raw == "WEP") {
            return "wep";
        }
        return "";
    }
    /*
     * REFER  https://wiki.openwrt.org/doc/uci/wireless#wpa_modes
     * */
    vector<WifiAP> etc::wifi_scan() {
        /*
         * Cell 01 - Address: 8C:C7:D0:0B:CF:18
         *          ESSID: " "
         *          Mode: Master  Channel: 1
         *          Signal: -256 dBm  Quality: 37/100
         *          Encryption: none
         * Cell 04 - Address: E0:10:7F:76:AA:28
         *          ESSID: "cogobuy-guest"
         *          Mode: Master  Channel: 1
         *          Signal: -256 dBm  Quality: 10/100
         *          Encryption: none
         * Cell 14 - Address: DC:FE:18:89:E6:11
         *          ESSID: "Tplink-Aisenz"
         *          Mode: Master  Channel: 11
         *          Signal: -256 dBm  Quality: 50/100
         *          Encryption: WPA2 PSK (AES-OCB)
         */
        vector<WifiAP> results;
        string &&text = lang::os::exec(ETC_WIFI_SCAN);
        std::stringstream ss2(text);

        WifiAP ap;
        string line;
        string reCell("^Cell\\s+\\d+\\s+-\\s+Address:.*");
        string reSSID("^\\s+ESSID:\\s+\"\\s*([^\"]*)\"");
        string reSignal("^\\s+Signal:.*?Quality:\\s+(.*?)\\s*$");
        string reEncryption("^\\s+Encryption:\\s+(.*?)(?:\\s+\\([^)]+\\))?\\s*$");
        while (std::getline(ss2, line)) {
            if (line.empty() || strings::re_match(line, reCell)) {
                if (ap.valid()) {
                    results.push_back(ap);
                }
                ap = WifiAP{};
                continue;
            }
            if (strings::re_match(line, reSSID)) {
                ap.name = strings::re_search(line, reSSID);
            } else if (strings::re_match(line, reSignal)) {
                ap.quality = strings::re_search(line, reSignal);
            } else if (strings::re_match(line, reEncryption)) {
                ap.encryption = translate_encryption(strings::re_search(line, reEncryption));
            }
        }
        if (ap.valid()) {
            results.push_back(ap);
        }
        return results;
    }

    void etc::wifi_setup(const WifiConfig &wifi) {
        std::stringstream output;
        std::ifstream infile(ETC_WIFI_FILE);
        bool modified = false;
        _wifi_iterate(infile, [&wifi, &output, &modified](Type typ, string &line, string word) {
            bool touch = false;
            switch (typ) {
                case Mode:
                    if (wifi.mode != word) {
                        output << "        option linkit_mode '" << wifi.mode << "'" << std::endl;
                        touch = true;
                    }
                    break;
                case StaSSID:
                    if (wifi.stassid != word) {
                        output << "        option ssid '" << wifi.stassid << "'" << std::endl;
                        touch = true;
                    }
                    break;
                case StaPasswd:
                    if (wifi.stapasswd != word) {
                        output << "        option key '" << wifi.stapasswd << "'" << std::endl;
                        touch = true;
                    }
                    break;
                case Encryption:
                    if (wifi.encryption != word) {
                        output << "        option encryption '" << wifi.encryption << "'" << std::endl;
                        touch = true;
                    }
                    break;
                default:
                    break;
            }
            if (!touch) {
                output << line << std::endl;
            } else {
                modified = true;
            }
        });
        if (modified) {
            std::ofstream dst(ETC_WIFI_FILE);
            dst << output.str();
        }
    }

    void etc::_wifi_iterate(std::istream &infile, std::function<void(Type, string &, string)> fn) {
        //std::ifstream infile(ETC_WIFI_FILE);
        string line;
        string reWifiDevice("^config\\s+wifi-device.*");
        string reIfaceAP("^config\\s+wifi-iface\\s+'ap'");
        string reIfaceSta("^config\\s+wifi-iface\\s+'sta'");
        string reSSID("^\\s+option\\s+ssid\\s+'([^']*)'");
        string rePASSWD("^\\s+option\\s+key\\s+'([^']*)'");
        string reEncryption("^\\s+option\\s+encryption\\s+'([^']*)'");
        string reOption("^\\s+option\\s+");
        string reLinkMode("^\\s+option\\s+linkit_mode\\s+'([^']*)'");
        Section section = Unknown;
        while (std::getline(infile, line)) {
            Type typ = Skip;
            if (strings::re_match(line, reWifiDevice)) {
                section = Device;
            } else if (strings::re_match(line, reIfaceAP)) {
                section = IfaceAP;
            } else if (strings::re_match(line, reIfaceSta)) {
                section = IfaceSta;
            } else {
                switch (section) {
                    case Device:
                        if (strings::re_match(line, reLinkMode)) {
                            fn(typ = Mode, line, strings::re_search(line, reLinkMode));
                        }
                        break;
                    case IfaceAP:
                        if (strings::re_match(line, reSSID)) {
                            fn(typ = APSSID, line, strings::re_search(line, reSSID));
                        }
                        break;
                    case IfaceSta:
                        if (strings::re_match(line, reSSID)) {
                            fn(typ = StaSSID, line, strings::re_search(line, reSSID));
                        } else if (strings::re_match(line, rePASSWD)) {
                            fn(typ = StaPasswd, line, strings::re_search(line, rePASSWD));
                        } else if (strings::re_match(line, reEncryption)) {
                            fn(typ = Encryption, line, strings::re_search(line, reEncryption));
                        }
                    default:break;
                }
            }
            if (typ == Skip) {
                fn(typ, line, "");
            }
        }
    }

    void etc::_ether_iterate(std::istream &infile, std::function<void(Type, string&, string &, string)> fn) {
        //std::ifstream infile(ETC_NETWORK_FILE);
        /*
         * config interface 'ETH'
         *     option proto 'dhcp'
         *     option ifname 'eth0'
         *     option ipaddr '192.168.100.1'
         */
        string line;
        string reIface("^config\\s+interface\\s+'([^']*)'\\s*$");
        string reOptProto("^\\s+option\\s+proto\\s+'([^']*)'");
        string reOptIfname("^\\s+option\\s+ifname\\s+'([^']*)'");
        string reOptAddr("^\\s+option\\s+ipaddr\\s+'([^']*)'");
        string reOptMask("^\\s+option\\s+netmask\\s+'([^']*)'");
        string reOptGateway("^\\s+option\\s+gateway\\s+'([^']*)'");
        string reOptDns("^\\s+option\\s+dns\\s+'([^']*)'");
        string reOptAny("^\\s+option\\s+.*");
        string reBlank("^\\s*$");
        Section section = Unknown;
        string iface;
        while (std::getline(infile, line)) {
            Type typ = Skip;
            if (strings::re_match(line, reIface)) {
                section = EtherIface;
                iface = strings::re_search(line, reIface);
            } else if (!strings::re_match(line, reOptAny)) {
                section = Unknown;
            } else {
                switch (section) {
                    case EtherIface:
                        if (strings::re_match(line, reOptProto)) {
                            fn(typ = EtherProto, line, iface, strings::re_search(line, reOptProto));
                        } else if (strings::re_match(line, reOptIfname)) {
                            fn(typ = EtherIfname, line, iface, strings::re_search(line, reOptIfname));
                        } else if (strings::re_match(line, reOptAddr)) {
                            fn(typ = EtherIpaddr, line, iface, strings::re_search(line, reOptAddr));
                        } else if (strings::re_match(line, reOptMask)) {
                            fn(typ = EtherNetmask, line, iface, strings::re_search(line, reOptMask));
                        } else if (strings::re_match(line, reOptGateway)) {
                            fn(typ = EtherGateway, line, iface, strings::re_search(line, reOptGateway));
                        } else if (strings::re_match(line, reOptDns)) {
                            fn(typ = EtherDns, line, iface, strings::re_search(line, reOptDns));
                        }
                        break;
                    default:break;
                }
            }
            if (typ == Skip) {
                fn(typ, line, iface, "");
            }
        }
    }

    std::list<string> etc::ulnwk_list() {
        std::list<string> results;
        DIR *dir;
        if ((dir = opendir ("/etc/chatscripts")) != NULL) {
            string current(ETC_3G_CURSOR);
            dirent *ent;
            while ((ent = readdir (dir)) != NULL) {
                if (ent->d_type == DT_REG) {
                    string name(ent->d_name);
                    if (strings::re_match(name, "^(.+)\\.chat$") && name != current) {
                        results.push_back(strings::re_search(name, "^(.+)\\.chat$"));
                    }
                }
            }
            closedir (dir);
        }
        return results;
    }

    void etc::ulnwk_change(string netw) {
        std::ifstream  src(string(ETC_3G_DIR) + "/" + netw + ".chat", std::ios::binary);
        std::ofstream  dst(string(ETC_3G_DIR) + "/" + string(ETC_3G_CURSOR),   std::ios::binary);
        dst << src.rdbuf();
    }

    void etc::GPIO::enable(GPIO::Mode mode) {
        //echo "29" > /sys/class/gpio/export
        //echo "out" > /sys/class/gpio/gpio29/direction
        //std::ofstream  dst(string("/sys/class/gpio/export"),   std::ios::binary);
        //dst << strings::sprintf("%d", pin);

        lang::os::fwrite(string("/sys/class/gpio/export"), strings::sprintf("%d", pin));

        //std::ofstream  osdir(strings::sprintf("/sys/class/gpio/gpio%d/direction", pin), std::ios::binary);
        //osdir << ((mode == etc::GPIO::Mode::Output) ? "out" : "in");

        lang::os::fwrite(strings::sprintf("/sys/class/gpio/gpio%d/direction", pin), (mode == etc::GPIO::Mode::Output) ? "out" : "in");
    }

    int etc::GPIO::read() {
        //cat /sys/class/gpio/gpio$1/value
        string fnpinval = strings::sprintf("/sys/class/gpio/gpio%d/value", pin);
        std::ifstream  isval(fnpinval, std::ios::binary);
        int val;
        isval >> val;
        return val;
    }

    void etc::GPIO::write(int val) {
        //echo "1" > /sys/class/gpio/gpio29/value
        string fnpinval = strings::sprintf("/sys/class/gpio/gpio%d/value", pin);
        std::ofstream  osval(fnpinval, std::ios::binary);
        osval << strings::sprintf("%d", val);
    }

}