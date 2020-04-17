//
// Created by Thinkpad on 2017/11/2.
//

#include <lang/lang-os.h>
#include <unistd.h>
#include <sys/reboot.h>
#include "lang-log.h"
#include <resolv.h>

namespace lang {

    namespace os {
        void reboot() {
            sync();
            int ret = ::reboot(RB_AUTOBOOT);
            WARNF("reboot! {}, {}", std::strerror(errno), ret);
        }

    }
    namespace net {

        sockaddr_in resolve_host(const char* hostname)
        {
            sockaddr_in address{};
            address.sin_family = AF_UNSPEC;
            addrinfo *result{};
            addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

            res_init();
            const auto rc = getaddrinfo(hostname, NULL, &hints, &result);
            if (rc == 0) {
                for (addrinfo* res = result; res; ) {
                    if (res->ai_family == AF_INET) {
                        address.sin_family = AF_INET;
                        address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
                        break;
                    }
                    res = res->ai_next;
                }
                freeaddrinfo(result);
            } else {
                WARNF("unable to resolve {} rc {} {}", hostname, rc, gai_strerror(rc));
            }
            return address;
        }
    }

}
