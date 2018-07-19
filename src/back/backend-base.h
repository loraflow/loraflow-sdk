//
// Created by Thinkpad on 2017/10/17.
//

#ifndef AICAST_BACKHAUL_BACKEND_BASE_H
#define AICAST_BACKHAUL_BACKEND_BASE_H

namespace haul {
    class BaseBackend {
    public:
        virtual void init(MessageQ *up, MessageQ *down) = 0;
        virtual void join() = 0;
    };
}

#endif //AICAST_BACKHAUL_BACKEND_BASE_H
