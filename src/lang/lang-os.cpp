//
// Created by Thinkpad on 2017/11/2.
//

#include <lang/lang-os.h>
#include <unistd.h>
#include <sys/reboot.h>
#include "lang-log.h"

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
            sockaddr_in address = {0};
            address.sin_family = AF_UNSPEC;
            struct addrinfo *result;
            struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

            if (getaddrinfo(hostname, NULL, &hints, &result) == 0)
            {
                struct addrinfo* res = result;

                /* prefer ip4 addresses */
                while (res)
                {
                    if (res->ai_family == AF_INET)
                    {
                        result = res;
                        break;
                    }
                    res = res->ai_next;
                }

                if (result->ai_family == AF_INET) {
                    address.sin_family = AF_INET;
                    address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
                }

                freeaddrinfo(result);
            }
            return address;
        }
    }

}
