//
// Created by gl on 2018/11/1.
//

#ifndef LORALAN_BACKHAUL_VERSION_HPP
#define LORALAN_BACKHAUL_VERSION_HPP

#include <string>

using namespace std;

#define _xstr(a) _str(a)
#define _str(a) #a

#ifndef CONFIG_FWVER
#define CONFIG_FWVER 0.0.0
#endif

#ifndef CONFIG_HWVER
#define CONFIG_HWVER 0.0.0
#endif

#ifndef CONFIG_GATEWAYTYPE
#define CONFIG_GATEWAYTYPE loralan-gateway
#endif

class versions {
public:
    static string FWVer() {
        return _xstr(CONFIG_FWVER);

    }
    static string HWVer() {
        return _xstr(CONFIG_HWVER);

    }
    static string GatewayType() {
        return _xstr(CONFIG_GATEWAYTYPE);

    }
};

#undef _str
#undef _xstr

#endif //LORALAN_BACKHAUL_VERSION_HPP
