//
// Created by Thinkpad on 2017/9/20.
//

#ifndef AICAST_BACKHAUL_TRANSLATE_H
#define AICAST_BACKHAUL_TRANSLATE_H

#include <ipc/lora-ipc.h>
#include <types/copy_loragw_hal.h>

namespace haul {
    namespace translate {
        using lora::SF;
        using lora::BW;
        using lora::CR;
        using lora::Modu;
        using lora::CRCStatus;
        inline time_t to_lora_time(uint32_t semt_count_us) {
            return semt_count_us/100;
        }
        inline SF to_lora_sf(uint8_t semt_datarate) {
            switch (semt_datarate) {
                case DR_LORA_SF7:
                    return lora::SF7;
                case DR_LORA_SF8:
                    return lora::SF8;
                case DR_LORA_SF9:
                    return lora::SF9;
                case DR_LORA_SF10:
                    return lora::SF10;
                case DR_LORA_SF11:
                    return lora::SF11;
                case DR_LORA_SF12:
                    return lora::SF12;
                default:
                    return lora::SF_INVALID;
            }
        }
        inline BW to_lora_bw(uint8_t semt_bw) {
            switch (semt_bw) {
                case BW_125KHZ:
                    return lora::BW_125K;
                case BW_250KHZ:
                    return lora::BW_250K;
                case BW_500KHZ:
                    return lora::BW_500K;
                default:
                    return lora::BW_INVALID;
            }
        }
        inline CR to_lora_cr(uint8_t semt_cr) {
            return (CR)semt_cr;
        }
        inline CRCStatus to_lora_crcstatus(uint8_t semt_cr) {
            switch (semt_cr) {
                case STAT_CRC_OK:
                    return lora::CRCST_OK;
                case STAT_CRC_BAD:
                    return lora::CRCST_BAD;
                case STAT_NO_CRC:
                    return lora::CRCST_NOCRC;
                default:
                    return lora::CRCST_UNDEFINED;
            }
        }
        inline Modu to_lora_modu(uint8_t semt_modu) {
            switch (semt_modu) {
                case MOD_LORA:
                    return lora::MODU_LORA;
                case MOD_FSK:
                    return lora::MODU_FSK;
                default:
                    return lora::MODU_UNDEFINED;
            }
        }
    }
}
#endif //AICAST_BACKHAUL_TRANSLATE_H
