//
// Created by Thinkpad on 2017/9/12.
//

#ifndef AICAST_BACKHAUL_FRONT_H
#define AICAST_BACKHAUL_FRONT_H


/*
 *        +----------------------+
 *        |       Backend        |
 *        +----------------------+
 *                   ^
 *                   |    incoming/outgoing  (from Router's point of view)
 *                   v
 *        +----------------------+
 *        |        Router        |
 *        +----------------------+
 *                   ^
 *                   |    incoming/outgoing  (from Router's point of view)
 *                   v
 *        +----------------------+
 *        |        Front         |
 *        +----------------------+
 *          (sockup)     (sockdn)
 *             ^|          ^|
 *             ||          ||      sockrecv/socksend up/dn
 *             |v          |v
 *        +----------------------+
 *        |      PktFwd          |
 *        +----------------------+
 * */

#include <types/pushdata.h>
#include <types/pullresp.h>
#include <router/back-router.h>

namespace haul {

    using namespace lang;

    class Front {
    public:

        void set_router(BackRouter *router) {
            _router = router;
        };

        virtual void start() {};
        virtual void send(PPullResp pr) = 0;

    protected:

        BackRouter *_router;
    };
}


#endif //AICAST_BACKHAUL_FRONT_H
