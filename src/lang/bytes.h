//
// Created by gl on 2018/6/25.
//

#ifndef _BYTES_H
#define _BYTES_H

#include <vector>
//#include <lang/lang.h>
//#include <lang/lang-endian.h>

using VarBytes = std::vector<uint8_t >;

namespace lang {
    struct Bytes {
        uint8_t *bin;
        int      len;

        Bytes():bin(nullptr),len(0) {}

        explicit Bytes(uint8_t *s):bin((uint8_t*)s),len(strlen((char*)s)) {}
        Bytes(uint8_t *s,int len):bin(s),len(len) {}

        Bytes Rewind() const {
            return {bin, 0};
        }
        inline bool Valid() const {
            return len > 0;
        }

        inline uint8_t operator[](int at) const {
            return bin[at];
        }
        inline uint8_t* operator+(int offset) {
            return bin + offset;
        }
        inline const uint8_t* operator+(int offset) const {
            return bin + offset;
        }

        inline Bytes& Copy(const Bytes &other) {
            len = MIN(len, other.len);
            memcpy(bin, other.bin, len);
            return *this;
        }
        inline uint8_t &Reserve() {
            return bin[len++];
        }
        inline uint8_t *Reserve(int nb) {
            auto re = bin +len;
            len += nb;
            return re;
        }
        inline void Put(uint8_t b) {
            bin[len++] = b;
        }
        inline void PutU16(uint16_t val) {
            endian::put_u16(bin+len, val);
            len += 2;
        }
        inline void PutU24(uint32_t val) {
            endian::put_u24(bin+len, val);
            len += 3;
        }
        inline void PutU32(uint32_t val) {
            endian::put_u32(bin+len, val);
            len += 4;
        }
        inline void PutU64(uint64_t val) {
            endian::put_u64(bin+len, val);
            len += 8;
        }
        inline uint8_t Get() {
            return bin[len++];
        }
        inline uint16_t GetU16() {
            auto re = endian::get_u16(bin+len);
            len += 2;
            return re;
        }
        inline uint32_t GetU24() {
            auto re = endian::get_u24(bin+len);
            len += 3;
            return re;
        }
        inline uint32_t GetU32() {
            auto re = endian::get_u32(bin+len);
            len += 4;
            return re;
        }
        inline uint64_t GetU64() {
            auto re = endian::get_u64(bin+len);
            len += 8;
            return re;
        }
        Option<uint64_t> IntoU64Hex() const {
            uint8_t tmp[sizeof(uint64_t)];
            if (len <= 0 || len > (int)sizeof(tmp)*2) {
                return {};
            }
            for (auto i=0; i < len; i++) {
                auto ch = static_cast<uint8_t>(_fromhexchar(bin[len - i - 1]));
                if (ch > 0xf) {
                    return {};
                }
                auto &ref = tmp[sizeof(tmp) - (i>>1) - 1];
                if ((i & 1) == 0) {
                    ref = ch;
                } else {
                    ref = (ch << 4) + ref;
                }
            }
            for (int i=(len+1)>>1; i<8; i++) {
                tmp[sizeof(tmp) - i - 1] = 0;
            }
            return Option<uint64_t >(endian::get_u64(tmp));
        }
        inline uint8_t *GetPtr() {
            return bin+len;
        }
        inline Bytes Slice(int off) {
            if (off >= 0 && off < len) {
                return Bytes{bin+off, len - off};
            } else {
                return {};
            }
        }
        inline void Put(const Bytes &other) {
            Put(other.bin, other.len);
        }
        inline void Put(const uint8_t*ptr, int nbytes) {
            memcpy(bin+len, ptr, static_cast<size_t>(nbytes));
            len += nbytes;
        }
        inline Bytes Offset(int offset) const {
            if (offset < len) {
                return {bin+offset, len-offset};
            }
            return {};
        }
        inline Bytes Limit(int limit) const {
            return {bin, MIN(len, limit)};
        }
        inline Bytes Split(int at) {
            Bytes re(bin, at);
            bin += at;
            len -= at;
            return re;
        }
        int Dump(uint8_t *buf) {
            if (len <= 0) {
                return 0;
            }
            memcpy(buf, this->bin, static_cast<size_t>(this->len));
            return len;
        }
        int DumpHex(uint8_t *buf) {
            for (int i=0; i<len; i++) {
                buf[2*i+0] = _tohexchar(bin[i] >> 4);
                buf[2*i+1] = _tohexchar(static_cast<uint8_t>(bin[i] & 0x0f));
            }
            return 2+len;
        }

