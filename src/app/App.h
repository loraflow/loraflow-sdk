//
// Created by Thinkpad on 2017/9/6.
//

#ifndef AICAST_BACKHAUL_APP_H
#define AICAST_BACKHAUL_APP_H


#include <cstring>
#include <lang/lang-os.h>
#include <lang/lang-log.h>
#include <conf/config.h>
#include <router/Router.h>
#include <backend-base.h>
#include <external/extns.h>
#include <front/Front.h>
#include <types/message.h>
#include <types/pullresp.h>
#include <dev/SerialPort.h>
#include <router/front-router.h>
#ifdef CONFIG_USE_LRMODFRONT
#include <router/front-router-lrmod.h>
#endif
#ifdef CONFIG_USE_UDPFRONT
#include <router/front-router-udp.h>
#endif

#ifdef CONFIG_SUPPORT_EMBEDNS
#endif

using namespace haul;

class App {

    MessageQ    _up_messages;
    MessageQ    _dn_messages;
    Router      *_router = nullptr;
    BaseBackend *_backend = nullptr;
    FrontRouter *_frouter;

    void _init();

public:

    void run() {
        _init();
        _router->join();
        if (_backend) {
            _backend->join();
        }
    }
};


#endif //AICAST_BACKHAUL_APP_H
