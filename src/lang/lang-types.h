//
// Created by Thinkpad on 2017/9/19.
//

#ifndef AICAST_BACKHAUL_LANG_TYPES_H
#define AICAST_BACKHAUL_LANG_TYPES_H

#include <ctime>
#include <cstdint>
#include <lang/lang-os.h>
#include <lang/lang-strings.h>
#include "lang-endian.h"

namespace lang {

    using namespace std;

    class Timeout {
        time_t _since;
        const time_t _dura;
    public:
        Timeout(time_t seconds):_dura(seconds) {
            refresh();
        }
        void refresh() {
            _since = os::epoch();
        }
        bool expired() const {
            return elapsed() >= _dura;
        }
        time_t elapsed() const {
            return os::epoch() - _since;
        }
    };

    class Hex {
        const uint8_t *_ptr;
        const int N;
    public:
        Hex(void*ptr, int nb):_ptr((uint8_t*)ptr),N(nb) {}


        string to_string() const {
            return strings::hex2str(_ptr, N);
        }

        friend ostream& operator<<(ostream& os, const Hex& hex) {
            return os << hex.to_string();
        }
    };

    class Float10 {
        const int _val;
    public:
        Float10(int val):_val(val) {}

        friend ostream& operator<<(ostream& os, const Float10& hex) {
            char buf[20];
            if (hex._val < 0) {
                int snr10 = -hex._val;
                sprintf(buf, "-%d.%d", snr10/10, snr10%10);
            } else {
                int snr10 = hex._val;
                sprintf(buf, "%d.%d", snr10/10, snr10%10);
            }
            return os << buf;
        }
    };

    class SockAddr {
        const struct sockaddr_in _addr;
    public:
        SockAddr(struct sockaddr_in addr):_addr(addr) {}
        friend ostream& operator<<(ostream& os, const SockAddr& sa) {
            const uint8_t *p = (const uint8_t*)&sa._addr.sin_addr.s_addr;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            os << (int)p[0] << '.' << (int)p[1] << '.' << (int)p[2] << '.' << (int)p[3];
#else
            os << (int)p[3] << '.' << (int)p[2] << '.' << (int)p[1] << '.' << (int)p[0];
#endif
            os << ':' << sa._addr.sin_port;
            return os;
        }
    };

    class MoteAddr {
        const uint8_t *_addr;
    public:
        MoteAddr(uint8_t *p):_addr(p) {}
        friend ostream& operator<<(ostream& os, const MoteAddr& sa) {
            auto p = sa._addr;
            os << (int)p[0] << '.' << (int)p[1] << '.' << (int)p[2] << '.' << (int)p[3];
            return os;
        }
    };

    class EUI64 {
        uint8_t _bin[8] __attribute__ ((aligned (4)));   //LoRaWAN using little endian
    public:
        EUI64() {
            *((uint32_t*)(_bin+0)) = 0;
            *((uint32_t*)(_bin+4)) = 0;
        }
        inline uint32_t hash_code() const {
            return (*((uint32_t*)_bin)) * 4144924453 + (*((uint32_t*)(_bin + 4))) * 1211853749;
        }
        inline void to(uint8_t *p) const {
            p[0] = _bin[0];
            p[1] = _bin[1];
            p[2] = _bin[2];
            p[3] = _bin[3];
            p[4] = _bin[4];
            p[5] = _bin[5];
            p[6] = _bin[6];
            p[7] = _bin[7];
        }
        inline void from(const uint8_t *p) {
            _bin[0] = p[0];
            _bin[1] = p[1];
            _bin[2] = p[2];
            _bin[3] = p[3];
            _bin[4] = p[4];
            _bin[5] = p[5];
            _bin[6] = p[6];
            _bin[7] = p[7];
        }

        string to_string() const {
            return strings::hex2str(_bin, sizeof(_bin), true);
        }

        bool from_string(const string val) {
            return strings::str2hex(val, _bin, sizeof(_bin), true) == sizeof(_bin);
        }

        inline bool operator <(const EUI64& rhs) const
        {
            int x = (*((uint32_t*)_bin)) - (*((uint32_t*)rhs._bin));
            if (x == 0) {
                x = (*((uint32_t*)(_bin+4))) - (*((uint32_t*)(rhs._bin+4)));
            }
            return x < 0;
        }

        inline bool operator ==(const EUI64& rhs) const
        {
            return (*((uint32_t*)_bin)) == (*((uint32_t*)rhs._bin)) &&
                    (*((uint32_t*)(_bin+4))) == (*((uint32_t*)(rhs._bin+4)));
        }

        inline bool operator !=(const EUI64& rhs) const
        {
            return ! (*this == rhs);
        }

        friend ostream& operator<<(ostream& os, const EUI64& eui) {
            return os << eui.to_string();
        }
     };

