//
// Created by Thinkpad on 2017/9/16.
//

#ifndef AICAST_LRMOD_LORA_IPC_H
#define AICAST_LRMOD_LORA_IPC_H

#include <cstdint>
#include <lang/lang-os.h>
#include <lang/lang-types.h>
#include <lang/lang-endian.h>

//
// BELOW ARE INTER-COPIED BETWEEN BACKHAUL & LRMOD PROJECT.
// TRY TO KEEP CONTENT INTER-CHANGABLE WITHOUT MODIFICATION.
//

/*
 * Wire = Frame, padding, Framing, padding, Frame, ... (padding will be zero or more 0xff)
 *
 * Frame = T-L-Data
 * +-----+-----+-----+-------~~~-------+-------+
 * | Tag | Ver | Len | Data            | Cksum |
 * +-----+-----+-----+-------~~~-------+-------+
 *
 * Tag = 1Bytes = MsgTag+LenMsb
 * +-----------------+----------------+
 * | LenMsb | AckReq | MsgTag         |
 * | 1bit   | 1bit   | 6bits          |
 * +-----------------+----------------+
 *
 * */

namespace lora {
    using lang::Buffer;
    using endian = lang::LittleEndian;
    using Frequency = uint32_t;
}

namespace lora {

#define IPC_HDR 3   //tag, ver, len

    enum Modu : uint8_t {MODU_UNDEFINED, MODU_LORA, MODU_FSK};

    enum CRCStatus : uint8_t {CRCST_UNDEFINED, CRCST_OK, CRCST_BAD, CRCST_NOCRC};

    enum Tag {TAG_SYNC=0, TAG_RXREQ, TAG_RXIND, TAG_TXREQ, TAG_KVOP, TAG_LOG, TAG_CADREQ};

    enum SF : uint8_t { SF_INVALID, SF6 = 6,
        SF7, SF8, SF9, SF10, SF11, SF12, SF_MIN = SF6 ,
        SF_MAX = SF12,
        SF7TO12MASK = 1<<(SF7-SF_MIN)|1<<(SF8-SF_MIN)|1<<(SF9-SF_MIN)|1<<(SF10-SF_MIN)|1<<(SF11-SF_MIN)|1<<(SF12-SF_MIN),
        SFALLMASK = 1<<(SF6-SF_MIN)|SF7TO12MASK,
    };

    enum BW : uint8_t { BW_INVALID, BW_125K = 7, BW_250K, BW_500K, BW_MIN = BW_125K };

    enum CR : uint8_t { CR_INVALID, CR_4_5 = 1, CR_4_6, CR_4_7, CR_4_8 };

    using Preamble = uint8_t;
    using Power = uint8_t;
    using SNR10_t = int16_t;
    using RSSI_t = int16_t;

    inline const char* crc_text(CRCStatus st) {
        return (st) == CRCST_OK ? "OK" : (st) == CRCST_BAD ? "BAD" : (st) == CRCST_NOCRC ? "NOCRC" : "";
    }
    inline int bw2num(uint8_t bw) {
        return (bw) == BW_125K ? 125 : (bw) == BW_250K ? 250 : (bw) == BW_500K ? 500 : 0;
    }
    inline BW num2bw(int bw) {
        return bw == 125 ? BW_125K : bw == 250 ? BW_250K : bw == 500 ? BW_500K : BW_INVALID;
    }

    inline const char* sfmask2text(uint8_t mask, char *buf) {
        if ((mask & SF7TO12MASK) == SF7TO12MASK) {
            return (mask & SFALLMASK) == SFALLMASK ? "ALLSF" : "SF7~SF12";
        } else {
            auto p = buf;
            *p = '\0';
            for (int i=0; mask && i <= SF_MAX - SF_MIN; mask >>=1, i++) {
                if (mask & 1) {
                    if (p != buf) {
                        *p++ = ',';
                    }
                    p += sprintf(p, "SF%d", SF_MIN + i);
                }
            }
            return buf;
        }
    }

