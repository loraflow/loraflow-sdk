//
// Created by Thinkpad on 2017/9/21.
//

#ifndef AICAST_BACKHAUL_TYPES_PULLRESP_H
#define AICAST_BACKHAUL_TYPES_PULLRESP_H

#include <cstdint>
#include <ipc/lora-ipc.h>
#include <types/message.h>
#include <types/copy_loragw_hal.h>
#include <lang/lang-os.h>
#include <lang/lang-json.h>
#include <lang/lang-log.h>

#ifndef CONFIG_DEFAULT_PREAMBLE
#define CONFIG_DEFAULT_PREAMBLE 8
#endif

namespace haul {

    using namespace lang;
    using lang::Json;
    using lora::TxReq;
    using lora::BW;
    using lora::SF;
    using lora::CR;

    class PullResp : public Message {
        uint8_t     _buf[256];   /*!> buffer containing the payload */
    public:
        TxReq       _tx;
        bool        _longdelay = false;

        uint8_t get_type() override {
            return PKT_PULL_RESP;
        }
        void fromTxReq(const TxReq &tx);
        bool fromBinary(const uint8_t *const buf, int size);
        bool fromText(const uint8_t *text, int size);
        int jsonize(uint8_t *buff) override;
    };

    using PPullResp = std::unique_ptr<PullResp>;
}

#endif //AICAST_BACKHAUL_TYPES_PULLRESP_H
