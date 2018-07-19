//
// Created by Thinkpad on 2017/11/16.
//

#include <lang/lang-base64.h>
#include <lang/lang-json.h>
#include <lang/lang-log.h>
#include <types/jsonutils.h>

namespace haul {

    namespace jsonutil {

        int put_crcstatus(uint8_t *buff_up, lora::CRCStatus status) {
            switch (status) {
                case lora::CRCST_OK:
                    memcpy(buff_up, (void *)",\"stat\":1", 9);
                    return 9;
                case lora::CRCST_BAD:
                    memcpy(buff_up, (void *)",\"stat\":-1", 10);
                    return 10;
                case lora::CRCST_NOCRC:
                    memcpy(buff_up, (void *)",\"stat\":0", 9);
                    return 9;
                default:
                    ERRORF("[up] unknown crcstatus {}", status);
                    throw BAD_CRC;
            }
        }

        int put_lora(uint8_t *buff_up, lora::SF sf, lora::BW bw, lora::CR cr, lora::SNR10_t snr10) {
            memcpy(buff_up, (void *)",\"modu\":\"LORA\"", 14);
            int buff_index = 14;

            /* Lora datarate & bandwidth, 16-19 useful chars */
            memcpy((void *)(buff_up + buff_index), (void *)",\"datr\":\"SF", 11);
            buff_index += 11;
            buff_index += sprintf((char*)buff_up + buff_index, "%d", sf);

            if (!INRANGE(bw, lora::BW_125K, lora::BW_500K)) {
                ERRORF("[up] unknown bandwidth {}", bw);
                throw BAD_BW;
            }
            static const char* bw_texts[] = {"BW125\"", "BW250\"", "BW500\""};
            memcpy(buff_up + buff_index, bw_texts[bw - lora::BW_125K], 6);
            buff_index += 6;

            if (!INRANGE(cr, lora::CR_4_5, lora::CR_4_8)) {
                ERRORF("[up] unknown coderate {}", cr);
                throw BAD_CR;
            }
            static const char* cr_texts[] = {",\"codr\":\"4/5\"", ",\"codr\":\"4/6\"", ",\"codr\":\"4/7\"", ",\"codr\":\"4/8\""};
            memcpy(buff_up + buff_index, cr_texts[cr - lora::CR_4_5], 13);
            buff_index += 13;

            if (snr10 < 0) {
                snr10 = -snr10;
                buff_index += sprintf((char *)(buff_up + buff_index), ",\"lsnr\":-%d.%d", snr10/10, snr10%10);
            } else {
                buff_index += sprintf((char *)(buff_up + buff_index), ",\"lsnr\":%d.%d", snr10/10, snr10%10);
            }
            return buff_index;
        }
        int put_fsk(uint8_t *buff_up, uint32_t datarate) {
            memcpy(buff_up, (void *)",\"modu\":\"FSK\"", 13);
            int buff_index = 13;
            buff_index += sprintf((char *)(buff_up + buff_index), ",\"datr\":%u", datarate);
            return buff_index;
        }
        int put_data(uint8_t *buff_up, uint8_t *payload, int size) {
            /* Packet base64-encoded payload, 14-350 useful chars */
            memcpy(buff_up, ",\"data\":\"", 9);
            int buff_index = 9;
            buff_index += lang::base64::bin_to_b64(payload, size, (char *)(buff_up + buff_index), 341); /* 255 bytes = 340 chars in b64 + null char */
            return buff_index;
        }
        inline int get_data(uint8_t *in, int size, uint8_t *payload, int maxlen) {
            return lang::base64::b64_to_bin((char*)in, size, payload, maxlen); /* 255 bytes = 340 chars in b64 + null char */
        }
    }
}