    class IPCMsg {
        uint8_t * const     _raw;
        const int           _total;
        const int           _len;
    public:

        IPCMsg(uint8_t *ptr, int total):
                _raw(ptr),
                _total(total),
                _len(ptr[2] + ((ptr[0] & 0x80) << 1)) {}

        //
        //CAUTION -
        //  check if message is 'finished' before any other processing
        //

        inline bool finished() {
            return _total >= IPC_HDR && _len <= _total - IPC_HDR - 1;
        }
        inline bool overflow(int maxbufsize) {
            return _total >= IPC_HDR && _len > maxbufsize - IPC_HDR - 1;
        }
        inline uint8_t tag() {
            return _raw[0] & 0x1f;
        }
        inline uint8_t ver() {
            return _raw[1];
        }
        inline uint8_t cksum() {
            uint8_t c = 0xff;
            for (int i=0; i<_len+IPC_HDR+1; i++) {
                c ^= _raw[i];
            }
            return c;
        }
        inline uint8_t *data() {
            return _raw + IPC_HDR;
        }
        inline int datalen() {
            return _len;
        }
        template<class T> inline void get(uint8_t *&data, T &len) {
            data = _raw + IPC_HDR;
            len = _len;
        }
        inline uint8_t *raw() {
            return _raw;
        }
        inline int rawlen() {
            return _len + IPC_HDR + 1;
        }
        inline uint8_t *next() {
            return _raw + IPC_HDR + _len + 1;
        }
    };

    class SyncMessage {
        uint8_t _raw[IPC_HDR + 1 + 4 + 1];  //header, act, timestamp, cksum
        uint8_t * const _buf;
    public:
        enum Act {SYNC, RESET, OK};
        //for outgoing
        SyncMessage():_buf(_raw) {
            _buf[0] = TAG_SYNC;  //tag
            _buf[1] = 0;         //ver
            _buf[2] = 5;         //len
        }
        SyncMessage(uint8_t *rx, int nb):_buf(rx) {
        }

        //
        //for outgoing sync message
        //
        inline void set_ver(uint8_t ver) {
            _buf[1] = ver;
        }
        inline uint8_t *update(Act act) {
            _buf[IPC_HDR + 0] = act;
            endian::put_u32p(_buf + IPC_HDR + 1, GET_MICROS());
            _buf[IPC_HDR + 1 + 4] = 0xff ^ _buf[0] ^ _buf[1] ^ _buf[2] ^ _buf[3] ^
                    _buf[4] ^ _buf[5] ^ _buf[6] ^ _buf[7];
            return _buf;
        }
        inline int dump(Act act, uint8_t *out) {
            _buf[IPC_HDR + 0] = act;
            *((uint32_t *)out) = *((uint32_t*)_buf);
            endian::put_u32p(out + IPC_HDR + 1, GET_MICROS());
            out[IPC_HDR + 1 + 4] = 0xff ^ out[0] ^ out[1] ^ out[2] ^ out[3] ^
                    out[4] ^ out[5] ^ out[6] ^ out[7];
            return size();
        }
        inline int size() {
            return sizeof(_raw);
        }

        //
        //for received sync message
        //
        inline uint8_t get_ver() {
            return _buf[1];
        }
        inline Act get_act() {
            return (Act)_buf[IPC_HDR + 0];
        }
        inline uint32_t get_time() {
            return endian::get_u32p(_buf+IPC_HDR+1);
        }
    };

    class LORA {
    public:
        static const char* toString(BW bw) {
            return bw == BW_125K ? "125K" : bw == BW_250K ? "250K" : bw == BW_500K ? "500K" : "?";
        }
    };

