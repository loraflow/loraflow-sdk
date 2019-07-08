//
// Created by Thinkpad on 2017/9/19.
//

#ifndef AICAST_BACKHAUL_PUSHDATA_H
#define AICAST_BACKHAUL_PUSHDATA_H

#include <lang/lang-os.h>
#include <lang/lang-endian.h>
#include <lang/lang-log.h>
#include <types/copy_loragw_hal.h>
#include <lang/lang-types.h>
#include <ipc/lora-ipc.h>
#include <types/message.h>
#include <types/translate.h>
#include <types/jsonutils.h>

namespace haul {

    using endian = lang::LittleEndian;
    using lora::RxInfo;
    using lora::RxInd;
    using lora::BW;
    using lora::SF;
    using lora::CR;

    class PushData : public Message {
        uint8_t *rawbuf{};
    public:
        struct RX {
            RxInfo      info{};
            uint8_t     size{};
            uint8_t     *payload{};
        };
        RX          _rxs[8]{};
        uint8_t     _nbpkt{};
        const uint64_t  _rxmills;
    public:
        PushData():_rxmills(lang::os::mills()) {}

        ~PushData() override {
            delete []rawbuf;  // delete null pointer has no effect
        }

        uint8_t get_type() override {
            return PKT_PUSH_DATA;
        }
        int64_t age() override {
            return (lang::os::mills() - _rxmills) / 1000;
        }
        void fromDirect(const lgw_pkt_rx_s *lgw_rxs, int nb_pkt);
        void fromRxInd(const RxInd &ind);
        bool fromJson(const Json &j);
        int toBinary(uint8_t *buff);
        int jsonize(uint8_t *buff) override;
    };

    using PPushData = std::unique_ptr<PushData>;
}


#endif //AICAST_BACKHAUL_PUSHDATA_H