    class DevAddr {
        uint8_t _bin[4] __attribute__ ((aligned (4)));      //little endian
    public:
        DevAddr() {
            *(uint32_t*)_bin = 0;
        }
        /*
         * The DevAddr consists of 32 bits identifies the end-device within the current network.
         * The most significant 7 bits are used as network identifier (NwkID) to separate addresses of
         * territorially overlapping networks of different network operators and to remedy roaming
         * issues. The least significant 25 bits, the network address (NwkAddr) of the end-device, can
         * be arbitrarily assigned by the network manager.
         * */
        int NwkID() const {
            return _bin[3] >> 1;
        }
        void set_NwkID(int nwkid) {
            _bin[3] = (_bin[3] & 1) | (nwkid&0x7f) << 1;
        }
        uint32_t get_int() {
            return LittleEndian::get_u32p(_bin);
        }
        inline uint32_t hash_code() const {
            return (*(uint32_t*)_bin) * 4144924453;
        }
        inline bool valid() {
            return *(uint32_t*)_bin != 0;
        }
        inline void random(int nwkid) {
            *(uint32_t*)_bin = std::rand();
            _bin[3] = (_bin[3] & 1) | (nwkid&0x7f) << 1;

        }
        inline void to(uint8_t *p) const {
            p[0] = _bin[0];
            p[1] = _bin[1];
            p[2] = _bin[2];
            p[3] = _bin[3];
        }
        inline void from(const uint8_t *p) {
            _bin[0] = p[0];
            _bin[1] = p[1];
            _bin[2] = p[2];
            _bin[3] = p[3];
        }

        string to_string() const {
            return strings::hex2str(_bin, sizeof(_bin), true);
        }

        bool from_string(const string val) {
            return strings::str2hex(val, _bin, sizeof(_bin), true) == sizeof(_bin);
        }

        inline bool operator <(const DevAddr& rhs) const
        {
            return (int)(*(uint32_t*)_bin - *(uint32_t*)rhs._bin) < 0;
        }

        inline bool operator ==(const DevAddr& rhs) const
        {
            return *(uint32_t*)_bin == *(uint32_t*)rhs._bin;
        }

        inline bool operator !=(const DevAddr& rhs) const
        {
            return *(uint32_t*)_bin != *(uint32_t*)rhs._bin;
        }

        friend ostream& operator<<(ostream& os, const DevAddr& addr) {
            return os << addr.to_string();
        }
     };

    struct Url {
        string raw;
        string scheme;
        string host;
        int port;
    public:
        Url() {}
        Url(string s, string h, int p):scheme(s),host(h),port(p) {}
        bool parse(string url) {
            raw = url;
            int pos = url.find("://");
            if (pos > 0) {
                scheme = url.substr(0, pos);
                url = url.substr(pos+3);
            } else {
                scheme = "";
            }
            pos = url.find(":");
            if (pos > 0) {
                host = url.substr(0, pos);
                url = url.substr(pos+1);
            } else {
                host = url;
                url = "";
            }
            if (url.size() > 0) {
                port = atoi(url.c_str());
            }
            return port > 0 && host.size()>0 && scheme.size()>0;
        }
    };

    struct Buffer {
        uint8_t *buffer __attribute__ ((aligned (4)));
        uint16_t idx;   //
        uint16_t cap;   //max bytes can writen / max bytes can be read
    public:
        Buffer() {
            cap = idx = 0;
        }
        Buffer(uint8_t *b, uint16_t n) {
            buffer = b;
            idx = 0;
            cap = n;
        }
        inline bool valid() {
            return idx<cap;
        }
        void invalidate() {
            cap = idx = 0;
        }
        void flip() {
            cap = idx;
            idx = 0;
        }
    };

    template <class T, int N> class Avg {
        T           _bin[N];
        T           _sum = 0;
        uint32_t    _count = 0;
        T           _initval = 0;
        bool        _initdone = false;
    public:
        Avg() {
            for (int i=0; i<(int)NUMELMNT(_bin); i++) {
                _bin[i] = 0;
            }
        }
        T update(T value) {
            int idx = _count % N;
            _sum += value - _bin[idx];
            _bin[idx] = value;
            if (!_initdone) {
                for (int i = idx+1; i<N; i++) {
                    _sum += value - _bin[i];
                    _bin[i] = value;
                }
                _initval = value;
                _initdone = true;
            }
            ++_count;
            return _sum / N;
        }

        T initial() {
            return _initval;
        }
    };

    //sync remote with local mills/micros
    class TimeSync {
        time_t      _offset = 0;   //offset = local - remote
        time_t      _offset_prev = 0;
        int         _count = 0;
        time_t      _initial = 0;
    public:
        void sync(time_t local, time_t remote) {
            _offset_prev = _offset;
            _offset = local - remote;
            if (_count++ == 0) {
                _offset_prev = _offset;
                _initial = _offset;
            }
        }
        time_t to_remote(time_t local) {
            return local - _offset;
        }
        time_t to_local(time_t remote) {
            return remote + _offset;
        }
        int cur_precision() {
            return _offset - _offset_prev;
        }
        int accu_precision() {
            return _offset - _initial;
        }
        void reset() {
            _count = 0;
        }
    };
}

namespace std
{
    template<> struct less<lang::EUI64>
    {
        bool operator() (const lang::EUI64& lhs, const lang::EUI64& rhs) const
        {
            return lhs < rhs;
        }
    };

    template<> struct hash<lang::EUI64>
    {
        std::size_t operator()(lang::EUI64 const& eui) const
        {
            return eui.hash_code();
        }
    };
    template<> struct less<lang::DevAddr>
    {
        bool operator() (const lang::DevAddr& lhs, const lang::DevAddr& rhs) const
        {
            return lhs < rhs;
        }
    };

    template<> struct hash<lang::DevAddr>
    {
        std::size_t operator()(lang::DevAddr const& eui) const
        {
            return eui.hash_code();
        }
    };
}

#endif //AICAST_BACKHAUL_LANG_TYPES_H
