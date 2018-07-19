//
// Created by Thinkpad on 2017/10/17.
//

#ifndef AICAST_BACKHAUL_LANG_CONFIG_BACKHAUL_H
#define AICAST_BACKHAUL_LANG_CONFIG_BACKHAUL_H

#include <lang/errors.h>
#include <lang/lang-json.h>

namespace conf {
    using namespace lang;

    struct NetworkService {

        struct External {
            string      url = "loraflow.io";
            string      token;

            errors::ErrStatus load(const Json &j) {
                jsons::optional(j, "url", url);
                jsons::optional(j, "token", token);
                return errors::ERR_OK;
            }
            Json save() const {
                return Json{
                        {"url", url},
                        {"token", token},
                };
            }
         };
        struct Local {
            struct AppService {
                struct Http {
                    string  method = "POST";
                    string  url;

                    errors::ErrStatus load(const Json &j) {
                        jsons::optional(j, "method", method);
                        jsons::optional(j, "url", url);
                        return errors::ERR_OK;
                    }
                    Json save() const {
                        return Json{
                                {"method", method},
                                {"url", url},
                        };
                    }
                };
                struct Mqtt {
                    string  server;
                    string  topic;
                    string  subtopic;
                    string  clientid;
                    string  user;
                    string  password;
                    string  version = "3.1";
                    int     keepalive = 50;

                    errors::ErrStatus load(const Json &j) {
                        jsons::optional(j, "server", server);
                        jsons::optional(j, "topic", topic);
                        jsons::optional(j, "subtopic", subtopic);
                        jsons::optional(j, "clientid", clientid);
                        jsons::optional(j, "user", user);
                        jsons::optional(j, "password", password);
                        jsons::optional(j, "version", version);
                        jsons::optional(j, "keepalive", keepalive);
                        return errors::ERR_OK;
                    }
                    Json save() const {
                        return Json {
                                {"server", server},
                                {"topic", topic},
                                {"subtopic", subtopic},
                                {"clientid", clientid},
                                {"version", version},
                                {"keepalive", keepalive},
                                {"user", user},
                                {"password", password},
                        };
                    }
                };
                struct Senzflow {
                    string  token;

                    errors::ErrStatus load(const Json &j) {
                        jsons::optional(j, "token", token);
                        return errors::ERR_OK;
                    }
                    Json save() const {
                        return Json{{"token", token}};
                    }
                };

                string       type;
                Http         http;
                Mqtt         mqtt;
                Senzflow     senzflow;

                errors::ErrStatus load(const Json &j) {
                    jsons::optional(j, "type", type);
                    http.load(jsons::optional(j, "http"));
                    mqtt.load(jsons::optional(j, "mqtt"));
                    senzflow.load(jsons::optional(j, "senzflow"));
                    return errors::ERR_OK;
                }

                Json save() const {
                    return Json{
                            {"type", type},
                            {"http", http.save()},
                            {"mqtt", mqtt.save()},
                            {"senzflow", senzflow.save()},
                    };
                }
            };

            AppService  appservice;
            string      netid;
            string      band;

            errors::ErrStatus load(const Json &j) {
                appservice.load(jsons::optional(j, "appservice"));
                jsons::optional(j, "band", band);
                jsons::optional(j, "netid", netid);
                return errors::ERR_OK;
            }

            Json save() const {
                return Json{
                        {"band", band},
                        {"netid", netid},
                        {"appservice", appservice.save()},
                };
            }
        };

        string          type;
        External        external;
        Local           local;

        string          runtype;   //run-time state, does not persist

        errors::ErrStatus load(const Json &j) {
            jsons::optional(j, "type", type);
            jsons::optional(j, "type", runtype);
            external.load(jsons::optional(j, "external"));
            local.load(jsons::optional(j, "local"));
            return errors::ERR_OK;
        }
        Json save() const {
            return Json{
                    {"type", type},
                    {"external", external.save()},
                    {"local", local.save()},
            };
        }
    };
}

#endif //AICAST_BACKHAUL_LANG_CONFIG_BACKHAUL_H
