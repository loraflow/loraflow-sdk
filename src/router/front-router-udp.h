//
// Created by Thinkpad on 2017/11/14.
//

#ifndef AICAST_BACKHAUL_FRONT_ROUTER_UDP_H
#define AICAST_BACKHAUL_FRONT_ROUTER_UDP_H

#include <front/UDPFront.h>
#include "front-router.h"

namespace haul {

    class RouteUDP : public FrontRouter {

        UDPFront  _front;

    public:
        RouteUDP(BackRouter *router) {
            _front.set_router(router);
            _front.start();
        }

        void RoutePullResp(PPullResp pull) override {
            _front.send(std::move(pull));
        }
    };
}
#endif //AICAST_BACKHAUL_FRONT_ROUTER_UDP_H
