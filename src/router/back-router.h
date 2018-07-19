//
// Created by Thinkpad on 2017/11/14.
//

#ifndef AICAST_BACKHAUL_BACK_ROUTER_H
#define AICAST_BACKHAUL_BACK_ROUTER_H

#include <types/pushdata.h>

namespace haul {

    class BackRouter {
    public:
        virtual void RoutePushData(PPushData push) = 0;
    };
}

#endif //AICAST_BACKHAUL_BACK_ROUTER_H
