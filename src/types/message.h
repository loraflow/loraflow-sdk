//
// Created by Thinkpad on 2017/9/20.
//

#ifndef AICAST_BACKHAUL_MESSAGE_H
#define AICAST_BACKHAUL_MESSAGE_H

#include <lang/lang-os.h>
#include <lang/constants.h>
#include <list>
#include <condition_variable>
#include <lang/lang-json.h>
#include "conf/config.h"

namespace haul {

#define STATUS_SIZE     200
#define NB_PKT_MAX      8 /* max number of packets per fetch/send cycle */
#define TX_BUFF_SIZE    ((540 * NB_PKT_MAX) + 30 + STATUS_SIZE)

    using namespace lang;

    class Message {
    public:
        virtual uint8_t get_type() = 0;
        virtual ~Message() {}
        virtual int jsonize(uint8_t *buff) = 0;
        inline bool is_pf_message() {
            return get_type() <= PKT_AISENZ_MIN;
        }
    };


    class ConnMessage : public Message {
    public:
        string   _uuid;
        string   _token;
        Json     _props;
        uint8_t get_type() override {
            return PKT_AISENZ_AUTH;
        }

        int jsonize(uint8_t *buff) override {
            Json j;
            j["mac"] = _uuid;
            j["token"] = _token;
            j["ip"] = os::getip();
            j["ver"] = os::version();
            if (conf::INSTANCE.local().system.compatible_pfv02) {
                j["e"] = {"text"};
            } else {
                j["e"] = {"text","bin"};
            }

            jsons::merge(j, _props);
            string authtext = j.dump();
            strcpy((char*)buff, authtext.c_str());
            return authtext.length();
        }
        //bool say_hello() {
        //    char content[160];
        //    char hello[sizeof(content)+40];
        //    sprintf(content, "{\"act\":\"hello\",\"opt\":{\"ip\":\"%s\",\"version\":\"%s\"}}",
        //            os::getip().c_str(), os::version().c_str());
        //    DEBUGF("HELLO {}", content);
        //    int len = fill_dgram((uint8_t*)hello, PKT_AISENZ_UP, NULL, content, strlen(content));
        //    return transport->Write(hello, len) > 0;
        //}
    };

    class ConnAckMessage : public Message {
    public:
        string   cause;
        uint8_t get_type() override {
            return PKT_CONACK;
        }
        int jsonize(uint8_t *buff) override {
            throw "Not Implemented";
        }
    };

    class HbeatMessage : public Message {
    public:
        Json info;
        uint8_t get_type() override {
            return PKT_HEARTBEAT;
        }
        int jsonize(uint8_t *buff) override {
            info["act"] = "hb";
            const string s = info.dump();
            strcpy((char*)buff, s.c_str());
            return s.length();
        }
        //bool say_heartbeat() {
        //    static const char heartbeat[] = {DGRAM_PROTO_1, 4, 0, DGRAM_PROTO_1 ^ 4,
        //                                     PROTOCOL_VERSION2, 0, 0, PKT_AISENZ_UP};  //NOTE - little endian
        //    DEBUGF("HEARTBEAT up");
        //    _heartbeat.refresh();
        //    return transport->Write(heartbeat, sizeof(heartbeat)) > 0;
        //}
    };

    class AisenzDownMessage : public Message {
    public:
        Json   content;
        uint8_t get_type() override {
            return PKT_AISENZ_DOWN;
        }
        int jsonize(uint8_t *buff) override {
            throw "Not Implemented";
        }
    };

    class AisenzUpMessage : public Message {
    public:
        Json   content;
        uint8_t get_type() override {
            return PKT_AISENZ_UP;
        }
        int jsonize(uint8_t *buff) override {
            string s = content.dump();
            strcpy((char*)buff, s.c_str());
            return s.length();
        }
    };

    class Datgram {
    public:
        int capacity;
        int len;
        struct sockaddr_in addr;
        char buffer[0];

        static Datgram* allocate(int size) {
            int N = sizeof(Datgram) + size + 4;
            Datgram *res = reinterpret_cast<Datgram *>(new char[N]);
            if (res) {
                res->capacity = size;
                res->len = 0;
            }
            return res;
        }
    };


    using PDatgram = std::unique_ptr<Datgram>;
    using PMessage = std::unique_ptr<Message>;

    using lang::os::Mutex;

    template<typename T>
    class UniqueQue {
        list<std::unique_ptr<T>>   _ll;
        Mutex               _m;
        condition_variable  _cv;
    public:
        enum {NO_WAIT=0, WAIT_FOREVER=-1};
        void put(std::unique_ptr<T> m) {
            GUARD_BEGIN(_m);
                _ll.push_back(std::move(m));
            GUARD_END();
            _cv.notify_all();
        }

        std::unique_ptr<T> take(int waitmills=WAIT_FOREVER) {
            std::unique_ptr<T> re = _try();
            if (!re && waitmills != NO_WAIT) {
                auto predict = [this](){ return !_ll.empty(); };
                while (!re) {
                    const auto wait = std::chrono::milliseconds(waitmills == WAIT_FOREVER ? 1000 : waitmills);
                    {
                        std::unique_lock<std::mutex> lk(_m);
                        _cv.wait_for(lk, wait, predict);
                    }
                    re = _try();
                    if (!!re || waitmills != WAIT_FOREVER) {
                        break;
                    }
                }
            }
            return re;
        }

        int size() {
            return _ll.size();
        }

        inline std::unique_ptr<T> _try() {
            std::unique_ptr<T> re;
            GUARD_BEGIN(_m);
                if (!_ll.empty()) {
                    re = std::move(_ll.front());
                    _ll.pop_front();
                }
            GUARD_END();
            return re;
        }
    };

    using MessageQ = UniqueQue<Message>;
    using DatgramQ = UniqueQue<Datgram>;
}

namespace std
{
    template<>
    struct default_delete<haul::Datgram> {
        void operator()(haul::Datgram* ptr) {
            delete ptr;
        }
    };
}

#endif //AICAST_BACKHAUL_MESSAGE_H
