//
// Created by Thinkpad on 2017/10/17.
//

#ifndef AICAST_BACKHAUL_CONFIG_LOCAL_H
#define AICAST_BACKHAUL_CONFIG_LOCAL_H

#include <cstdint>
#include <lang/lang-json.h>
#include <lang/errors.h>
#include <conf/conf-networkservice.h>
#include <conf/conf-system.h>

namespace conf {

    using namespace lang;

    struct Local {

        System          system;
        NetworkService  networkservice;

        errors::ErrStatus load_from(const Json &j) {
            system.load(jsons::optional(j, "system"));
            networkservice.load(jsons::optional(j, "networkservice"));
            return errors::ERR_OK;
        }
        Json get() const {
            return Json{
                    {"system", system.save()},
                    {"networkservice", networkservice.save()},
            };
        }
    };

}

#endif //AICAST_BACKHAUL_CONFIG_LOCAL_H
