//
// Created by Thinkpad on 2017/9/8.
//

#ifndef AICAST_BACKHAUL_TCPADAPTOR_H
#define AICAST_BACKHAUL_TCPADAPTOR_H


#include <netdb.h>
#include <external/extns-transport.h>

namespace haul {

    namespace extns {

        class TCPAdaptor : public Adaptor {
            class TcpSock : public Transport {
                const int _sock;
            public:
                explicit TcpSock(int sock):_sock(sock) {}

                int Reade(void *ptr, int size) override {
                    const int re = read(_sock, ptr, size);
                    if (re == -1) {
                        const auto err = errno;
                        return (err == EWOULDBLOCK || err == EAGAIN) ? 0 : -1;
                    } else {
                        return re > 0 ? re : -1;
                    }
                }

                int Write(const void *ptr, int size) override {
                    return (int)send(_sock, ptr, size, MSG_NOSIGNAL);
                }

            private:
                int Close() override {
                    return close(_sock);
                }
            };
        public:
            Transport *connect(Url &url) override {

                sockaddr_in serveraddr = lang::net::resolve_host(url.host.c_str());
                if (serveraddr.sin_family == AF_UNSPEC) {
                    ERRORF("get host {}", url.host);
                    return nullptr;
                }

                int sockfd = socket(serveraddr.sin_family, SOCK_STREAM, 0);
                if (sockfd < 0) {
                    ERRNOF("opening socket");
                    return nullptr;
                }

                ASSERT(sockfd != 0);
                {
                    int optval = 1;
                    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int));
                }

                serveraddr.sin_port = htons((unsigned short)url.port);
                if (::connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
                    ERRNOF("connect");
                    close(sockfd);
                    return nullptr;
                }
                struct timeval tv{};
                tv.tv_sec = 2;
                tv.tv_usec = 0;
                setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
                return new TcpSock(sockfd);
            }
        };
    }
}

#endif //AICAST_BACKHAUL_TCPADAPTOR_H
