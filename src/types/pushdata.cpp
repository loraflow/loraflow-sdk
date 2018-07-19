//
// Created by Thinkpad on 2017/11/16.
//

#include <lang/lang-base64.h>
#include <types/pushdata.h>

namespace haul {


    void PushData::fromDirect(const lgw_pkt_rx_s * const lgw_rxs, int nb_pkt) {
        int size = 0;
        for (int i=0; i<nb_pkt; i++) {
            size += (uint8_t)(lgw_rxs+i)->size;
        }
        rawbuf = new uint8_t[size+nb_pkt+1];
        uint8_t * p = rawbuf;
        for (int i=0; i<nb_pkt; i++) {
            auto &rx = _rxs[i];
            auto &_info = rx.info;
            const lgw_pkt_rx_s * rxpkt = lgw_rxs+i;
            _info.time_us = rxpkt->count_us;
            _info.freq = rxpkt->freq_hz;
            _info.rssi = rxpkt->rssi;
            _info.snr10 = rxpkt->snr*10;
            _info.if_chain = rxpkt->if_chain;
            _info.rf_chain = rxpkt->rf_chain;
            _info.modu = translate::to_lora_modu(rxpkt->modulation);
            if (_info.modu == lora::MODU_LORA) {
                _info.dr.sf = translate::to_lora_sf(rxpkt->datarate);
            } else {
                _info.dr.bitrate = rxpkt->datarate;
            }
            _info.dr.bw = translate::to_lora_bw(rxpkt->bandwidth);
            _info.cr = translate::to_lora_cr(rxpkt->coderate);
            _info.status = translate::to_lora_crcstatus(rxpkt->status);
            rx.size = lgw_rxs->size;
            rx.payload = p;
            memcpy(p, lgw_rxs->payload, rx.size);
            p += rx.size;
        }
        _nbpkt = nb_pkt;
    }