    struct _utils {
    protected:
        inline uint8_t _encode_clsmod(uint8_t class_, Modu modu) const {
            return ((modu) << 4) | ((class_ - 'A') & 0x0f);
        }
        template <class Class_, class Modu> inline void _decode_clsmod(uint8_t clsmod, Class_ &class_, Modu &modu) {
            modu = (Modu)(clsmod>>4);
            class_ = (Class_)(clsmod&0x0f) + 'A';
        }
        inline uint8_t _encode_modustatbw(Modu modu, CRCStatus st, BW bw) const {
            return (modu << 6) | (st << 4) | (bw);
        }
        inline void _decode_modustatbw(uint8_t mdst, Modu &modu, CRCStatus &st, BW &bw) {
            modu = (Modu)((mdst >> 6) & 3);
            st = (CRCStatus)((mdst >> 4) & 3);
            bw = (BW)(mdst & 0xf);
        }
        inline uint8_t _encode_bwsf(BW bw, SF sf) const {
            return ((bw - BW_MIN) << 4) | (sf - SF_MIN);
        }
        inline void _decode_bwsf(uint8_t bwsf, BW &bw, SF &sf) {
            bw = (BW)((bwsf>>4)+BW_MIN);
            sf = (SF)((bwsf&0x0f) + (uint8_t)SF_MIN);
        }
        inline uint8_t _encode_sf(SF min, SF max) const {
            return ((min - SF_MIN) << 4) | (max - SF_MIN);
        }
        template <class SF> inline void _decode_sf(uint8_t bwsf, SF &min, SF &max) {
            min = (SF)((bwsf>>4)+SF_MIN);
            max = (SF)(bwsf&0x0f)+SF_MIN;
        }
        inline uint8_t _encode_sfcr(SF sf, CR cr) const {
            return ((sf - SF_MIN) << 4) | cr;
        }
        template <class SF, class CR> inline void _decode_sfcr(uint8_t bwsf, SF &sf, CR &cr) {
            sf = (SF)((bwsf>>4)+SF_MIN);
            cr = (CR)(bwsf&0x0f);
        }
    };

    struct Datarate {
        Modu        modu = MODU_LORA;
        BW          bw;
        union {
            SF          sf;
            uint16_t    bitrate;
        };
    };

    struct TxReq : public _utils {
        uint8_t *   data;
        uint32_t    time_us;
        Frequency   freq;
        Datarate    dr;
        Preamble    preamble;
        Power       power;
        Modu        modu;
        uint8_t     rf_chain;
        CR          cr;
        uint8_t     class_;  //class A, B, C
        bool        crc;
        //bool        invertiq;        //true=invert(gateway tx), false=no invert(node tx)
        bool        haspower = false;
        uint8_t     nbytes;

        static const int MINBYTES = 11;
        /*
         * +---------+-----------+----------+-------+---------+-----------+--------+--------~~~+
         * | CLMD(1) | Time(3)?   | FREQ(3)  | PR(1) | BWSF(1) | PICRCC(1) | PW(1?) | Data ...  |
         * +---------+-----------+----------+-------+---------+-----------+--------+--------~~~+
         * */
        inline Tag get_tag() {
            return TAG_TXREQ;
        }
        int32_t estimate_size() {
            return MINBYTES + nbytes;
        }
        inline void set_power(uint8_t p) {
            power = p;
            haspower = true;
        }
        bool serialize(Buffer *buf, uint8_t &cksum) {
            if (MINBYTES + nbytes > buf->cap - buf->idx) {
                return false;  //overflow!
            }
            uint8_t *ptr = buf->buffer+buf->idx;
            uint8_t *pstart = ptr;
            *ptr++ = _encode_clsmod(class_, modu);
            if (class_ != 'C') {
                endian::put_u32(ptr, time_us);
            }
            endian::put_u24(ptr, freq / 100);
            *ptr++ = preamble;
            *ptr++ = _encode_bwsf(dr.bw, dr.sf);
            uint8_t picrcc = cr;
            if (haspower) {
                picrcc |= 1<<7;
            }
            //if (invertiq) {
            //    picrcc |= 1<<6;
            //}
            if (crc) {
                picrcc |= 1<<5;
            }
            *ptr++ = picrcc;
            if (haspower) {
                *ptr++ = power;
            }
            while (pstart<ptr) {
                cksum ^= *pstart++;
            }
            for (int i=0; i<nbytes; i++) {
                cksum ^= (ptr[i] = data[i]);
            }
            buf->idx = ptr - buf->buffer + nbytes;
            return true;
        }
        bool deserialize(const uint8_t *ptr, const int size) {
            if (size < MINBYTES-4) {
                return false;
            }
            const uint8_t *const base = ptr;
            _decode_clsmod(*ptr++, class_, modu);
            if (class_ != 'C') {
                if (size < MINBYTES) {
                    return false;
                }
                time_us = endian::get_u32(ptr);
            }
            freq = endian::get_u24(ptr) * 100;
            preamble = *ptr++;
            _decode_bwsf(*ptr++, dr.bw, dr.sf);
            uint8_t picrcc = *ptr++;
            cr = (CR)(picrcc&0x0f);
            haspower = (picrcc & (1<<7)) != 0;
            //invertiq = (picrcc & (1<<6)) != 0;
            crc      = (picrcc & (1<<5)) != 0;
            if (haspower) {
                power = *ptr++;
            }
            nbytes = size - (ptr - base);
            data = const_cast<uint8_t *>(ptr);
            ptr += nbytes;
            return true;
        }
    };

