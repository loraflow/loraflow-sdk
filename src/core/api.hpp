//
// Created by gl on 2018/11/2.
//

#ifndef LORALAN_BACKHAUL_API_HPP
#define LORALAN_BACKHAUL_API_HPP


#include <httplib.h>
#include <lang/lang-json.h>
#include <utility>
#include <conf/config.h>
#include <conf/conf-etc.h>
#include <version.hpp>

#define CTYPE_JSON "application/json"
#define CTYPE_TEXT "text/plain"

#ifndef CONFIG_LOG_FILE
#define CONFIG_LOG_FILE  "/tmp/messages"
#endif
#ifndef CONFIG_UPGRADE_FILE
#define CONFIG_UPGRADE_FILE  "./www/send/loralan_upgrade.zip"
#endif
#ifndef CONFIG_LOG_SIZE
#define CONFIG_LOG_SIZE  512000
#endif
#ifndef CONFIG_WEBROOT
#define CONFIG_WEBROOT  "./www"
#endif
using Timepoint = std::chrono::steady_clock::time_point;
using namespace lang;
struct Cnode{
    int name = 1;
};


static const std::string base64_chars =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


static inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}





class WebAPI {
#define MUST_AUTHORIZED()         do {  \
                if (!_authorized) {    \
                    res.set_content("Forbidden", CTYPE_TEXT); \
                    return;     \
                }               \
            } while(0)

    httplib::Server svr;
    bool _authorized = true;
    const Timepoint _uptime;

