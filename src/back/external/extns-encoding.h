//
// Created by Thinkpad on 2017/11/4.
//

#ifndef AICAST_BACKHAUL_ENCODING_H
#define AICAST_BACKHAUL_ENCODING_H

#include <cstdint>
#include <types/message.h>
#include <lang/lang-types.h>
#include <lang/lang-log.h>
#include <lang/lang-json.h>
#include <types/pushdata.h>
#include <types/pullresp.h>

namespace haul {
    namespace extns {

        using namespace lang;

        class PFEncode {
        public:
            virtual int encode(Message *message, uint8_t *buf, int capacity) = 0;
        };

        class PFDecode {
        public:
            virtual Message *decode(uint8_t *buf, int size) = 0;
        };

        class PFCodec : public PFEncode, public PFDecode {};

        class PFCodec_V10 : public PFCodec {
#define TYPEID_CONN     0
#define TYPEID_CONNACK  0
#define TYPEID_PUSHDATA 1
#define TYPEID_PULLRESP 1
#define TYPEID_AISENZUP 2
#define TYPEID_AISENZDN 2
            int encode(Message *message, uint8_t *buf, int capacity) override {
                int nbytes = -1;
                uint8_t type = message->get_type();
                switch (type) {
                    case PKT_PUSH_DATA:
                    {
                        PushData *pushData = reinterpret_cast<PushData *>(message);
                        nbytes = 1 + pushData->toBinary(buf+1);
                        if (nbytes > 1) { //binary successful
                            buf[0] = (1<<7) | (1<<3) | (TYPEID_PUSHDATA);  //binaryflag, ver, typeid
                            DEBUGF("PUSHDATA BINARY {}", lang::Hex(buf, nbytes));
                        } else {
                            buf[0] = (0<<7) | (1<<3) | (TYPEID_PUSHDATA);  //binaryflag, ver, typeid
                            nbytes = 1 + pushData->jsonize(buf + 1);
                            DEBUGF("PUSHDATA {}", (char*)(buf+1));
                        }
                        break;
                    }
                    case PKT_AISENZ_AUTH:
                        buf[0] = (0<<7) | (1<<3) | (TYPEID_CONN);  //binaryflag, ver, typeid
                        nbytes = 1 + message->jsonize(buf + 1);
                        DEBUGF("AUTH {}", (char*)(buf+1));
                        break;
                    case PKT_HEARTBEAT:
                        DEBUGF("HBEAT");
                        //fall-through
                    case PKT_AISENZ_UP:
                        buf[0] = (0<<7) | (1<<3) | (TYPEID_AISENZUP);  //binaryflag, ver, typeid
                        nbytes = 1 + message->jsonize(buf + 1);
                        DEBUGF("AISENZUP {}", (char*)(buf+1));
                        break;
                    case PKT_PULL_DATA:  //fallthrough
                    case PKT_TX_ACK:
                        nbytes = -1;
                        DEBUGF("IGNORE {}", type);
                        break;
                    default:
                        WARNF("unhandled message type {}", type);
                }
                return nbytes;
            }