    struct CadReq : public _utils {
        Frequency   freq;
        uint8_t     sfmask;
        BW          bw;
        static const int MINBYTES = 5;
    public:
        /*
         * +----------------+----------+----------+
         * | FREQ(3)        | sfmask(1)| Bw(1)    |
         * +----------------+----------+----------+
         * */
        inline Tag get_tag() {
            return TAG_CADREQ;
        }
        int32_t estimate_size() {
            return MINBYTES;
        }
        bool serialize(Buffer *buf, uint8_t &cksum) {
            if (MINBYTES > buf->cap - buf->idx) {
                return false;  //overflow!
            }
            int pos = buf->idx;
            uint8_t  *ptr = buf->buffer+buf->idx;
            endian::put_u24(ptr, freq / 100);
            *ptr++ = sfmask;
            *ptr++ = bw;
            buf->idx = ptr - buf->buffer;
            for (int i=pos; i<buf->idx; i++) {
                cksum ^= buf->buffer[i];
            }
            return true;
        }
        bool deserialize(const uint8_t *ptr, int size) {
            if (MINBYTES > size) {
                return false;  //underrun!
            }
            freq = endian::get_u24(ptr) * 100;
            sfmask = *ptr++;
            bw = (BW)*ptr++;
            return true;
        }
        SF sfseek(const SF from) const {
            for (SF sf = from; sf <= SF_MAX; sf=(SF)(sf+1)) {
                if ((sfmask & (1 << (sf - SF_MIN))) != 0) {
                    return sf;
                }
            }
            for (SF sf = SF_MIN; sf < from; sf=(SF)((int)sf+1)) {
                if ((sfmask & (1 << (sf - SF_MIN))) != 0) {
                    return sf;
                }
            }
            return from;
        }
        void set_sf(SF sf) {
            sfmask = 1 << (sf - SF_MIN);
        }
    };

