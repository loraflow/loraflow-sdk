//
// Created by Thinkpad on 2017/11/23.
//

#include <app/App.h>

void App::_init() {
    _router = new Router(_up_messages, _dn_messages);
#ifdef CONFIG_USE_UDPFRONT
    _frouter = new RouteUDP(_router);
#endif
#ifdef CONFIG_USE_LRMODFRONT
    _frouter = new RouteLRMod(_router);
#endif
#if !defined(CONFIG_USE_UDPFRONT) && !defined(CONFIG_USE_LRMODFRONT)
#error missing CONFIG_USE_UDPFRONT, CONFIG_USE_LRMODFRONT, or ...
#endif
    _router->init(_frouter);

#if defined(CONFIG_USE_EXTNS)
    const string runtype = "external";
#else
    const auto &nsconf = conf::INSTANCE.c_local().networkservice;
    const string runtype = nsconf.runtype.size() ? nsconf.runtype : nsconf.type;
#endif
    if (runtype == "external") {
        _backend = &extns::_backend();
    }
#ifdef CONFIG_SUPPORT_EMBEDNS
    if (runtype == "local") {
            _backend = &ns::_backend();
    }
#endif
    if (!_backend) {
        if (runtype != "") {
            ERRORF("bad backhaul type {}", runtype);
            exit(-1);
        } else {
            ERRORF("missing backhaul type");
            exit(-1);
        }
    }

    _backend->init(&_up_messages, &_dn_messages);
}
