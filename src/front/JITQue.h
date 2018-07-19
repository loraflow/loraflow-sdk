//
// Created by Thinkpad on 2017/9/24.
//

#ifndef AICAST_BACKHAUL_JITQUE_H
#define AICAST_BACKHAUL_JITQUE_H


#include <types/message.h>
#include <types/pullresp.h>

namespace haul {



    class JITQue {
    public:
        class User {
        public:
            virtual void jit_send(PullResp* pr) = 0;
        };

    protected:
        static const int ADVANCE_US = 1000000;
        using Msg = PullResp*;
        using Msglist = list<Msg>;
        Mutex               _m;
        condition_variable  _cv;
        Msglist             _unbound;
        Msglist             _bound;
        thread              _th;
        User                &_user;

    public:

        JITQue(User &u) : _user(u) {}

        void put(Msg msg) {
            auto tref = msg->_tx.time_us;
            GUARD_BEGIN(_m);
            if (msg->_tx.class_ != 'C') {
                Msglist::iterator it = _bound.begin();
                for (; it != _bound.end(); ++it) {
                    if ((int)((*it)->_tx.time_us - tref) >= 0) {
                        break;
                    }
                }
                _bound.insert(it, msg);
            } else {
                _unbound.push_back(msg);
            }
            GUARD_END();
            _cv.notify_all();
        }

        void start() {
            _th = std::thread([this]() {
                while (1) {
                    Msg msg = _take();
                    _user.jit_send(msg);
                    //os::sleep_ms(1000);
                }
            });
        }
    protected:
        Msg _take() {
            volatile Msg re = nullptr;
            auto pred = [this, &re]() -> bool {
                const uint32_t ref = os::micros();
                if (!_bound.empty()) {
                    re = _bound.front();
                    if ((int)(re->_tx.time_us - ref) <= ADVANCE_US) {
                        _bound.pop_front();
                    } else {
                        re = nullptr;
                    }
                }
                if (!re && !_unbound.empty()) {
                    re = _unbound.front();
                    _unbound.pop_front();
                }
                return re;
            };
            while (!re) {
                unique_lock<mutex> lk(_m);
                _cv.wait_for(lk, std::chrono::microseconds(ADVANCE_US/2), pred);
            }
            return re;
        }
    };
}

#if 0
#include <features.h>

#define TX_START_DELAY  (1500+6000)   //us
#define TX_INTVL        1500   //us
#define MAX_FUTURE      (1000000 * 10)   //us

#define JUSTTIME    0
#define CONFLICTS   -1
#define TOOLATE     -2
#define TOOFAR      -3

static int _sched(RadioDevice *radio, TxData *txd) {
    const time_t T = txd->txp.Time;
    if (!T) {
        return JUSTTIME;
    }
    const time_t now_micro = getmicros();
    if (radio->busyUntil > 0) {
        const int r = (int)(radio->busyUntil - now_micro);
        if (r > 0) {
            if ((int)(T - radio->busyUntil) < TX_INTVL + TX_START_DELAY) {
                return CONFLICTS;
            }
        }
    }

    const int diff = (int)(T - now_micro);
    if (diff < 0) {
        return TOOLATE;
    }

    if (diff <= TX_START_DELAY) {
        return JUSTTIME;
    }
    if (diff > MAX_FUTURE) {
        INFOF(LOGMOD_JIT, "too far: %d. now=%ld, t=%ld", diff, now_micro, T);
        return TOOFAR;
    }
    return diff - TX_START_DELAY;
}

static void _enqueue(RadioDevice *radio, TxData *txd) {
    radio->jit.enqueued++;
    const int wait = _sched(radio, txd);
    if (wait >= 0) {
        if (!txd->txp.Time) {
            list_add(radio->txqUntimed, txd);
        } else {
            TxData *ptr = list_head(radio->txqTimed);
            TxData *prev = NULL;
            while (ptr) {
                int diff = (int)(ptr->txp.Time - txd->txp.Time);
                if (diff >= 0) {
                    if (diff < (txd->toa + TX_INTVL)) {
                        _free(txd);
                        radio->jit.conflicts++;
                        return;
                    }
                    break;
                }
                prev = ptr;
                ptr = ptr->next;
            }
            list_insert(radio->txqTimed, prev, txd);
        }
    } else {
        _free(txd);
        switch (wait) {
            case TOOLATE:
                radio->jit.tooLate++;
                break;
            case CONFLICTS:
                radio->jit.conflicts++;
                break;
            case TOOFAR:
                radio->jit.tooFar++;
                break;
        }
    }
}