    struct RxReq : public _utils {
        Frequency   freq;
        SF          sf;
        BW          bw;
        uint8_t     symbto;     //symbol timeout
        bool        continuous;
        bool        repeat;
        static const int MINBYTES = 6;
    public:
        inline bool is_lora() const {
            return sf != SF_INVALID;
        }
        inline Tag get_tag() {
            return TAG_RXREQ;
        }
        int32_t estimate_size() {
            return MINBYTES;
        }
        bool serialize(Buffer *buf, uint8_t &cksum) {
            if (MINBYTES > buf->cap - buf->idx) {
                return false;  //overflow!
            }
            int pos = buf->idx;
            uint8_t  *ptr = buf->buffer+buf->idx;
            endian::put_u24(ptr, freq / 100);
            *ptr++ = symbto;
            *ptr++ = sf;
            uint8_t bwicr = bw;
            if (continuous) {
                bwicr |= 1 << 6;
            }
            if (repeat) {
                bwicr |= 1 << 5;
            }
            *ptr++ = bwicr;
            buf->idx = ptr - buf->buffer;
            for (int i=pos; i<buf->idx; i++) {
                cksum ^= buf->buffer[i];
            }
            return true;
        }
        bool deserialize(const uint8_t *ptr, int size) {
            if (MINBYTES > size) {
                return false;  //underrun!
            }
            freq = endian::get_u24(ptr) * 100;
            symbto = *ptr++;
            sf = (SF)*ptr++;
            uint8_t bwicr = *ptr++;
            //invertiq = (bwicr & (1 << 7)) != 0;
            continuous = (bwicr & (1 << 6)) != 0;
            repeat = (bwicr & (1 << 5)) != 0;
            bw = (BW)(bwicr&0x0f);
            return true;
        }
    };

    struct RxInfo : public _utils {
        uint32_t    time_us;
        Frequency   freq;
        Datarate    dr;
        Modu        modu = MODU_LORA;
        CRCStatus   status;         //crc status
        CR          cr;
        uint8_t     if_chain;       /*!> by which IF chain was packet received */
        uint8_t     rf_chain;       /*!> through which RF chain the packet was received */
        SNR10_t     snr10;
        RSSI_t      rssi;
        static const int MINBYTES = 14;
        /*
         * lora
         * +---------+----------------+---------+-----------+--------+---------+
         * | TIME(4) | FREQ(3)        | RSSI(2) | MDSTBW(2) | SNR(2) | SFCR(1) |
         * +---------+----------------+---------+-----------+--------+---------+
         * fsk
         * +---------+----------------+---------+-----------+---------+
         * | TIME(4) | FREQ(3)        | RSSI(2) | MDSTBW(2) | BitR(2) |
         * +---------+----------------+---------+-----------+---------+
         * */
        int32_t estimate_size() {
            return MINBYTES;
        }
        bool serialize(Buffer *buf) const {
            if (MINBYTES > buf->cap - buf->idx) {
                return false;  //overflow!
            }
            uint8_t *ptr = buf->buffer+buf->idx;
            endian::put_u32(ptr, time_us);
            endian::put_u24(ptr, freq / 100);
            endian::put_u16(ptr, rssi);
            *ptr++ = _encode_modustatbw(modu, status, dr.bw);
            if (modu == MODU_LORA) {
                endian::put_u16(ptr, snr10);
                *ptr++ = _encode_sfcr(dr.sf, cr);
            } else {
                endian::put_u16(ptr, dr.bitrate);
            }
            buf->idx = ptr - buf->buffer;
            return true;
        }
        bool deserialize(const uint8_t *&ptr, int size) {
            if (MINBYTES > size) {
                return false;  //underrun!
            }
            time_us = endian::get_u32(ptr);
            freq = endian::get_u24(ptr) * 100;
            rssi = endian::get_u16(ptr);
            _decode_modustatbw(*ptr++, modu, status, dr.bw);
            if (modu == MODU_LORA) {
                snr10 = endian::get_u16(ptr);
                _decode_sfcr(*ptr++, dr.sf, cr);
            } else {
                dr.bitrate = endian::get_u16(ptr);
            }
            return true;
        }
    };

