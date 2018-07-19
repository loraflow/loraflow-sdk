//
// Created by Thinkpad on 2017/9/21.
//

#ifndef AICAST_BACKHAUL_LANG_BASE64_H
#define AICAST_BACKHAUL_LANG_BASE64_H

#include <cstdint>

namespace lang {

    namespace base64 {
        int bin_to_b64(const uint8_t * in, int size, char * out, int max_len);
        int b64_to_bin(const char * in, int size, uint8_t * out, int max_len);
    };
}
#endif //AICAST_BACKHAUL_LANG_BASE64_H