    void PushData::fromRxInd(const RxInd &ind) {
        const auto& N = ind.data.len;
        rawbuf = new uint8_t[N];
        _rxs[0].info = ind.info;
        _rxs[0].size = N;
        _rxs[0].payload = rawbuf;
        memcpy(rawbuf, ind.data.bin, N);
        _nbpkt = 1;
    }
    bool PushData::fromJson(const Json &j) {
        Json jarray;
        if (!jsons::optional(j, "rxpk", jarray) || !jarray.is_array()) {
            return false;
        }
        const int N = jarray.size();
        rawbuf = new uint8_t[256*N];
        uint8_t *pbuf = rawbuf;
        for (int i=0; i<N; i++) {
            Json jpkt = jarray[i];
            double f32;
            string modu, datr;
            auto &pkt = _rxs[i];
            auto &info = pkt.info;
            jsons::optional(jpkt, "tmst", info.time_us);
            if (jsons::optional(jpkt, "freq", f32)) {
                info.freq = f32 * 1e6;
            }
            jsons::optional(jpkt, "modu", modu);
            if (modu == "LORA") {
                info.modu = lora::MODU_LORA;
                int bw, sf;
                if (!jsons::optional(jpkt, "datr", datr) || sscanf(datr.c_str(), "SF%dBW%d", &sf, &bw) != 2) {
                    return false;
                }
                if (!INRANGE(sf, 6, 12)) {
                    return false;
                }
                info.dr.sf = (lora::SF)sf;
                switch (bw) {
                    case 125: info.dr.bw = lora::BW_125K;break;
                    case 250: info.dr.bw = lora::BW_250K;break;
                    case 500: info.dr.bw = lora::BW_500K;break;
                    default: return false;
                }
            } else if (modu == "FSK") {
                info.modu = lora::MODU_FSK;
                if (!jsons::optional(jpkt, "datr", info.dr.bitrate) || !info.dr.bitrate) {
                    return false;
                }
            } else {
                return false;
            }
            int stat;
            if (jsons::optional(jpkt, "stat", stat)) {
                switch (stat) {
                    case 1: info.status = lora::CRCST_OK; break;
                    case -1: info.status = lora::CRCST_BAD; break;
                    case 0: info.status = lora::CRCST_NOCRC; break;
                    default: return false;
                }
            } else {
                info.status = lora::CRCST_UNDEFINED;
            }
            if (!jsons::optional(jpkt, "chan", info.if_chain)) {
                info.if_chain = 0;
            }
            if (!jsons::optional(jpkt, "rfch", info.rf_chain)) {
                info.rf_chain = 0;
            }
            string codr;
            jsons::optional(jpkt, "codr", codr);
            if (codr == "4/5") {
                info.cr = lora::CR_4_5;
            } else if (codr == "4/6" || codr == "2/3") {
                info.cr = lora::CR_4_6;
            } else if (codr == "4/7") {
                info.cr = lora::CR_4_7;
            } else if (codr == "4/8" || codr == "1/2") {
                info.cr = lora::CR_4_8;
            } else {
                return false;
            }
            int rssi;
            float snr;
            jsons::optional(jpkt, "rssi", rssi);
            jsons::optional(jpkt, "lsnr", snr);
            info.snr10 = snr * 10;
            info.rssi = rssi;
            string data;
            if (!jsons::optional(jpkt, "data", data)) {
                return false;
            }
            int nb = lang::base64::b64_to_bin(data.c_str(), data.size(), pbuf, 255);
            if (nb <= 0) {
                return false;
            }
            pkt.size = nb;
            pkt.payload = pbuf;
            pbuf += nb;
        };
        _nbpkt = N;
        return true;
    }
    int PushData::toBinary(uint8_t * const buff) {
        uint8_t *p = buff;
        for (int i=0; i<_nbpkt; i++) {
            auto &pkt = _rxs[i];
            auto &_info = pkt.info;

            if (_info.if_chain > 15 || _info.rf_chain>3 ||
                !INRANGE(_info.dr.sf, lora::SF6, lora::SF12) ||
                !INRANGE(_info.dr.bw, lora::BW_125K, lora::BW_500K)||
                !INRANGE(_info.cr, lora::CR_4_5, lora::CR_4_8)) {
                return 0;
            }
#ifdef ISSUE2014
            *p++ = 0;
#else
            if (i==0) {
                *p++ = 6;
                endian::put_u16(p, _rxmills);
                endian::put_u32(p, _rxmills>>16);
            } else {
                *p++ = 0;
            }
#endif
            endian::put_u32(p, _info.time_us);
            endian::put_u24(p, _info.freq/100);
            //Chan (4bits)+Rfch(2bit)+Stat(2bit)
            *p++ = (_info.if_chain << 4) | (_info.rf_chain << 2) | (_info.status);
            //Modu (1) + SF (3) + BW (2) + Codr (2)
            *p++ = (0<<7) | ((_info.dr.sf - lora::SF6) << 4) | ((_info.dr.bw - lora::BW_125K) << 2) | (_info.cr - lora::CR_4_5);
            //RSSI =Trim(RSSIreal+148，0，255)
            const int rssi = _info.rssi+148;
            *p++ = (uint8_t)(rssi < 0 ? 0 : rssi > 255 ? 255 : rssi);
            //SNR = Trim(SNRreal+20,0,40) × 255 / 40
            const int snr = ((float)(_info.snr10 + 200))/10.0 * 255.0 / 40.0;
            *p++ = (uint8_t)(snr < 0 ? 0 : snr > 255 ? 255 : snr);
            *p++ = pkt.size;
            memcpy(p, pkt.payload, pkt.size);
            p += pkt.size;
        }
        return p - buff;
    }
    int PushData::jsonize(uint8_t *buff) {
        try {
            int idx = sprintf((char *)buff, "{\"rxpk\":[");

            for (int i=0; i<_nbpkt; i++) {
                auto &pkt = _rxs[i];
                auto &_info = pkt.info;
                if (i>0) {
                    buff[idx++] = ',';
                }
                idx += sprintf((char *)buff+idx, "{\"tmst\":%u", _info.time_us);

                //TODO /* Packet RX time (GPS based), 37 useful chars */
                /* Packet concentrator channel, RF chain & RX frequency, 34-36 useful chars */
                idx += snprintf((char *)(buff + idx), TX_BUFF_SIZE-idx,
                                ",\"chan\":%1u,\"rfch\":%1u,\"freq\":%.6lf",
                                _info.if_chain, _info.rf_chain, ((double)_info.freq / 1e6));

                idx += jsonutil::put_crcstatus(buff + idx, (lora::CRCStatus)_info.status);

                switch (_info.modu) {
                    case lora::MODU_LORA:
                        idx += jsonutil::put_lora(buff + idx, _info.dr.sf, _info.dr.bw, _info.cr, _info.snr10);
                        break;
                    case lora::MODU_FSK:
                        idx += jsonutil::put_fsk(buff + idx, _info.dr.bitrate);
                        break;
                    default:
                        ERRORF("[up] received packet with unknown modulation\n");
                        throw jsonutil::BAD_MODU;
                }

                idx += sprintf((char*)buff + idx, ",\"rssi\":%d,\"size\":%u", _info.rssi, pkt.size);

                idx += jsonutil::put_data(buff + idx, pkt.payload, pkt.size);
                buff[idx++] = '"';
                buff[idx++] = '}';
            }
            buff[idx++] = ']'; /* add string terminator, for safety */
            buff[idx++] = '}'; /* add string terminator, for safety */
            buff[idx] = 0; /* add string terminator, for safety */
            return idx;
        } catch (jsonutil::Exception e) {
            return 0;
        }
    }

}