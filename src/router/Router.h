//
// Created by Thinkpad on 2017/9/13.
//

#ifndef AICAST_BACKHAUL_ROUTER_H
#define AICAST_BACKHAUL_ROUTER_H


#include <front/Front.h>
#include <types/pushdata.h>
#include <types/pullresp.h>
#include <lang/lang-watchdog.h>
#include "front-router.h"
#include "back-router.h"

#ifndef CONFIG_ROUTER_WATCHDOG_TIMEOUT
#define CONFIG_ROUTER_WATCHDOG_TIMEOUT  180
#endif

namespace haul {
    using namespace std;

    class Router : public BackRouter {

        const long  _starttime;
        thread      _th;
        MessageQ   &_up;
        MessageQ   &_down;

        FrontRouter *_frouter;

    public:

        Router(MessageQ &m, MessageQ &d):_starttime(os::epoch()), _up(m),_down(d) {
            _th = thread(&Router::_run, this);
        }
        void init(FrontRouter *router) {
            _frouter = router;
        }
        void join() {
            _th.join();
        }

        void RoutePushData(PPushData ppd) override {
            _up.put(std::move(ppd));
        }

    private:

        void _run() {
            lang::watch::Dog _dog;
            for (;;) {
                PMessage pd = _down.take();
                _dog.after(CONFIG_ROUTER_WATCHDOG_TIMEOUT);
                switch (pd->get_type()) {
                    case PKT_PULL_RESP:
                    {
                        PPullResp pr((PullResp*)pd.release());
                        _frouter->RoutePullResp(std::move(pr));
                        break;
                    }
                    case PKT_AISENZ_DOWN:
                    {
                        _handle_aisenz_down(reinterpret_cast<AisenzDownMessage *>(pd.get()));
                        break;
                    }
                    default:
                        WARNF("unknown downstream packet {}", pd->get_type());
                        break;
                }
                _dog.stop();
            }
        }

        void _handle_aisenz_down(AisenzDownMessage *message);
    };
}

#endif //AICAST_BACKHAUL_ROUTER_H
