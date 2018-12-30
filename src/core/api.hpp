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
#include <hythread.h>

#define CTYPE_JSON "application/json"
#define CTYPE_TEXT "text/plain"

#ifndef CONFIG_LOG_SIZE
#define CONFIG_LOG_SIZE  512000
#endif
#define RESULT_GET "Result"
#define RESULT_OK "OK"
#define RESULT_FAILED "ERROR"
#define LOG_SIZE (1000*1024)
using Timepoint = std::chrono::steady_clock::time_point;
using namespace lang;
using namespace httplib;
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


static void parseWebGetRequest(const Request &req, Response &res) {
    TEST("req:path{} ({})",req.path,req.body);
    Json form = Json::parse(req.body);
    if (!form.empty()) {
        string name = jsons::optional<string>("name", form, "NO");
        string path = jsons::optional<string>("path", form, "NO");
        if(name != "NO" && path != "NO" ) {
            INFOF("PATH GET IS : ({})",path + "/" + name);
            string result = lang::os::fread(path + "/" + name);
            res.set_content(result, CTYPE_JSON);
            return;
        }
    }
    res.status = 400;
    string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",RESULT_FAILED);
    res.set_content(test, CTYPE_JSON);
}

static void parseWebPostRequest(const Request &req, Response &res) {
    TEST("req:path{}",req.path);
    string result = "Bad Request";
    Json form = Json::parse(req.body);
    if (!form.empty()) {
        string content = jsons::optional<string>("content", form, "NO");
        string name = jsons::optional<string>("name", form, "NO");
        string path = jsons::optional<string>("path", form, "NO");
        if(content != "NO" && name != "NO" && path != "NO" ) {
            result = "OK";
//            std::fstream dst1(path + "/" + name);
//            mkdir(path.c_str(),0777);  //不管文件夹是否存在，都进行创建操作
            lang::os::fwrite(path + "/" + name,content);
//            dst1.close();
//            res.set_content(result, CTYPE_JSON);
        }
    }
    if(result == "Bad Request") {
        res.status = 400;
    }
    string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",result == "OK" ? RESULT_OK:RESULT_FAILED);
    res.set_content(test, CTYPE_JSON);
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
    int currentTotalLen = 0;
    enum UPGRADE_STATUS : uint8_t {UPGRADE_START,UPGRADE_FAILED,UPGRADE_SUCCESS,UPGRADE_ING,UPGRADE_MAX};
    UPGRADE_STATUS upgradeSuccess = UPGRADE_START;

public:
    WebAPI():_uptime(lang::os::now()) {}

    bool Running() const {
        return svr.is_running();
    }

    void setUpgardeSuccess() {
        upgradeSuccess = UPGRADE_SUCCESS;
    }

    void setUpgradeFailed() {
        upgradeSuccess = UPGRADE_FAILED;
    }

    void setUpgradeIng() {
        upgradeSuccess = UPGRADE_ING;
    }

    void setUpgradeStart() {
        upgradeSuccess = UPGRADE_START;
    }

    void Bind(const std::map<string, Cnode *> &nodesmap, const string &host, int port) {

        svr.Post("/type=local_get", [&](const Request &req, Response &res) {
            parseWebGetRequest(req, res);
        });

        svr.Post("/type=global_get", [&](const Request &req, Response &res) {
            parseWebGetRequest(req, res);
        });
        svr.Post("/type=log_get", [&](const Request &req, Response &res) {
            Json form = Json::parse(req.body);
            if (!form.empty()) {
                string name = jsons::optional<string>("name", form, "NO");
                string path = jsons::optional<string>("path", form, "NO");
                if(name != "NO" && path != "NO" ) {
                    INFOF("PATH GET IS : ({})",path + "/" + name);
                    string result1 = lang::os::ftail(path + "/" + name, LOG_SIZE);
                    res.set_content(result1, CTYPE_JSON);
                }
            }
        });
        svr.Post("/type=network_get", [&](const Request &req, Response &res) {
            parseWebGetRequest(req, res);
        });
        svr.Post("/type=ntp_get", [&](const Request &req, Response &res) {
            parseWebGetRequest(req, res);
        });
        svr.Post("/type=state", [&](const Request &req, Response &res) {
            parseWebGetRequest(req, res);
        });
        svr.Post("/type=info", [&](const Request &req, Response &res) {
            parseWebGetRequest(req, res);
        });
        svr.Get("/type=rate", [&](const Request &req, Response &res) {
            TEST("req:path{}",req.path);
#define RATE "rate"
            INFOF("RATE:({}:{})",currentTotalLen,hylength);
            int rate = 0;
            string path = "/root/.upgrade/status";
            std::fstream dst1;
            string result = lang::os::fread(path);
            dst1.close();
            if (result == "0\n") {
                setUpgardeSuccess();
            }else if (result == "1\n"){
                setUpgradeFailed();
            }else {
                setUpgradeIng();
            }
            if (upgradeSuccess == UPGRADE_SUCCESS) {
                rate = 100;
            }else if (upgradeSuccess == UPGRADE_FAILED){
                rate = -1;
            }else if(upgradeSuccess == UPGRADE_ING){
                // 404 not found
                res.status = 404;
            }else if (hylength >= currentTotalLen && currentTotalLen > 0) {
                rate = 50;
            }else {
                if (currentTotalLen > 0) {
                    rate = (int)((double)(hylength*100)/(double)currentTotalLen);
                    rate = rate >= 50 ? 40 : rate;
                }else {
                    res.status = 404;
                }
            }
            string test = strings::sprintf("{" "\n\"" RATE "\":\"%d\" \n" "}\n",rate);
            res.set_content(test, CTYPE_JSON);
        });
        svr.Post("/type=upgrade", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            //decode Base64
            INFOF("start decode Base64 : Matches::({});({})",req.matches[1],req.matches[2]);
            string result = "Bad Request";
            if (!req.body.empty()) {
                INFOF("start decode Base64");
                Json form = Json::parse(req.body);
                if (!form.empty()) {
                    string content = jsons::optional<string>("content",form, "NO");
                    string name = jsons::optional<string>("name",form, "NO");
                    string path = jsons::optional<string>("path",form, "NO");
                    if (content != "NO" && name != "NO" && path != "NO" ) {

                        INFOF("START FILE OPERATION");
                        std::fstream dst1;
                        mkdir(path.c_str(),0777);
                        lang::os::fwrite(path + "/" + name, base64_decode(content));
                        dst1.close();
                        INFOF("STOP FILE OPERATION");
                        result = "OK";
                        upgradeSuccess = UPGRADE_START;
                    }
                    INFOF("Upgrade name({}):path({})",name,path);
                }
            }
            INFOF("RESULT:{}",result);
            string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",result == "OK" ? RESULT_OK:RESULT_FAILED);
            res.set_content(test, CTYPE_JSON);
        });

        svr.Post("/type=local_post", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            parseWebPostRequest(req,res);
        });

        svr.Post("/type=global_post", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            parseWebPostRequest(req,res);
        });

        svr.Post("/type=network_post", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            parseWebPostRequest(req,res);
        });

        svr.Post("/type=log_post", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            parseWebPostRequest(req,res);
        });

        svr.Post("/type=ntp_post", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            parseWebPostRequest(req,res);
        });

        svr.Post("/type=size", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            string result = "Bad Request";
            INFOF("size BODY is {}",req.body);
            Json form = Json::parse(req.body);
            if (!form.empty()) {
                int hysize = jsons::optional<int>("size",form, -1);
                if (hysize != -1) {
                    result = "OK";
                }
                currentTotalLen = hysize;
                INFOF("Upgrade size({})",currentTotalLen);
            }
            string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",result == "OK" ? RESULT_OK:RESULT_FAILED);
            res.set_content(test, CTYPE_JSON);
        });

        svr.Post("/type=reset", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            INFOF("RESET OK");
            string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",RESULT_OK);
            res.set_content(test, CTYPE_JSON);
            isReset = true;
//            lang::os::sleep_ms(1000);
//            lang::os::reboot();
        });

