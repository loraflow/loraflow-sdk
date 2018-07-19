//
// Created by aisenz on 2018/1/3.
//

#ifndef AICAST_BACKHAUL_STAT_TRACK_H
#define AICAST_BACKHAUL_STAT_TRACK_H

#include <lang/lang-json.h>
#include <lang/lang-log.h>

namespace haul {

    using namespace lang;

    namespace tracker {

        /*
         * ``` json
{"stat":{
	"time":"2014-01-12 08:59:28 GMT",
	"lati":46.24000,
	"long":3.25230,
	"alti":145,
	"rxnb":2,
	"rxok":2,
	"rxfw":2,
	"ackr":100.0,
	"dwnb":2,
	"txnb":2
}}
```*/
        struct Stats {
            double  latitude = 0;
            double  longitude = 0;
            int32_t   altitude = 0;
        };

        class Tracker {
            Stats   _last;
            Stats   _next;
            bool    _updated = false;
        public:
            bool IsUpdated() const {
                return _updated;
            }
            bool IsEmpty() const {
                return _next.longitude == 0 && _next.latitude == 0;
            }
            Stats get() {
                _last = _next;
                _updated = false;
                return _next;
            }
            void refresh(lang::Json &j) {
                if (!j.empty() && j.is_object()) {
                    double lat, lng;
                    if (jsons::optional(j, "lati", lat) && jsons::optional(j, "long", lng)) {
                        _next.latitude = lat;
                        _next.longitude = lng;
                        jsons::optional(j, "alti", _next.altitude);
                        {
                            //
                            // mobility detection: https://stackoverflow.com/a/15742266
                            //
                            const auto THRESHOLD = 100;
                            const auto R = 6371000;  // radius of the earth
                            const auto deg2rad = 0.0174532925;
                            const auto lat1 = deg2rad * _last.latitude;
                            const auto lng1 = deg2rad * _last.longitude;
                            const auto lat2 = deg2rad * _next.latitude;
                            const auto lng2 = deg2rad * _next.longitude;
                            const auto x = (lng2 - lng1) * ::cos( 0.5*(lat2+lat1));
                            const auto y = lat2 - lat1;
                            const auto dist = R * ::sqrt( x*x + y*y );
                            _updated = dist >= THRESHOLD;
                            DEBUGF("GPS update {} AT {},{},{} dist {}", _updated, lat, lng, _next.altitude, dist);
                        }
                    }
                }
            }
        };

        Tracker & _tracker();
    }
}

#endif //AICAST_BACKHAUL_STAT_TRACK_H