public:
    WebAPI():_uptime(lang::os::now()) {}

    bool Running() const {
        return svr.is_running();
    }

    void Bind(const std::map<string, Cnode *> &nodesmap, const string &host, int port) {
        using namespace httplib;

//        svr.Get("/", [&](const Request &req, Response &res) {
//            string html = lang::os::fread(string(CONFIG_WEBROOT) + "/index.html");
//            res.set_content(html, CTYPE_TEXT);
//        });
        /*+ADD Start+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
        svr.Get("/test", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/hytest/test.txt");
            res.set_content(test, CTYPE_TEXT);
        });

        svr.Get("/type=local", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/send/local_conf.json");
            res.set_content(test, CTYPE_JSON);
        });

        svr.Get("/type=global", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/send/global_conf.json");
            res.set_content(test, CTYPE_JSON);
        });
        svr.Get("/type=log", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/send/log.txt");
            res.set_content(test, CTYPE_TEXT);
        });
        svr.Get("/type=network", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/send/network");
            res.set_content(test, CTYPE_TEXT);
        });
        svr.Get("/type=ntp", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/send/NTP.txt");
            res.set_content(test, CTYPE_TEXT);
        });
        svr.Get("/type=state", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/send/state.json");
            res.set_content(test, CTYPE_JSON);
        });
        svr.Get("/type=info", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
            string test = lang::os::fread("./www/send/info.json");
            res.set_content(test, CTYPE_JSON);
        });
        //http://192.168.31.195:8000/type=upgrade/name=upgrade.zip/size=9342
//        svr.Post("/type=upgrade/name=upgrade.zip/size=9342", [&](const Request &req, Response &res) {
        svr.Post("/type=upgrade", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            //decode Base64
            INFOF("start decode Base64 : Matches::({});({})",req.matches[1],req.matches[2]);
            string result = "Bad Request";
            bool valid = false;
            for (auto &f : req.files) {
                auto &file = f.second;
                if (file.length > 0) {
                    INFOF("start decode Base64");
                    lang::os::fwrite(CONFIG_UPGRADE_FILE, base64_decode(req.body.substr(file.offset, file.length)));
                    result = "OK";
                    valid = true;
                    break;
                }
            }
            if (!valid && !req.body.empty()) {
                INFOF("start decode Base64");
                lang::os::fwrite(CONFIG_UPGRADE_FILE, base64_decode(req.body));
                result = "OK";
            }
            INFOF("start decode Base64");
            INFOF("RESULT:{}",result);
            res.set_content(result, CTYPE_TEXT);
        });

        svr.Post("/type=local", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string result = "Bad Request";
            for (auto &f : req.files) {
                auto &file = f.second;
                if (file.length > 0) {
                    lang::os::fwrite("./www/send/local_conf.json", req.body.substr(file.offset, file.length));
                    result = "OK";
                    break;
                }
            }
            res.set_content(result, CTYPE_TEXT);
        });

        svr.Post("/type=global", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string result = "Bad Request";
            for (auto &f : req.files) {
                auto &file = f.second;
                if (file.length > 0) {
                    lang::os::fwrite("./www/send/global_conf.json", req.body.substr(file.offset, file.length));
                    result = "OK";
                    break;
                }
            }
            res.set_content(result, CTYPE_TEXT);
        });

        svr.Post("/type=network", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string result = "Bad Request";
            for (auto &f : req.files) {
                auto &file = f.second;
                if (file.length > 0) {
                    lang::os::fwrite("./www/send/network", req.body.substr(file.offset, file.length));
                    result = "OK";
                    break;
                }
            }
            res.set_content(result, CTYPE_TEXT);
        });

        svr.Post("/type=info", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string result = "Bad Request";
            for (auto &f : req.files) {
                auto &file = f.second;
                if (file.length > 0) {
                    lang::os::fwrite("./www/send/info.json", req.body.substr(file.offset, file.length));
                    result = "OK";
                    break;
                }
            }
            res.set_content(result, CTYPE_TEXT);
        });

        svr.Post("/type=log", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string result = "Bad Request";
            for (auto &f : req.files) {
                auto &file = f.second;
                if (file.length > 0) {
                    lang::os::fwrite("./www/send/log.txt", req.body.substr(file.offset, file.length));
                    result = "OK";
                    break;
                }
            }
            res.set_content(result, CTYPE_TEXT);
        });

        svr.Post("/type=ntp", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string result = "Bad Request";
            for (auto &f : req.files) {
                auto &file = f.second;
                if (file.length > 0) {
                    lang::os::fwrite("./www/send/NTP.txt", req.body.substr(file.offset, file.length));
                    result = "OK";
                    break;
                }
            }
            res.set_content(result, CTYPE_TEXT);
        });

        svr.Post("/type=reset", [&](const Request &req, Response &res) {
//            MUST_AUTHORIZED();
            INFOF("RESET OK");
            res.set_content("OK", CTYPE_TEXT);
//            lang::os::sleep_ms(1000);
//            exit(0);
        });



        svr.Post("/type=changepwd", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            //todo 先验证原密码是否正确
            string userinfo = lang::os::fread("./www/send/login.json");
            Json info = Json::parse(userinfo);
            string username = jsons::optional<string>("username", info, "admin");
            string password = jsons::optional<string>("password", info, "123456");
            string result("OK");
            if (!password.empty() && req.body.size() != 0) {
                Json form = Json::parse(req.body);
                string s1 = jsons::optional<string>("username",form, "");
                string s2 = jsons::optional<string>("old",form, "");
                string s3 = jsons::optional<string>("new",form, "");
                if (s1 != username || s2 != password) {
                    result = "Bad username or password";
                }else {
                    string on_info;
                    on_info.clear();
#define USER "username"
#define PWD  "password"
                    lang::os::fwrite("./www/send/login.json", strings::sprintf("{" "\n\"" USER "\":\"%s\",\n " "\"" PWD "\":\"%s\"\n" "}\n",s1.data(),s3.data()));
                }
            }else {
                result = "EMPTY username or Password ";
            }
//            _authorized = result == "OK";
            res.set_content(result, CTYPE_TEXT);
        });



        /*+ADD End++++++++++++++++++++++++++++++++++++  +++++++++++++++++++++++++++++++*/
        svr.Post("/type=logout", [&](const Request &req, Response &res) {
//            _authorized = false;
            INFOF("LOGIN OUT");
            res.set_content("OK", CTYPE_TEXT);
        });
        svr.Post("/type=login", [&](const Request &req, Response &res) {
            INFOF("LOGIN IN");
            string userinfo = lang::os::fread("./www/send/login.json");
            Json info = Json::parse(userinfo);
            string username = jsons::optional<string>("username", info, "admin");
            string password = jsons::optional<string>("password", info, "123456");
            INFOF("user:{}",username);
            INFOF("pwd:{}",password);
            string result("OK");
            if (!password.empty() && req.body.size() != 0) {
                Json form = Json::parse(req.body);
                string s1 = jsons::optional<string>("username",form, "");
                string s2 = jsons::optional<string>("password",form, "");
                if (s1 != username || s2 != password) {
                    result = "Bad username or password";
                }
            }else {
                result = "EMPTY username or Password ";
            }
//            _authorized = result == "OK";
            res.set_content(result, CTYPE_TEXT);
        });
//        svr.Get("/v1/config", [&](const Request &req, Response &res) {
//            auto cf = conf::INSTANCE.GetUpdate();
//            res.set_content(cf.dump(), CTYPE_JSON);
//        });
//        svr.Post("/v1/config", [&](const Request &req, Response &res) {
//            MUST_AUTHORIZED();
//            Json update = Json::parse(req.body);
//            if (conf::INSTANCE.SetUpdate(update)) {
//                res.set_content("OK", CTYPE_TEXT);
//            } else {
//                res.set_content("Bad Request", CTYPE_TEXT);
//            }
//        });
//        svr.Delete("/v1/logs", [&](const Request &req, Response &res) {
//            MUST_AUTHORIZED();
////            truncate(CONFIG_LOG_FILE, 0);
//            res.set_content("OK", CTYPE_TEXT);
//        });
//        svr.Get("/v1/logs", [&](const Request &req, Response &res) {
//            FILE *fp;
//            if ((fp = fopen(CONFIG_LOG_FILE, "rb")) == nullptr) {
//                res.set_content("---", CTYPE_TEXT);
//            } else {
//                fseek(fp, 0L, SEEK_END);
//                long fileSize = ftell(fp);
//                char buf[CONFIG_LOG_SIZE+1];
//                if (fileSize < (long)sizeof(buf)) {
//                    fseek(fp, 0L, SEEK_SET);
//                } else {
//                    fseek(fp, 1-sizeof(buf), SEEK_END);
//                }
//                long bytesRead = fread(buf, 1, sizeof(buf)-1, fp);
//                buf[MAX(bytesRead, sizeof(buf)-1)] = 0;
//                fclose(fp);
//                res.set_content(buf, CTYPE_TEXT);
//            }
//        });
//
//        svr.Get("/v1/logs/level", [&](const Request &req, Response &res) {
//            res.set_content(Log::get_level(), CTYPE_TEXT);
//        });
        svr.Post(R"(/v1/logs/level/(\w+))", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string level = req.matches[1];
            Log::set_level(level);
            if (Log::get_level() != level) {
                reply_err(res, "bad level");
            } else {
                res.set_content("OK", CTYPE_TEXT);
            }
        });
