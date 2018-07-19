//
// Created by aisenz on 2018/1/3.
//

#include <lang/lang-os.h>
#include "stat-track.h"

namespace haul {
    namespace tracker {

        static Tracker *_ptracker = nullptr;

        Tracker &_tracker() {
            if (!_ptracker) {
                lang::os::exclusively([]() {
                    if (!_ptracker) {
                        _ptracker = new Tracker;
                    }
                });
            }
            return *_ptracker;
        }
    }
}