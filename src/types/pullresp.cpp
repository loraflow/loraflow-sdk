//
// Created by Thinkpad on 2017/11/16.
//
#include <types/pullresp.h>
#include <lang/lang-endian.h>
#include <types/jsonutils.h>
#include <lang/lang-base64.h>

namespace haul {

    using endian = lang::LittleEndian;

    int PullResp::jsonize(uint8_t *const buff) {
        int nb = ::sprintf((char*)buff, "{\"txpk\":{");
        if (_tx.class_ == 'C') {
            nb += ::sprintf((char*)buff+nb, "\"imme\":true,");
        } else {
            nb += ::sprintf((char*)buff+nb, "\"imme\":false,");
            if (_tx.class_ == 'B') {
                nb += ::sprintf((char*)buff+nb, "\"tmms\":%u,", _tx.time_us);
            } else {
                nb += ::sprintf((char*)buff+nb, "\"tmst\":%u,", _tx.time_us);
            }
        }
        nb += ::sprintf((char*)buff+nb, "\"freq\":%d.%06d,", _tx.freq/1000000, _tx.freq%1000000);
        nb += ::sprintf((char*)buff+nb, "\"rfch\":%d,", _tx.rf_chain);
        nb += ::sprintf((char*)buff+nb, "\"powe\":%d,", _tx.haspower ? _tx.power : 14);
        if (_tx.modu == lora::MODU_FSK) {
            nb += ::sprintf((char*)buff+nb, "\"modu\":\"FSK\",");
            nb += ::sprintf((char*)buff+nb, "\"datr\":%d,", _tx.dr.bitrate);
        } else {
            nb += ::sprintf((char*)buff+nb, "\"modu\":\"LORA\",");
            nb += ::sprintf((char*)buff+nb, "\"datr\":\"SF%dBW%d\",", _tx.dr.sf, lora::bw2num(_tx.dr.bw));
        }
        nb += ::sprintf((char*)buff+nb, "\"codr\":\"4/%d\",", 4+_tx.cr);
        nb += ::sprintf((char*)buff+nb, "\"ipol\":true,");
        nb += ::sprintf((char*)buff+nb, "\"prea\":%d,", _tx.preamble);
        nb += ::sprintf((char*)buff+nb, "\"size\":%d,", _tx.nbytes);
        nb += ::sprintf((char*)buff+nb, "\"data\":\"");

        nb += lang::base64::bin_to_b64(_tx.data, _tx.nbytes, (char*)buff+nb, 255*8/6);

        buff[nb++] = '\"';
        buff[nb++] = '}';
        buff[nb++] = '}';
        buff[nb] = '\0';
        return nb;
    }

