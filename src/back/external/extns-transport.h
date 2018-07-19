//
// Created by Thinkpad on 2017/11/20.
//

#ifndef AICAST_BACKHAUL_EXTNS_TRANSPORT_H
#define AICAST_BACKHAUL_EXTNS_TRANSPORT_H


#include <queue>
#include <map>
#include <lang/lang-os.h>
#include <lang/lang-endian.h>
#include <lang/constants.h>
#include <lang/lang-json.h>
#include <lang/lang-log.h>
#include <lang/lang-types.h>

#define DGRAM_PROTO_1          1
#define DGRAM_PROTO_2          2
#define SIGN_DGRAM_PROTO(p)    (*(p) ^ *((p)+1) ^ *((p)+2))
#define MIN_RECONNECT_INTVL    3000

namespace haul {

    namespace extns {

        using namespace std;
        using namespace lang;
        using endian = LittleEndian;

        class Transports;

        class Transport {
            friend class Transports;
            int _references;
            virtual int Close() = 0;
        public:
            virtual ~Transport(){}
            Transport() {
                _references = 0;
            }
            uint32_t version;
            virtual int Reade(void*ptr, int size) = 0;
            virtual int Write(const void*ptr, int size) = 0;
            Transport* reference() {
                _references++;
                return this;
            }
        };

        class Adaptor {
        public:
            virtual Transport *connect(Url &url) = 0;
        };

        class Transports {
        public:
            typedef map<string, Adaptor*>   Adaptors;

        private:
            time_t              _attempted;
            Adaptors            _adaptors;

        public:

            void support(string scheme, Adaptor *adaptor) {
                _adaptors[scheme] = adaptor;
            }

            void close(Transport * t) {
                if (--t->_references <= 0) {
                    t->Close();
                    delete t;
                }
            }
            Transport * connect(string uuid, queue<Url> &servers) {
                if (_attempted) {
                    if (os::mills() - _attempted < MIN_RECONNECT_INTVL) {
                        os::sleep_ms(rand() % (MIN_RECONNECT_INTVL*2));
                    }
                }

                _attempted = os::mills();

                auto url = servers.front();

                servers.pop();
                servers.push(url);

                INFOF("connecting {}://{}:{}", url.scheme, url.host, url.port);

                try {
                    auto trans = _adaptors.at(url.scheme)->connect(url);

                    if (trans) {
                        // clear backoff after a successful connect
                        INFOF("connected with {}://{}", url.scheme, url.host);
                        static uint32_t _vergen = 0;
                        trans->version = (++_vergen << 1) | 1;
                        trans->_references = 1;
                    } else {
                        WARNF("cannot connect {}://{}", url.scheme, url.host);
                    }
                    return trans;
                } catch (out_of_range e) {
                    return nullptr;
                }
            }
        };
    }
}

#endif //AICAST_BACKHAUL_EXTNS_TRANSPORT_H