    struct VarBytes {
        uint8_t     *bin = 0;
        uint8_t     len = 0;
    };
    struct RxInd {
        RxInfo      info;
        VarBytes    data;
        /*
         * +---------------------+----------------~~~+
         * |  RXINFO(...)        | DATA(...)         |
         * +---------------------+----------------~~~+
         * */
        inline Tag get_tag() {
            return TAG_RXIND;
        }
        int32_t estimate_size() {
            return data.len + info.estimate_size();
        }
        bool serialize(Buffer *buf, uint8_t &cksum) {
            int pos = buf->idx;
            if (!info.serialize(buf)) {
                return false;
            }
            if (data.len > buf->cap - buf->idx) {
                return false;
            }
            for (int i=pos; i<buf->idx; i++) {
                cksum ^= buf->buffer[i];
            }
            for (int i=0; i<data.len; i++) {
                cksum ^= (buf->buffer[buf->idx+i] = data.bin[i]);
            }
            buf->idx += data.len;
            return true;
        }
        bool deserialize(const uint8_t *ptr, int size) {
            const uint8_t *saved = ptr;
            if (!info.deserialize(ptr, size)) {
                return false;
            }
            data.bin = const_cast<uint8_t *>(ptr);
            ptr = saved + size;
            data.len = ptr - data.bin;
            return true;
        }
        inline bool is_ok() {
            return info.status == CRCST_OK;
        }
    };

    struct KVOp {
#define KVOP_OP_SET             "s"
#define KVOP_OP_GET             "g"
#define KVOP_OP_ACK             "Y"
#define KVOP_OP_NACK            "X"
#define KVOP_KEY_THRESHOLD      "threshold"
#define KVOP_KEY_UUID           "i"

        const char * op = nullptr;
        const char * key = nullptr;
        const char * val = nullptr;
        /*
        * +----~~~--+---~~~---+--~~~~---+
        * |  OP     | Key     | Val     |
        * +----~~~--+---~~~---+--~~~~---+
        * */

        inline Tag get_tag() {
            return TAG_KVOP;
        }
        int32_t estimate_size() {
            return strlen(op) + strlen(key) + strlen(val) + 3;
        }
        bool serialize(Buffer *buf, uint8_t &cksum) {
            int pos = buf->idx;
            const char * ss[] = {op, key, val};
            const size_t nn[] = {strlen((char*)op), strlen((char*)key), strlen((char*)val)};
            for (int i=0; i< (int)(sizeof(nn)/ sizeof(nn[0])); i++) {
                const char *str = ss[i];
                for (size_t j=0; j<nn[i]; j++) {
                    buf->buffer[pos++] = str[j];
                    cksum ^= str[j];
                }
                buf->buffer[pos++] = '\0';
            }
            buf->idx = pos;
            return true;
        }
        bool deserialize(const uint8_t *ptr, int size) {
            const char **ss[] = {&op, &key, &val};
            int n = 0;
            const char *mark = reinterpret_cast<const char *>(ptr);
            for (int i=0; i<size && n < (int)(sizeof(ss)/ sizeof(ss[0])); i++) {
                if (!ptr[i]) {
                    *ss[n++] = mark;
                    mark = reinterpret_cast<const char *>(ptr + i + 1);
                }
            }
            return true;
        }
    };

    namespace codec {

        template<typename T, int32_t(T::*EST)() = &T::estimate_size>
        inline int estimate(T *arg) {
            return 4 + (arg->*EST)();
        };

        template<typename M,
                Tag(M::*GT)() = &M::get_tag,
                bool(M::*SER)(Buffer *, uint8_t&) = &M::serialize>
        int encode(uint8_t *ptr, int max, M *arg, uint8_t ver) {
            Buffer buf(ptr, max);
            buf.idx = 3;
            uint8_t cksum = 0xff;
            if (!(arg->*SER)(&buf, cksum)) {
                return -1;
            }
            ptr[0] = (arg->*GT)();      //tag
            ptr[1] = ver;               //ver
            ptr[2] = buf.idx - 3;       //len
            if (buf.idx - 3 > 255) {
                ptr[0] |= 0x80;
            }
            ptr[buf.idx++] = cksum ^ ptr[0] ^ ptr[1] ^ ptr[2];
            return buf.idx;
        }
    }

}

#endif //AICAST_LRMOD_LORA_IPC_H
