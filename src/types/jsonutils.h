//
// Created by Thinkpad on 2017/9/20.
//

#ifndef AICAST_BACKHAUL_JSON_HELP_H
#define AICAST_BACKHAUL_JSON_HELP_H

#include <ipc/lora-ipc.h>

namespace haul {

    namespace jsonutil {

        using lang::Json;
        using std::string;

        enum Exception {BAD_CRC, BAD_BW, BAD_CR, BAD_MODU};

        int put_crcstatus(uint8_t *buff_up, lora::CRCStatus status);
        int put_lora(uint8_t *buff_up, lora::SF sf, lora::BW bw, lora::CR cr, lora::SNR10_t snr10);
        int put_fsk(uint8_t *buff_up, uint32_t datarate);
        int put_data(uint8_t *buff_up, uint8_t *payload, int size);
        int get_data(uint8_t *in, int size, uint8_t *payload, int maxlen);
    }
}
#endif //AICAST_BACKHAUL_JSON_HELP_H
