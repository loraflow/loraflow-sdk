//
// Created by Thinkpad on 2017/11/14.
//

#ifndef AICAST_BACKHAUL_FRONT_ROUTER_LRMOD_H
#define AICAST_BACKHAUL_FRONT_ROUTER_LRMOD_H


#include <front/LRModFront.h>
#include "back-router.h"
#include "front-router.h"

namespace haul {

    class RouteLRMod : public FrontRouter {

        LRModFront  *_front1 = nullptr;
        LRModFront  *_front2 = nullptr;

    public:
        RouteLRMod(BackRouter *router) {
            auto &cfg = conf::INSTANCE;
            const auto &l = cfg.c_local();
            const auto &g = cfg.c_global();

            const auto &sysconf = l.system;

            if (sysconf.mod1.size()) {
                const auto &conf = g.radio0;
                _front1 = new LRModFront(sysconf.mod1, 0, 0);
                _front1->set_router(router);
                _front1->configure(conf);
            }
            if (sysconf.mod2.size()) {
                const auto &conf = g.radio1;
                _front2 = new LRModFront(sysconf.mod2, 1, 1);
                _front2->set_router(router);
                _front2->configure(conf);
            }
            conf::INSTANCE.listen([this]() {
                const auto &g = conf::INSTANCE.c_global();
                if (_front1) {
                    _front1->configure(g.radio0);
                }
                if (_front2) {
                    _front2->configure(g.radio1);
                }
            });
        }

        void RoutePullResp(PPullResp pull) override {
            LRModFront  *p = pull->_tx.rf_chain == 0 || !_front2 ? _front1 : _front2;
            if (p) {
                p->send(std::move(pull));
            }
        }
    };
}
#endif //AICAST_BACKHAUL_FRONT_ROUTER_LRMOD_H