#define ON_USER "username"
#define ON_LOGIN "login"
#define ON_ERROR "failed"
#define ON_SUCCESS "success"

        svr.Post("/type=changepwd", [&](const Request &req, Response &res) {
            MUST_AUTHORIZED();
            //先验证原密码是否正确
            string name;
            string path;
            string content;

            Json form = Json::parse(req.body);
            if (!form.empty()) {
                content = jsons::optional<string>("content", form, "NO");
                name = jsons::optional<string>("name", form, "NO");
                path = jsons::optional<string>("path", form, "NO");
                if(content == "NO" || name == "NO" || path == "NO" ) {
                    string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",RESULT_FAILED);
                    res.set_content(test, CTYPE_JSON);
                    return ;
                }
            }

            string userinfo = lang::os::fread(path + "/" + name);
            Json info = Json::parse(userinfo);
            string username = jsons::optional<string>("username", info, "admin");
            string password = jsons::optional<string>("password", info, "123456");
            string result = strings::sprintf("{" "\n\"" ON_LOGIN "\":\"%s\" \n" "}\n", ON_SUCCESS);;
            if (!password.empty() && content.size() != 0) {
                Json form = Json::parse(content);
                string s1 = jsons::optional<string>("username",form, "");
                string s2 = jsons::optional<string>("old",form, "");
                string s3 = jsons::optional<string>("new",form, "");
                if (s1 != username || s2 != password) {
                    result = strings::sprintf("{" "\n\"" ON_LOGIN "\":\"%s\" \n" "}\n", ON_ERROR);
                }else {
                    string on_info;
                    on_info.clear();
#define USER "username"
#define PWD  "password"
                    lang::os::fwrite(path + "/" + name, strings::sprintf("{" "\n\"" USER "\":\"%s\",\n " "\"" PWD "\":\"%s\"\n" "}\n",s1.data(),s3.data()));
                }
            }else {
                result = strings::sprintf("{" "\n\"" ON_LOGIN "\":\"%s\" \n" "}\n", ON_ERROR);
            }
            res.set_content(result, CTYPE_TEXT);
        });



        /*+ADD End++++++++++++++++++++++++++++++++++++  +++++++++++++++++++++++++++++++*/
        svr.Post("/type=logout", [&](const Request &req, Response &res) {
            INFOF("LOGIN OUT");
            string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",RESULT_OK);
            res.set_content(test, CTYPE_JSON);
        });
        svr.Post("/type=login", [&](const Request &req, Response &res) {
            string name;
            string path;
            string content;
//            bool login = false;
            INFOF("BAD REQUEST : --> ({})",req.body);
            Json form = Json::parse(req.body);
            if (!form.empty()) {
                content = jsons::optional<string>("content", form, "NO");
                name = jsons::optional<string>("name", form, "NO");
                path = jsons::optional<string>("path", form, "NO");
                if(content == "NO" || name == "NO" || path == "NO" ) {
                    INFOF("BAD REQUEST : --> ({})({})({})",content,name,path);
                    string test = strings::sprintf("{" "\n\"" RESULT_GET "\":\"%s\" \n" "}\n",RESULT_FAILED);
                    res.set_content(test, CTYPE_JSON);
                    return ;
                }
            }
            string userinfo = lang::os::fread(path + "/" + name);
            INFOF("userinfo:({})",userinfo);
            string result = strings::sprintf("{" "\n\"" ON_LOGIN "\":\"%s\" \n" "}\n", ON_SUCCESS);;
            if (userinfo.empty()) {
                result = strings::sprintf("{" "\n\"" ON_LOGIN "\":\"%s\" \n" "}\n", ON_ERROR);
                res.status = 200;
                INFOF("Login Result:({})","EMPTY Info File ");
            }else {
                Json info = Json::parse(userinfo);
                string username = jsons::optional<string>("username", info, "admin");
                string password = jsons::optional<string>("password", info, "123456");
                INFOF("user:{}",username);
                INFOF("pwd:{}",password);
                if (!password.empty() && content.size() != 0) {
                    Json form = Json::parse(content);
                    string s1 = jsons::optional<string>("username",form, "");
                    string s2 = jsons::optional<string>("password",form, "");
                    if (s1 != username || s2 != password) {
                        result = strings::sprintf("{" "\n\"" ON_LOGIN "\":\"%s\" \n" "}\n", ON_ERROR);
                        res.status = 200;
                        INFOF("Login Result:({})","Bad username or password");
                    }else {
                        result = strings::sprintf("{" "\n\"" ON_USER "\":\"%s\",\n " "\"" ON_LOGIN "\":\"%s\"\n" "}\n",username.data(),ON_SUCCESS);
//                        login = true;
                    }
                }else {
                    result = strings::sprintf("{" "\n\"" ON_LOGIN "\":\"%s\" \n" "}\n", ON_ERROR);
                    res.status = 200;
                    INFOF("Login Result:({})","EMPTY username or Password ");
                }
            }
//            _authorized = login;
            res.set_content(result, CTYPE_TEXT);
        });

        svr.set_error_handler([&](const Request & /*req*/, Response &res) {
#define _CODE  "code"
#define _FAILED  "failed"
            string buf = strings::sprintf("{" "\n\"" _CODE "\":\"%s\" \n" "}\n",_FAILED);
            res.set_content(buf, "text/html");
        });

        svr.set_logger([&](const Request &req, const Response &res) {
            INFOF("{} {} {} {}", req.method, res.status, req.path, req.version); // NOLINT(bugprone-lambda-function-name)
        });

        svr.listen(host.c_str(), port);

        INFOF("bind hmi at {}:{}", host, port);
    }

    static void reply_err(httplib::Response &res, const string &err) {
        Json e = {
                {"_err", err},
        };
        res.set_content(e.dump(), CTYPE_JSON);
    }
};

#endif //LORALAN_BACKHAUL_API_HPP
