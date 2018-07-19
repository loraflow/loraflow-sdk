//
// Created by Thinkpad on 2017/9/8.
//

#ifndef AICAST_BACKHAUL_TCPADAPTOR_H
#define AICAST_BACKHAUL_TCPADAPTOR_H


#include <netdb.h>
#include <external/extns-transport.h>

namespace haul {

    namespace extns {

        class extns : public Adaptor {
            class TcpSock : public Transport {
                const int _sock;
            public:
                TcpSock(int sock):_sock(sock) {
                }

                int Reade(void *ptr, int size) override {
                    return (int)read(_sock, ptr, size);
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
            virtual Transport *connect(Url &url) {

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
                return new TcpSock(sockfd);
            }
        };
    }
}

#endif //AICAST_BACKHAUL_TCPADAPTOR_H