//        svr.Get("/v1/info", [&](const Request &req, Response &res) {
//            Json info = {
//                    {"gwtype", versions::GatewayType()},
//                    {"fwver", versions::FWVer()},
//                    {"hwver", versions::HWVer()},
//                    {"uptime", lang::os::mills_since(_uptime)/1000},
//            };
//            Json nodes{};
//            for (auto kv : nodesmap) {
////                nodes[kv.first] = kv.second->Status();
//            }
//            info["nodes"] = nodes;
//            res.set_content(info.dump(), CTYPE_JSON);
//        });
//        svr.Get("/v1/radios", [&](const Request &req, Response &res) {
//            std::list<Json> list;
//            for (auto kv : nodesmap) {
////                list.push_back(kv.second->Radio());
//            }
//            res.set_content(Json(list).dump(), CTYPE_JSON);
//        });
//        svr.Post("/v1/reset", [&](const Request &req, Response &res) {
//            MUST_AUTHORIZED();
//            res.set_content("OK", CTYPE_TEXT);
//            lang::os::sleep_ms(1000);
//            exit(0);
//        });
//        svr.Get("/v1/ethernet", [&](const Request &req, Response &res) {
//            res.set_content(doGetEthernet().dump(), CTYPE_JSON);
//        });
//        svr.Post("/v1/ethernet", [&](const Request &req, Response &res) {
//            MUST_AUTHORIZED();
//            doPostEthernet(Json::parse(req.body));
//            res.set_content("OK", CTYPE_TEXT);
//        });
//        svr.Post("/v1/upgrade", [&](const Request &req, Response &res) {
//            MUST_AUTHORIZED();
//            string result = "Bad Request";
//            for (auto &f : req.files) {
//                auto &file = f.second;
//                if (file.length > 0) {
//                    lang::os::fwrite(CONFIG_UPGRADE_FILE, req.body.substr(file.offset, file.length));
//                    result = "OK";
//                    break;
//                }
//            }
//            res.set_content(result, CTYPE_TEXT);
//        });
//        svr.Delete("/v1/upgrade", [&](const Request &req, Response &res) {
//            MUST_AUTHORIZED();
//            string result = "Bad Request";
//            if (lang::os::fexists(CONFIG_UPGRADE_FILE)) {
//                if (0 != remove(CONFIG_UPGRADE_FILE)) {
//                    result = "OK";
//                } else {
//                    result = "Internal server error";
//                }
//            }
//            res.set_content(result, CTYPE_TEXT);
//        });
        svr.set_error_handler([&](const Request & /*req*/, Response &res) {
            const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
            char buf[200];
            snprintf(buf, sizeof(buf), fmt, res.status);
            res.set_content(buf, "text/html");
        });

        svr.set_logger([&](const Request &req, const Response &res) {
            INFOF("{} {} {} {}", req.method, res.status, req.path, req.version); // NOLINT(bugprone-lambda-function-name)
        });

        svr.set_base_dir(CONFIG_WEBROOT);
        svr.listen(host.c_str(), port);

        INFOF("bind hmi at {}:{}", host, port);
    }

    static void reply_err(httplib::Response &res, const string &err) {
        Json e = {
                {"_err", err},
        };
        res.set_content(e.dump(), CTYPE_JSON);
    }

    void writeResponse2HeXiang(int len) {
        svr.write_response(svr.getCurrentSock(),len);
    }

    socket_t getCurrentSocket() {
        return svr.getCurrentSock();
    }

protected:
    static Json doGetEthernet() {
        auto &&cfg = conf::etc::ether_query();
        return Json{{"proto", cfg.proto},
                    {"ipaddr", cfg.ipaddr},
                    {"netmask", cfg.netmask},
                    {"gateway", cfg.gateway},
                    {"dns", cfg.dns},
        };
    }
    static string doPostEthernet(const Json &body) {
        auto cfg = conf::etc::ether_query();
        string err;
        jsons::optional("proto",body , cfg.proto);
        if (cfg.is_static()) {
            jsons::optional("ipaddr",body , cfg.ipaddr);
            jsons::optional("netmask",body , cfg.netmask);
            jsons::optional("gateway",body ,  cfg.gateway);
            jsons::optional("dns",body, cfg.dns);
            if (cfg.ipaddr.empty() || cfg.netmask.empty() || cfg.gateway.empty() || cfg.dns.empty()) {
                err = "Bad Request";
            }
        } else if (!cfg.is_dhcp()) {
            err = "Bad Request";
        }
        if (err.empty()) {
            conf::etc::ether_setup(cfg);
        }
        return err;
    }
};

#endif //LORALAN_BACKHAUL_API_HPP