    void PullResp::fromTxReq(const TxReq &tx) {
        memcpy(_buf, tx.data, tx.nbytes);
        _tx = tx;
        _tx.data = _buf;
        _longdelay = _tx.class_ != 'C' && (int)(_tx.time_us - os::micros()) >= 1000000;
    }
    bool PullResp::fromBinary(const uint8_t *const buf, int size) {
        if (size <= 10) {
            return false;
        }
        const uint8_t *ptr = buf;
        const uint8_t byte0 = *ptr++;
        _tx.modu = lora::MODU_LORA;
        _tx.preamble = ((byte0 >>2) & 0x0f) + 5;
        if (_tx.preamble == 5) {
            _tx.preamble = CONFIG_DEFAULT_PREAMBLE;
        }
        switch (byte0 & 3) {
            case 0:
                _tx.class_ = 'C';
                break;
            case 1:
                _tx.class_ = 'A';
                _tx.time_us = endian::get_u32(ptr);
                break;
            case 2:
                _tx.class_ = 'B';
                DEBUGF("Unsupported class B");
                return false;
            default:
                return false;
        }
        _tx.freq = endian::get_u24(ptr) * 100;
        //Rfch(2)+Ipol(1)+Power(5)
        const uint8_t rfippo = *ptr++;
        _tx.power = rfippo & 0x1f;
#ifdef CONFIG_FORCE_RADIO0TX
        _tx.rf_chain = 0;
#else
        _tx.rf_chain = rfippo >> 6;
#endif
        //Modu (1) + SF (3) + BW (2) + Codr (2)
        const uint8_t sfbwcr = *ptr++;
        _tx.dr.sf = lora::SF((uint8_t)(lora::SF6) + ((sfbwcr >> 4) & 7));
        _tx.dr.bw = lora::BW((uint8_t)lora::BW_125K + ((sfbwcr >> 2) & 3));
        _tx.cr = lora::CR((uint8_t)(lora::CR_4_5) + ((sfbwcr) & 3));
        //remains are data in raw format
        _tx.nbytes = size - (ptr - buf);
        memcpy(_buf, ptr, _tx.nbytes);
        _tx.data = _buf;
        _tx.haspower = _tx.power != 0;
        return true;
    }
    bool PullResp::fromText(const uint8_t *text, int size) {
        try {
            Json txpk = Json::parse(text)["txpk"];
            bool sent_immediate = jsons::getdefault(txpk, "imme", false);
            if (!sent_immediate) {
                uint32_t tmst;
                if (jsons::optional(txpk, "tmst", tmst)) {
                    _tx.time_us = tmst;
                    _tx.class_ = 'A';
                } else {
                    _tx.class_ = 'B';
                    _tx.time_us = txpk["tmms"];
                    //TODO translate gps time to local time!
                    DEBUGF("Unsupported class B");
                    return false;
                }
                _longdelay = (int)(_tx.time_us - os::micros()) >= 1000000;
            } else {
                _tx.class_ = 'C';
            }
            _tx.crc = !jsons::getdefault(txpk, "ncrc", false);
            _tx.freq = ((uint32_t)(1000000 * (float)txpk["freq"]) + 9) / 10 * 10;  //round up to multiple of 10
            _tx.rf_chain = txpk["rfch"];

            _tx.haspower = jsons::optional(txpk, "powe", _tx.power);

            string modu = txpk["modu"];
            if (modu == "LORA") {
                _tx.modu = lora::MODU_LORA;

                /* Parse Lora spreading-factor and modulation bandwidth (mandatory) */
                string datr = txpk["datr"];
                int sf, bw;
                if (2 != sscanf(datr.c_str(), "SF%dBW%d", &sf, &bw)) {
                    throw ("WARNING: [down] format error in \"txpk.datr\", TX aborted\n");
                }
                if (!INRANGE(sf, lora::SF7, lora::SF12)) {
                    throw "invalid SF";
                }
                _tx.dr.sf = (lora::SF)sf;

                switch (bw) {
                    case 125: _tx.dr.bw = lora::BW_125K; break;
                    case 250: _tx.dr.bw = lora::BW_250K; break;
                    case 500: _tx.dr.bw = lora::BW_500K; break;
                    default:
                        throw ("WARNING: [down] format error in \"txpk.datr\", invalid BW, TX aborted\n");
                }

                /* Parse ECC coding rate (optional field) */
                string codr = txpk["codr"];
                if      (codr == "4/5") _tx.cr = lora::CR_4_5;
                else if (codr == "4/6") _tx.cr = lora::CR_4_6;
                else if (codr == "2/3") _tx.cr = lora::CR_4_6;
                else if (codr == "4/7") _tx.cr = lora::CR_4_7;
                else if (codr == "4/8") _tx.cr = lora::CR_4_8;
                else if (codr == "1/2") _tx.cr = lora::CR_4_8;
                else {
                    DEBUGF ("WARNING: [down] format error in \"txpk.codr\", TX aborted\n");
                    return false;
                }

                //_tx.invertiq = txpk["ipol"];   //for gateway tx, ipol is true

                _tx.preamble = jsons::getdefault(txpk, "prea", 8);

            } else if (modu == "FSK") {
                DEBUGF("TODO support FSK"); //TODO fsk
                return false;
            } else {
                DEBUGF ("WARNING: [down] invalid modulation in \"txpk.modu\", TX aborted\n");
                return false;
            }
            _tx.nbytes = txpk["size"];
            string payload = txpk["data"];
            int decoded_size = lang::base64::b64_to_bin(payload.c_str(), payload.size(), _buf, sizeof(_buf));
            _tx.data = _buf;
            if (decoded_size != _tx.nbytes) {
                WARNF("decode size mismatch {} ~ {}", decoded_size, _tx.nbytes);
            }
            return true;
        } catch (...) {
            WARNF("bad packet {}", text);
            return false;
        }
    }

}