//
// Created by Thinkpad on 2017/9/6.
//

#ifndef AICAST_BACKHAUL_UDPSERVER_H
#define AICAST_BACKHAUL_UDPSERVER_H

#include <sys/socket.h>
#include <unistd.h>
#include <lang/errors.h>
#include <lang/lang-os.h>
#include <lang/constants.h>
#include <lang/lang-log.h>
#include <front/Front.h>
#include <router/stat-track.h>
#include <conf/config.h>

namespace haul {

    class UDPFront : public Front {
        sockaddr_in _addrup = {0};
        sockaddr_in _addrdn = {0};
        os::Mutex   _lock;
        int         _sock;
        short       _seq;
        std::thread _thrd;
        uint8_t     _rxbuff[DGRAM_SIZE_BIG+1];
        uint8_t     _txbuff[600];
        uint8_t     _pfproto = 0;
        EUI64       _mac;
    public:

        void start() override {
            Url listen;
            _sock = _open(Url("udp", "0.0.0.0", conf::INSTANCE.local().system.udplisten));
            if (_sock < 0) {
                FATALF("unable to open udp at {}", listen.port);
            }

            _thrd = std::thread(&UDPFront::_daemon_read, this);
        }

        void join() {
            _thrd.join();
        }

        void send(PPullResp pr) override {
            if (_addrdn.sin_addr.s_addr) {
                auto pullresp = pr.get();
                _txbuff[0] = _pfproto;
                endian::put_u16p(_txbuff+1, ++_seq);
                _txbuff[3] = PKT_PULL_RESP;
                int nb = 4 + pullresp->jsonize(_txbuff + 4);
                DEBUGF("TX {}", (char*)_txbuff+4);
                _send(_txbuff, nb, _addrdn);
            } else {
                DEBUGF("no route to pf. discard!");
            }
        }

    private:
        void _daemon_read() {
            struct sockaddr_in clientaddr;
            for (;;) {
                socklen_t clientlen = sizeof(clientaddr);
                auto n = recvfrom(_sock, _rxbuff, sizeof(_rxbuff) - 1, 0,
                                  (struct sockaddr *) &clientaddr, &clientlen);
                if (n <= 0) {
                    FATALF("udp socket read ret {}", n);
                }
                _rxbuff[n] = 0;
                _handle_rx(_rxbuff, n, clientaddr);
            }
        }

        void _handle_rx(uint8_t *buf, int size, sockaddr_in &caddr) {
            if (size < 4) {
                return;
            }
            _pfproto = buf[0];
            switch (buf[3]) {
                case PKT_PUSH_DATA:
                    if (size > 12) {
                        _mac.from(buf+4);
                        _addrup = caddr;
                        _rx_pushdata(buf+12, size-12);
                        _ack(PKT_PUSH_ACK, buf+1);
                    }
                    break;
                case PKT_PULL_DATA:
                    DEBUGF("PULL_DATA");
                    _mac.from(buf+4);
                    _addrdn = caddr;
                    _ack(PKT_PULL_ACK, buf+1);
                    break;
                case PKT_TX_ACK:
                    DEBUGF("TX_ACK");
                    break;
                default:
                    DEBUGF("unknown pkt {}", buf[3]);
                    break;
            }
        }
        void _rx_pushdata(uint8_t *buf, int size) {
            DEBUGF("RXPK {}", (char*)buf);
            PushData *push = new PushData();
            auto rx = Json::parse(buf);
            if (!push->fromJson(rx)) {
                DEBUGF("invalid");
                delete push;
            } else {
                PPushData pd(push);
                _router->RoutePushData(std::move(pd));
            }
            Json jstat;
            if (jsons::optional(rx, "stat", jstat)) {
                tracker::_tracker().refresh(jstat);
            }
        }
        void _ack(uint8_t pktid, uint8_t *token) {
            uint8_t pkt[4];
            pkt[0] = _pfproto;
            pkt[1] = token[0];
            pkt[2] = token[1];
            pkt[3] = pktid;
            auto &addr = pktid == PKT_PULL_ACK ? _addrdn : _addrup;
            _send(pkt, 4, addr);
        }

        void _send(uint8_t *buf, int size, sockaddr_in &addr) {
            GUARD_BEGIN(_lock);
                auto n = ::sendto(_sock, buf, size, 0, (struct sockaddr *) &addr, sizeof(addr));
                if (n < 0) {
                    FATALF("write udp socket ret {}", n);
                }
            GUARD_END();
        }

        static int _open(Url listen) {
            int sockfd;
            int optval;
            struct sockaddr_in serveraddr;

            sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            ASSERT(sockfd != 0);
            if (sockfd < 0) {
                ERRNOF("ERRORF opening socket");
                return -1;
            }

            optval = 1;
            setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                       (const void *)&optval , sizeof(int));

            bzero((char *) &serveraddr, sizeof(serveraddr));
            serveraddr.sin_family = AF_INET;
            serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
            serveraddr.sin_port = htons((unsigned short)listen.port);

            if (bind(sockfd, (struct sockaddr *) &serveraddr,
                     sizeof(serveraddr)) < 0) {
                ERRNOF("udp binding");
                return -1;
            }

            INFOF("opened udp server at {}. sock {}", listen.port, sockfd);
            return sockfd;
        }
    };

}


#endif //AICAST_BACKHAUL_UDPSERVER_H
