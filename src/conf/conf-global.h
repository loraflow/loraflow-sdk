//
// Created by Thinkpad on 2017/10/17.
//

#ifndef AICAST_BACKHAUL_CONFIG_GLOBAL_H
#define AICAST_BACKHAUL_CONFIG_GLOBAL_H

#include <cstdint>
#include <ipc/lora-ipc.h>
#include <lang/lang-json.h>
#include <lang/errors.h>

namespace conf {

    using namespace lang;
    using namespace lang::errors;

    namespace priv {
        inline uint8_t to_sfmask(const list<int> &sf) {
            if (sf.size() == 0) {
                return (1<<(lora::SF7-lora::SF_MIN)) | (1<<(lora::SF8-lora::SF_MIN)) |
                       (1<<(lora::SF9-lora::SF_MIN)) | (1<<(lora::SF10-lora::SF_MIN)) |
                       (1<<(lora::SF11-lora::SF_MIN)) | (1<<(lora::SF12-lora::SF_MIN));
            }
            uint8_t mask = 0;
            for (auto it = sf.cbegin(); it != sf.cend(); ++it) {
                int sf = *it;
                if (INRANGE(sf, lora::SF_MIN, lora::SF_MAX)) {
                    mask |= 1<<(sf - lora::SF_MIN);
                }
            }
            return mask;
        }
        inline list<int> from_sfmask(uint8_t mask) {
            list<int> sf;
            for (int i=lora::SF_MIN; i<= lora::SF_MAX; ++i) {
                if ((mask & (1 << (i-lora::SF_MIN))) != 0) {
                    sf.push_back(i);
                }
            }
            return sf;
        }
    }

    struct Radio {
        //string      type;
        //float       rssi_offset;
        bool        tx_enable = false;
        bool        enable = false;
        uint32_t    freq = 0;
        uint32_t    tx_freq_min = 0;
        uint32_t    tx_freq_max = 0;
        uint8_t     sfmask;
        int         cad_threshold = 50;

        inline bool multisf() const {
            return 0 != (sfmask & (sfmask - 1));
        }
        lora::SF unisf() {
            lora::SF sf = lora::SF_MIN;
            for (; sf < lora::SF_MAX; sf=(lora::SF)((uint8_t)sf+1)) {
                if ((sfmask & (1 << (sf - lora::SF_MIN))) != 0) {
                    break;
                }
            }
            return sf;
        }
    protected:
        void load_disk(const Json &in) {
            std::list<int> llsf;
            enable = jsons::getdefault(in, "enable", true);
            freq = jsons::getdefault(in, "freq", 0u);
            cad_threshold = jsons::getdefault(in, "cad_threshold", 50);
            jsons::getlist(in, "sf", llsf);
            sfmask = priv::to_sfmask(llsf);
            tx_enable = jsons::getdefault(in, "tx_enable", false);
            tx_freq_min = jsons::getdefault(in, "tx_freq_min", 0);
            tx_freq_max = jsons::getdefault(in, "tx_freq_max", 0);
        }
        void save_disk(Json &out) {
            out["enable"] = enable;
            out["freq"] = freq;
            out["cad_threshold"] = cad_threshold;
            out["sf"] = priv::from_sfmask(sfmask);
            out["tx_enable"] = tx_enable;
            if (tx_enable) {
                out["tx_freq_min"] = tx_freq_min;
                out["tx_freq_max"] = tx_freq_max;
            }
        }
        ErrStatus load_api(const Json &in) {
            std::list<int> llsf;
            jsons::optional(in, "enable", enable);
            jsons::optional(in, "freq", freq);
            jsons::optional(in, "cad_threshold", cad_threshold);
            if (jsons::getlist(in, "sf", llsf)) {
                sfmask = priv::to_sfmask(llsf);
            }
            jsons::optional(in, "tx", tx_enable);
            jsons::optional(in, "txmin", tx_freq_min);
            jsons::optional(in, "txmax", tx_freq_max);
            return ERR_OK;
        }
        void save_api(Json &out) {
            out["enable"] = enable;
            out["freq"] = freq;
            out["sf"] = priv::from_sfmask(sfmask);
            out["cad_threshold"] = cad_threshold;
            if (tx_enable) {
                out["tx"] = true;
                out["txmin"] = tx_freq_min;
                out["txmax"] = tx_freq_max;
            } else {
                out["tx"] = false;
            }
        }
        friend class Config;
    };

    struct Global {
        Radio   radio0;
        Radio   radio1;
    };

}

#endif //AICAST_BACKHAUL_CONFIG_GLOBAL_H
