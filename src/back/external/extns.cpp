//
// Created by Thinkpad on 2017/11/20.
//

#include <external/extns.h>
#include <types/message.h>
#include <conf/config.h>
#include <router/stat-track.h>

namespace haul {
    namespace extns {

        static Backend *_pbackend = nullptr;

        Backend &_backend() {
            if (!_pbackend) {
                lang::os::exclusively([](){
                    if (!_pbackend) {
                        _pbackend = new Backend;
                    }
                });
            }
            return *_pbackend;
        }

        void Backend::init(MessageQ *up, MessageQ *down) {
            _up = up;
            _down = down;
            const auto &lcf = conf::INSTANCE.local();
            const auto &ext = lcf.networkservice.external;
            const auto &gwid = lcf.system.gatewayID;
            if (gwid == "0000000000000000" || !_mac.from_string(gwid)) {
                ERRORF("bad gateway id {}", gwid);
                exit(-1);
            } else if (!ext.url.size()) {
                ERRORF("missing ns server");
                exit(-1);
            }

            _catchtimeout = lcf.system.catchtimeout;

            _servers.push(Url("tls", ext.url, 4882));
            _servers.push(Url("tcp", ext.url, 4881));

            _transports.support("tcp", new extns());
#if CONFIG_SUPPORT_SSL == 1
            _transports.support("tcp", new TLSAdaptor());
#endif
            if (conf::INSTANCE.local().system.compatible_pfv02) {
                _dgproto = new DgramProtoV1;
                _codec = new PFCodec_V02(_mac);
            } else {
                _dgproto = new DgramProtoV2;
                _codec = new PFCodec_V10;
            }

            DEBUGF("pf watchdog ttl {} exec {}", lcf.system.pfwatchttl, lcf.system.pfwatchcmd);

            _thrd = std::thread(&Backend::_daemon_read, this);
            _thwr = std::thread(&Backend::_daemon_write, this);
            INFOF("start external network service with id {}", _mac.to_string());
        }

        void* Backend::_daemon_read() {
            Transport *ref = nullptr;
            for (;;) {
                bool reconnect = !ref;
                if (!reconnect) {
                    try {
                        auto dg = _dgproto->read(ref);
                        Message *m = _codec->decode(dg.data, dg.length);
                        if (!m) {
                            DEBUGF("null msg?");
                        } else {
                            _watchdog.refresh();  //got valid data
                            switch (m->get_type()) {
                                case PKT_CONACK:
                                {
                                    auto ack = reinterpret_cast<ConnAckMessage *>(m);
                                    if (!ack->cause.empty()) {
                                        WARNF("disconnected cause {}", ack->cause);
                                        reconnect = true;
                                    }
                                    delete m;
                                    break;
                                }
                                default:
                                {
                                    unique_ptr<Message> pr(m);
                                    _down->put(std::move(pr));
                                    break;
                                }
                            }
                        }
                    } catch (BadConnection e) {
                        INFOF("bad connection {}", e.cause);
                        reconnect = true;
                    }
                }
                if (reconnect) {
                    _dgproto->reset();
                    //uint32_t brokenVersion = 0;
                    if (ref) {
                        brokenVersion = ref->version;  //let writer thread notify broken pipe
                        _transports.close(ref);
                        ref = nullptr;
                    }
                    while (!ref) {
                        os::sleep_ms(500);
                        GUARD_BEGIN(_lock);
                            if (transport) {
                                if (transport && transport->version && transport->version != brokenVersion) {
                                    ref = transport->reference();
                                    brokenVersion = 0;
                                }
                            }
                        GUARD_END();
                    }
                }
            }
        }

        void* Backend::_daemon_write() {
            const auto MIN_GPS_INTVL = 5;
            PMessage pmsg;
            auto &tr = tracker::_tracker();
            time_t gpsUpdate = 0;
            Timeout  rxPushTimeout(conf::INSTANCE.local().system.pfwatchttl);

            for (;;) {
                while (true) {
                    if (_watchdog.expired()) {
                        WARNF("watchdog fired");
                        break;
                    }
                    while (true) {
                        if (!pmsg && !(pmsg = _up->take(3000))) {
                            break;
                        }
                        if (pmsg->age() <= _catchtimeout) {
                            break;
                        }
                        WARNF("message expired after {}s (max {} allowd)", pmsg->age(), _catchtimeout);
                        pmsg = nullptr;
                    }
                    if (!transport || brokenVersion == transport->version) {
                        break;
                    }
                    try {
                        if (pmsg) {
                            if (pmsg->get_type() == PKT_PUSH_DATA) {
                                rxPushTimeout.refresh();
                            }
                            _dgproto->write(transport, pmsg.get(), *_codec);
                            pmsg = nullptr;
                            _heartbeat.refresh();
                        } else {
                            if (_heartbeat.expired() || (tr.IsUpdated() && (os::epoch() - gpsUpdate) >= MIN_GPS_INTVL)) {
                                HbeatMessage hb;
                                if (!tr.IsEmpty()) {
                                    gpsUpdate = os::epoch();
                                    auto &&t = tr.get();
                                    Json stat;
                                    stat["lati"] = t.latitude;
                                    stat["long"] = t.longitude;
                                    stat["alti"] = t.altitude;
                                    hb.info["stat"] = stat;
                                }
                                _dgproto->write(transport, &hb, *_codec);
                                _heartbeat.refresh();
                            }
                            if (rxPushTimeout.expired()) {
                                const auto &cmd = conf::INSTANCE.local().system.pfwatchcmd;
                                if (cmd.length() > 0) {
                                    int rc = system(cmd.c_str());
                                    WARNF("reset pf radio chip {}", rc);
                                }
                                rxPushTimeout.refresh();
                            }
                        }
                    } catch (const BadConnection &e) {
                        if (pmsg && pmsg->get_type() != PKT_PUSH_DATA) {  //Retransmit PUSHDATA ONLY
                            pmsg = nullptr;
                        }
                        WARNF("connection broken {}", e.cause);
                        break;
                    }
                }
                _renew_connection();
                _watchdog.refresh();
            }
        }

        void Backend::_renew_connection() {

            if (transport) {
                GUARD_BEGIN(_lock);
                    ERRORF("close tcp socket");
                    _transports.close(transport);
                    transport = nullptr;
                GUARD_END();
                os::sleep_ms(1000 + random()%5000);
            }
            for (int i=0; i<3 && !transport; i++) {
                auto t = _transports.connect(_mac.to_string(), _servers);
                if (!t) {
                    os::sleep_ms(1000);
                    continue;
                }
                try {
                    ConnMessage cm;
                    Json props;
                    props["a"] = {"upg"};
                    cm._uuid = _mac.to_string();
                    cm._token = conf::INSTANCE.local().networkservice.external.token;
                    cm._props = props;
                    _dgproto->write(t, &cm, *_codec);
                    GUARD_BEGIN(_lock);
                        transport = t;
                    GUARD_END();
                } catch (...) {
                    _transports.close(t);
                }
            }
        }
    }
}