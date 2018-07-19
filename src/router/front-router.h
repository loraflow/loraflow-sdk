//
// Created by Thinkpad on 2017/11/14.
//

#ifndef AICAST_BACKHAUL_FRONT_ROUTER_H
#define AICAST_BACKHAUL_FRONT_ROUTER_H

#include <types/pullresp.h>

namespace haul {
    class FrontRouter {
    public:
        virtual void RoutePullResp(PPullResp pull) = 0;
    };
}

#endif //AICAST_BACKHAUL_FRONT_ROUTER_H
