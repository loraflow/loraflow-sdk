//
// Created by Thinkpad on 2017/9/12.
//

#ifdef CONFIG_USE_UDPFRONT

#include <lang/lang-os.h>
#include <front/DirectFront.h>
#include <lang/lang-endian.h>
#include <lang/lang-log.h>

using haul::DirectFront;

DirectFront *direct_front;

extern "C" int recv_sock_up(uint8_t *buff_up, int size) {
    //auto dg = direct_front->sockrecv_up(1000);
    //return dg ? dg->claim(buff_up, size) : -1;
    return 0;
}

extern "C" int send_sock_up(uint8_t *buff, int size) {
    return direct_front->socksend_up(buff, size);
}

extern "C" int recv_sock_down(uint8_t *buff_down, int size) {
    //auto dg = direct_front->sockrecv_dn(500);
    //return dg ? dg->claim(buff_down, size) : -1;
    return 0;
}

extern "C" int send_sock_down(uint8_t *buff_down, int size) {
    return direct_front->socksend_dn(buff_down, size);
}

extern "C" void rx_push_data(struct lgw_pkt_rx_s *rxpkt, int nb_pkt) {
    if (nb_pkt > 0) {
        direct_front->rx_push_data(rxpkt, nb_pkt);
    }
}

#endif //CONFIG_USE_UDPFRONT
