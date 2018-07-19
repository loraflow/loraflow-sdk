//
// Created by Thinkpad on 2017/9/12.
//

#ifndef AICAST_BACKHAUL_DIRECTFRONT_H
#define AICAST_BACKHAUL_DIRECTFRONT_H


#include "front/Front.h"

namespace haul {

    using namespace lang;
    using haul::PushData;

    class DirectFront : public Front {
        //Datgrams  _sock_rx;     //pf send to here for uplink/downlink_ack
        //Datgrams  _sockup_tx;   //pf recv from here for uplink_ack
        //Datgrams  _sockdn_tx;   //pf recv from here for downlink

        inline int _send(uint8_t *buff, int size) {
            auto dg = Datgram::allocate(size+1);
            if (!dg) {
                return -1;
            }
            memcpy(dg->buffer, buff, size);
            dg->buffer[size] = '\0';
            dg->len = size;
            dg->addr.sin_addr.s_addr = 0;
            //_sock_rx.put(dg);
            return size;
        }

    public:

        //
        // socksend/sockrecv - sock simulate api for PF
        //

        Datgram* sockrecv_up(int mills) {
            return nullptr;//_sockup_tx.take(mills);
        }

        Datgram* sockrecv_dn(int mills) {
            return nullptr;//_sockdn_tx.take(mills);
        }

        int socksend_up(uint8_t *buff, int size) {
            return _send(buff, size);
        }

        int socksend_dn(uint8_t *buff, int size) {
            return _send(buff, size);
        }

        void rx_push_data(lgw_pkt_rx_s *rxpkt, int nb_pkt) {
            PushData *pd = new PushData;
            pd->fromDirect(rxpkt, nb_pkt);
            PPushData ppd(pd);
            _router->RoutePushData(std::move(ppd));
        }
    };
}

#endif //AICAST_BACKHAUL_DIRECTFRONT_H
