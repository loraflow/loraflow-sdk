//
// Created by Thinkpad on 2017/11/4.
//

#ifndef AICAST_BACKHAUL_DGRAM_H
#define AICAST_BACKHAUL_DGRAM_H

#include <lang/lang-os.h>
#include <lang/errors.h>
#include <lang/lang-endian.h>
#include <types/message.h>
#include <backend-base.h>
#include <external/extns-encoding.h>
#include "nonstd/optional.hpp"

namespace haul {
    namespace extns {


        using namespace lang;
        using endian = LittleEndian;
        using nonstd::optional;

        class BadConnection : public std::exception {
        public:
            enum Cause {CLOSED, BADPROTO} cause;

            explicit BadConnection(Cause c):cause(c) {}
        };

        struct Dgram {
            uint8_t     *data;
            int         length;
        };

        class DgramProto {
        public:
            virtual void reset() = 0;
            virtual optional<Dgram> read(Transport *ref) = 0;
            virtual void write(Transport *ref, Message *message, PFEncode &enc) = 0;
        };

        class DgramProtoV1 : public DgramProto {
            static const int DGRAM_OVERHEAD = 4;
            uint8_t _write_buff[65536]{};
            uint8_t _read_buff[65536]{};
            uint8_t *wptr = _read_buff;   //write ptr
            uint8_t *rptr = _read_buff;   //read ptr
            uint8_t  swapchar{};

            void reset() override {
                wptr = _read_buff;
                rptr = _read_buff;
            }
            optional<Dgram> read(Transport *tpt) override {
                optional<Dgram> result{};
                int n;
                *rptr = swapchar;
                while(true) {
                    if (wptr - rptr >= 4) {     //loop starts from begin of buff
                        const uint8_t ver = *rptr;
                        const uint16_t size = endian::get_u16p(rptr + 1);
                        if (ver != DGRAM_PROTO_1) {
                            ERRORF("invalid dgram protocol {} {}", ver, Hex(rptr, 8));
                            throw BadConnection(BadConnection::BADPROTO);
                        }
                        if (size >= DGRAM_SIZE_BIG) {
                            ERRORF("dgram len {} overflow", size);
                            throw BadConnection(BadConnection::BADPROTO);
                        }
                        if (size <= wptr - rptr - DGRAM_OVERHEAD) {
                            rptr += DGRAM_OVERHEAD;
                            swapchar = rptr[size];
                            rptr[size] = '\0';
                            result = Dgram{rptr, size};
                            rptr += size;
                            break;
                        }
                    }
                    if (rptr != _read_buff) {
                        memmove(_read_buff, rptr, wptr - rptr);
                        wptr -= rptr - _read_buff;
                        rptr = _read_buff;
                    }

                    if ((n = tpt->Reade(wptr, sizeof(_read_buff) - (wptr - _read_buff))) < 0) {
                        throw BadConnection(BadConnection::CLOSED);
                    }
                    if (!n) {
                        break;
                    }
                    wptr += n;
                }
                return result;
            }
            void write(Transport *tpt, Message *message, PFEncode &enc) override {
                int nbytes = enc.encode(message, _write_buff+4, sizeof(_write_buff)-4);
                if (nbytes < 0) {
                    DEBUGF("message skipped by encoder");
                } else {
                    _write_buff[0] = DGRAM_PROTO_1;
                    endian::put_u16p(_write_buff + 1, nbytes);
                    _write_buff[3] = SIGN_DGRAM_PROTO(_write_buff);
                    if (tpt->Write(_write_buff, DGRAM_OVERHEAD + nbytes) <= 0) {
                        throw BadConnection(BadConnection::CLOSED);
                    }
                }
            }
        };

        class DgramProtoV2 : public DgramProto {
            static const int DGRAM_OVERHEAD = 2;
            uint8_t _write_buff[4096]{};
            uint8_t _cache[4096]{};
            uint8_t _read_buff[4096]{};
            uint8_t *wptr = _read_buff;   //write ptr
            uint8_t *rptr = _read_buff;   //read ptr
            uint8_t  swapchar{};
            void reset() override {
                wptr = _read_buff;
                rptr = _read_buff;
            }
            optional<Dgram> read(Transport *tpt) override {
                optional<Dgram> result{};
                int n;
                *rptr = swapchar;
                while(true) {
                    if (wptr - rptr >= DGRAM_OVERHEAD) {     //loop starts from begin of buff
                        const uint8_t ver = *rptr;
                        if ((ver & 3u) != DGRAM_PROTO_2) {
                            ERRORF("invalid dgram protocol {} {}", ver, Hex(rptr, 8));
                            throw BadConnection(BadConnection::BADPROTO);
                        }
                        const uint16_t size = ((uint16_t)(ver>>4u) << 8u) + rptr[1];
                        if (size <= wptr - rptr - DGRAM_OVERHEAD) {
                            rptr += DGRAM_OVERHEAD;
                            swapchar = rptr[size];
                            rptr[size] = '\0';
                            result = {rptr, size};
                            rptr += size;
                            break;
                        }
                        if (size > sizeof(_read_buff) - DGRAM_OVERHEAD) {
                            ERRORF("dgram protocol error (size %d {} overflow)", size);
                            throw BadConnection(BadConnection::BADPROTO);
                        }
                    }
                    if (rptr != _read_buff) {
                        memmove(_read_buff, rptr, wptr - rptr);
                        wptr -= rptr - _read_buff;
                        rptr = _read_buff;
                    }

                    if ((n = tpt->Reade(wptr, sizeof(_read_buff) - (wptr - _read_buff))) < 0) {
                        throw BadConnection(BadConnection::CLOSED);
                    }
                    if (!n) {
                        break;
                    }
                    wptr += n;
                }
                return result;
            }
            void write(Transport *tpt, Message *message, PFEncode &enc) override {
                const int maxcap = sizeof(_write_buff)-DGRAM_OVERHEAD;
                const int nbytes = enc.encode(message, _write_buff+DGRAM_OVERHEAD, maxcap);
                if (nbytes > maxcap) {
                    WARNF("message {} size {} overflow!", (int)message->get_type(), nbytes);
                } else {
                    _write_buff[0] = DGRAM_PROTO_2 + ((uint8_t)((uint16_t)nbytes >> 8u) << 4u);
                    _write_buff[1] = (uint8_t)nbytes;
                    if (tpt->Write(_write_buff, DGRAM_OVERHEAD + nbytes) <= 0) {
                        throw BadConnection(BadConnection::CLOSED);
                    }
                }
            }
        };

    }
}
#endif //AICAST_BACKHAUL_DGRAM_H