static void _pf_jit_task(RadioDevice *dev) {
#define MINWAITMS 1
#define MAXWAITMS 1000

    INFOF(LOGMOD_JIT, "task jit starting");

    TxData *node;

    int shouldWait = MINWAITMS;
    for (dev->jitstatus = TASK_STATUS_RUNNING; dev->jitstatus == TASK_STATUS_RUNNING; ) {
        shouldWait = shouldWait <= MINWAITMS ? MINWAITMS : shouldWait > MAXWAITMS ? MAXWAITMS : shouldWait;
        MSGQ_RECEIVE(dev->mq, node, shouldWait);
        if (node) {
            _enqueue(dev, node);
        } else {
            FEED_WATCHDOG();
        }
        if (dev->busyUntil) {
            shouldWait = dev->busyUntil - getmicros();
            if (shouldWait > 1000) {
                shouldWait /= 1000;
                continue;
            }
            dev->busyUntil = 0;
        }
        list_t tl = dev->txqTimed;
        node = list_head(tl);
        if (!node) {
            tl = dev->txqUntimed;
            node = list_head(tl);
            if (!node) {
                shouldWait = MAXWAITMS;
                continue;
            }
        }
        shouldWait = _sched(dev, node);
        dev->phy.msf.blockbegin= getmicros();
        if (shouldWait>0) {
            shouldWait = (shouldWait+999)/1000;
            continue;
        }
        DEBUGF(LOGMOD_JIT, "sched %x: wt=%d", node, shouldWait);
        if (shouldWait == JUSTTIME) {
            dev->driver.Tx(dev, node);
            dev->jit.attemptd++;
        } else {
            dev->jit.tooSlow++;
        }
        list_remove(tl, node);
        _free(node);
        shouldWait = MINWAITMS;
    }
    dev->jitstatus = TASK_STATUS_INIT;
    INFOF(LOGMOD_JIT, "task jit stopping");
    TASK_SUICIDE();
}

void jit_enque(RadioDevice *dev, TxPacket *txp) {
    do {
        if (!dev->phy.rcfg.TxEnable) {
            INFOF(LOGMOD_JIT, "device tx disabled");
            break;
        }
        int size = (sizeof(TxData) + txp->Length + 31) & ~31;
        TxData *txd = (TxData*)malloc(size);
        if (!txd) {
            break;
        }
        txd->txp = *txp;
        txd->txp.Data = (byte*)(txd+1);
        txd->toa = LoRaTOA(txp);
        txd->next = NULL;
        memcpy(txd->txp.Data, txp->Data, txp->Length);
        if (!MSGQ_SEND_OK(dev->mq, txd, 100)) {
            WARNF(LOGMOD_JIT, "jitq #%d full", dev->no);
            _free(txd);
        }
    } while(0);
}

void jit_init(RadioDevice *dev) {
    if (dev->phy.rcfg.TxEnable) {
        dev->jitstatus = TASK_STATUS_STARTING;
        TASK_CREATE(_pf_jit_task, "tPFJit", dev, TASK_STACK_PFJIT, TASK_PRI_PFJIT, TASK_CPU_PFJIT);
        while (dev->jitstatus != TASK_STATUS_RUNNING) {
            sleep_ms(2);
        }
    }
}

void jit_deinit(RadioDevice *dev) {
    if (dev->jitstatus != TASK_STATUS_INIT) {
        dev->jitstatus = TASK_STATUS_STOPPING;
        while (dev->jitstatus != TASK_STATUS_INIT) {
            sleep_ms(2);
        }
    }
}
#endif

#endif //AICAST_BACKHAUL_JITQUE_H
