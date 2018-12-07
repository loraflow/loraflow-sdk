//
// Created by Thinkpad on 2017/9/6.
//

#ifndef AICAST_BACKHAUL_BACKEND_H
#define AICAST_BACKHAUL_BACKEND_H


#include <lang/lang-os.h>
#include <lang/errors.h>
#include <lang/lang-endian.h>
#include <types/message.h>
#include <backend-base.h>
#include <external/extns-transport.h>
#include <external/extns-tcp.h>
#include "extns-dgram.h"

#if CONFIG_SUPPORT_SSL == 1
#include <ext-ns/TLSAdaptor.h>
#endif

namespace haul {

    namespace extns {

        using namespace lang;
        using endian = LittleEndian;

        class Backend : public BaseBackend {
            os::Mutex       _lock;
            Transport       *transport = nullptr;
            uint32_t        brokenVersion = 0;
            std::thread     _thrd;
            std::thread     _thwr;
            Timeout         _watchdog;
            Timeout         _heartbeat;
            MessageQ        *_up;
            MessageQ        *_down;
            queue<Url>      _servers;
            EUI64           _mac;
            Transports      _transports;
            DgramProto      *_dgproto;
            PFCodec         *_codec;
            std::string     _ipaddrs;
            uint32_t        _catchtimeout{};
        public:

            Backend():_watchdog(Timeout(MAX(15*60, HEARTBEAT_TIMEOUT))),
                      _heartbeat(Timeout(HEARTBEAT_TIMEOUT/3+1)) {}

            void init(MessageQ *up, MessageQ *down) override;

            void join() override {
                _thrd.join();
                _thwr.join();
            }

        private:

            void* _daemon_read();
            void* _daemon_write();
            void _renew_connection();
        };

        Backend &_backend();
    }
}

#endif //AICAST_BACKHAUL_BACKEND_H