        inline bool operator==(const Bytes &rhs) const {
            return len == rhs.len && memcmp(bin, rhs.bin, len) == 0;
        }
        inline bool operator==(ConstBytes &rhs) const {
            return len == rhs.right && memcmp(bin, rhs.left, len) == 0;
        }
        inline bool operator==(const char* rhs) const {
            return rhs[len] == '\0' && memcmp(bin, rhs, len) == 0;
        }
        inline bool operator==(const uint8_t rhs) const {
            return len == 1 && bin[0] == rhs;
        }
        inline bool operator!=(const Bytes &rhs) const {
            return !(rhs == *this);
        }
        Option<int> IntoInt() const {
            do {
                if (!len) {
                    break;
                }
                const auto sign = bin[0];
                int res = 0;
                int i = 0;
                if (sign == '-' || sign == '+') {
                    i = 1;
                }
                for (; i<len; i++) {
                    const uint8_t n = bin[i] - '0';
                    if (n > 9) {
                        break;
                    }
                    res = res * 10 + n;
                }
                return Option<int>(sign == '-' ? -res : res);
            } while(0);
            return {};
        }
        Option<uint32_t > IntoUint() const {
            do {
                if (!len) {
                    break;
                }
                uint32_t res = 0;
                for (int i=0; i<len; i++) {
                    const uint8_t n = bin[i] - '0';
                    if (n > 9) {
                        break;
                    }
                    res = res * 10 + n;
                }
                return Option<uint32_t >(res);
            } while(0);
            return {};
        }
        Option<double> IntoDouble() const {
            do {
                if (!len) {
                    break;
                }
                uint32_t res = 0;
                uint32_t scale = 0;
                for (int i=0; i<len; i++) {
                    const uint8_t n = bin[i] - '0';
                    if (n > 9) {
                        if (bin[i] == '.') {
                            if (!scale) {
                                scale = 1;
                                continue;
                            }
                        }
                        break;
                    }
                    res = res * 10 + n;
                    scale = scale * 10;
                }
                return Option<double >(scale>1 ? (double)res / (double)scale : (double)res);
            } while(0);
            return {};
        }
        int IntoBinary(uint8_t *out) const {
            if (0 != (len&1)) {
                return 0;
            }
            for (int i=0; i<len/2; i++) {
                auto x = (_fromhexchar(bin[i * 2]) << 4) + _fromhexchar(bin[i * 2 + 1]);
                if (x > 255) {
                    return 0;
                }
                out[i] = static_cast<uint8_t>(x);
            }
            return len>>1;
        }

        void Dump(VarBytes &re) {
            const auto N = re.size();
            re.resize(N + len);
            auto ptr = re.data() + N;
            for (auto i=0; i<len; i++) {
                ptr[i] = bin[i];
            }
        }
        VarBytes Dump(VarBytes &&re) {
            Dump(re);
            return re;
        }

    protected:
        static uint8_t _tohexchar(uint8_t ch) {
            return static_cast<uint8_t>(ch < 10 ? '0' + ch : ch + ('a' - 10));
        }
        static uint32_t _fromhexchar(uint8_t ch) {
            if (ch >= '0' && ch <= '9') {
                return ch - '0';
            }
            if (ch >= 'a' && ch <= 'f') {
                return static_cast<uint8_t>(ch - 'a' + 10);
            }
            if (ch >= 'A' && ch <= 'F') {
                return static_cast<uint8_t>(ch - 'A' + 10);
            }
            return 256;
        }
    };
}
#endif //NEW_BACKHAUL_BYTES_H
