//
// Created by Thinkpad on 2017/9/8.
//

#ifndef AICAST_BACKHAUL_TLSTRANSPORT_H
#define AICAST_BACKHAUL_TLSTRANSPORT_H


#include <external/extns-transport.h>

extern void ssl_init(void);

namespace haul {

    namespace extns {

        class extns : public Adaptor {

            virtual Transport *connect(Url &url) {
                return nullptr;
            }

        public:
            struct SSL_INITIALIZER {
                SSL_INITIALIZER() {
                    ssl_init();
                }
            };
            static SSL_INITIALIZER initializer;
        };
    }
}

#endif //AICAST_BACKHAUL_TLSTRANSPORT_H