            Message *decode(uint8_t *buf, int size) override {
                if (size < 2 || (buf[0] & 0x18u) != (1u << 3u)) {
                    if (size == 0) {
                        DEBUGF("hbeat down");
                        return new HbeatMessage;
                    }
                    DEBUGF("PF10 invalid {} bytes: {:02x}...", size, buf[0]);
                    return nullptr;
                }
                Message *ptr = nullptr;
                switch (buf[0] & 7u) {
                    case TYPEID_PULLRESP:
                    {
                        auto resp = new PullResp;
                        bool valid;
                        if (buf[0] & (1u<<7u)) { //is binary
                            DEBUGF("PULLRESP BINARY {}", lang::Hex(buf, size));
                            valid = resp->fromBinary(buf+1, size - 1);
                        } else {
                            DEBUGF("PULLRESP {}", (char*)buf+1);
                            valid = resp->fromText(buf+1, size - 1);
                        }
                        if (!valid) {
                            delete resp;
                        } else {
                            ptr = resp;
                        }
                        break;
                    }
                    case TYPEID_CONNACK:
                    {
                        auto ack = new ConnAckMessage;
                        try {
                            Json jdown = Json::parse(buf+1);
                            jsons::optional(jdown, "c", ack->cause);
                            ptr = ack;
                            DEBUGF("connack {}", (char*)(buf+1));
                        } catch(...) {
                            std::exception_ptr p = std::current_exception();
                            DEBUGF("invalid connack {}", (char*)(buf+1));
                            delete ack;
                        }
                        break;
                    }
                    case TYPEID_AISENZDN:
                    {
                        DEBUGF("AISENZDN {}", (char*)(buf+1));
                        auto down = new AisenzDownMessage;
                        try {
                            down->content = Json::parse(string((char*)buf+1, size-1));
                            ptr = down;
                        } catch (...) {
                            WARNF("bad json {}", (char*)buf+1);
                            delete down;
                        }
                        break;
                    }
                    default:
                        WARNF("unhandled pf10 byte0 {}", buf[0]);
                }
                return ptr;
            }
        };
        class PFCodec_V02 : public PFCodec {
            EUI64           &_mac;
        public:
            explicit PFCodec_V02(EUI64 &m):_mac(m) {}

            int encode(Message *message, uint8_t *buf, int capacity) override {
                int pfhdrbytes = 4;
                int jsonbytes;
                uint8_t type = message->get_type();
                switch (type) {
                    case PKT_PUSH_DATA:
                        _mac.to(buf + 4);
                        pfhdrbytes += 8;
                        jsonbytes = message->jsonize(buf + pfhdrbytes);
                        break;
                    case PKT_PULL_DATA:  //fallthrough
                    case PKT_TX_ACK:
                        _mac.to(buf + 4);
                        pfhdrbytes += 8;
                        //fallthrough
                    case PKT_AISENZ_AUTH:  //fallthrough
                    case PKT_HEARTBEAT:     //fallthrough
                    default:
                        jsonbytes = message->jsonize(&buf[pfhdrbytes]);
                        break;
                }
                buf[0] = PROTOCOL_VERSION2;
                buf[3] = type;
                DEBUGF("UP: {}", (char*)&buf[pfhdrbytes]);
                return pfhdrbytes + jsonbytes;
            }

            Message *decode(uint8_t *buf, int size) override {
                if (size < 4 || (buf[0] != PROTOCOL_VERSION2 && buf[0] != PROTOCOL_VERSION1)) {
                    DEBUGF("PF02 invalid {} bytes {:02x}...", size, buf[0]);
                    return nullptr;
                }
                DEBUGF("DOWN {}", (char*)buf+4);
                Message *ptr = nullptr;
                switch (buf[3]) {
                    case PKT_PULL_RESP:
                    {
                        auto resp = new PullResp;
                        if (!resp->fromText(buf+4, size - 4)) {
                            DEBUGF("bad pullresp", (char*)buf+4);
                            delete resp;
                        } else {
                            ptr = resp;
                        }
                        break;
                    }
                    case PKT_AISENZ_DOWN:
                    {
                        auto down = new AisenzDownMessage;
                        try {
                            if(size <= 4)
                            {
                                DEBUGF("hbeat down");
                                return new HbeatMessage;
                            }
                            down->content = Json::parse(string((char*)buf+4, size-4));
                            ptr = down;
                        } catch (...) {
                            WARNF("bad json {}", (char*)buf+1);
                            delete down;
                        }
                        break;
                    }
                    case PKT_AISENZ_FLASH:
                    {
                        PRINTF("{}", (char*)(buf+4));
                        break;
                    }
                    default:
                    {
                        WARNF("unhandled pf02 pkttyp {}", buf[3]);
                        break;
                    }
                }
                return ptr;
            }
        };

    }
}
#endif //AICAST_BACKHAUL_ENCODING_H
