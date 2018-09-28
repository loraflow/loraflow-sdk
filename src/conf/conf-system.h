//
// Created by Thinkpad on 2017/10/17.
//

#ifndef AICAST_BACKHAUL_CONFIG_SYSTEM_H
#define AICAST_BACKHAUL_CONFIG_SYSTEM_H

#include <cstdint>
#include <ipc/lora-ipc.h>
#include <lang/lang-json.h>
#include <lang/errors.h>
#include "conf-etc.h"

namespace conf {
    using namespace lang;

#define RUNNING_HOME   "/mnt/mmcblk0p1"
#define UPGRADEFILE   "nanohaul_upgrade.zip"

    /*
     * "datahome": "/mnt/mmcblk0p1/nanohaul",
       "assetshome": "/usr/nanohaul/assets",
       "dbhome": "/mnt/mmcblk0p1/nanohaul",
       "uphome": "/mnt/mmcblk0p1/nanohaul/up"
       upgradeDir=${SD_DIR}/upgrade
       upgradeFile="nanohaul_upgrade.zip"
       resultFile="result"
       SDDIR = /mnt/mmcblk0p1/
            */
    struct System {
        string          gatewayID;
        string          password = "admin";
        string          mod1 = "/dev/ttyS1";
        string          mod2 = "/dev/ttyS0";
        string          datahome = ".";
        string          dbhome = ".";
        string          uphome = RUNNING_HOME "/upgrade";
        string          upgradefile = RUNNING_HOME "/" UPGRADEFILE;
        string          assets_home = "./assets";
        string          datafile = "LORAWANTRACK-";
        string          loglevel = "info";
        bool            compatible_pfv02 = false;
        bool            muteled = false;
        string          ulnwk;
        int32_t         weblisten = 8080;
        int32_t         udplisten = 1680;
        int32_t         pfwatchttl = 4200;
        string          pfwatchcmd = "systemctl restart lrgateway";
        uint32_t        catchtimeout = 0;   //for how long will message be kept in case of network failure (in seconds)

        errors::ErrStatus load(const Json &j) {
            jsons::optional(j, "gateway_ID", gatewayID);
            jsons::optional(j, "log", loglevel);
            jsons::optional(j, "mod1", mod1);
            jsons::optional(j, "mod2", mod2);
            jsons::optional(j, "web", weblisten);
            jsons::optional(j, "udplisten", udplisten);
            jsons::optional(j, "pfwatchttl", pfwatchttl);
            jsons::optional(j, "pfwatchcmd", pfwatchcmd);
            jsons::optional(j, "password", password);
            jsons::optional(j, "datahome", datahome);
            jsons::optional(j, "assetshome", assets_home);
            jsons::optional(j, "dbhome", dbhome);
            jsons::optional(j, "uphome", uphome);
            jsons::optional(j, "upgradefile", upgradefile);
            jsons::optional(j, "compatible_pfv02", compatible_pfv02);
            jsons::optional(j, "muteled", muteled);
            jsons::optional(j, "ulnwk", ulnwk);
            jsons::optional(j, "catchtimeout", catchtimeout);
            std::transform(gatewayID.begin(), gatewayID.end(), gatewayID.begin(), ::tolower);
            return errors::ERR_OK;
        }
        Json save() const {
            return Json{
                    {"version", lang::os::version()},
                    {"ulnwk", ulnwk},
                    {"muteled", muteled},
                    {"password", password},
                    {"ulnwk_list", etc::ulnwk_list()},
            };
        }
    };

}

#endif //AICAST_BACKHAUL_CONFIG_SYSTEM_H
