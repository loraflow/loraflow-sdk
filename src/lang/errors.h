//
// Created by Thinkpad on 2017/9/6.
//

#ifndef AICAST_BACKHAUL_ERRORS_H
#define AICAST_BACKHAUL_ERRORS_H

#include <string>

namespace lang {
    namespace errors {

        using std::string;

#define _ERR_MAP(XX)        \
            XX(0,   OK,       OK)    \
            XX(1,   SERVER_ERROR,       Server Error)           \
            XX(2,   BADREQ,             Bad Request)           \
            XX(3,   FORBIDDEN,          Forbidden)           \
            XX(4,   CONFLICTS,          Conflicts)           \
            XX(5,   INCOMPLETE,         Incomplete)           \
            XX(6,   INVALID_PASSWD,     Invalid PASSWD)       \
            XX(7,   INVALID_SERVER,     Invalid SERVER)       \
            XX(8,   INVALID_DEVADDR,    Invalid DevAddr)       \
            XX(9,   INVALID_DEVEUI,     Invalid DevEUI)       \
            XX(10,  INVALID_APPEUI,     Invalid AppEUI)       \
            XX(11,  INVALID_APPSKEY,    Invalid AppSKey)       \
            XX(12,  INVALID_NWKSKEY,    Invalid NwkSKey)       \
            XX(13,  INVALID_APPKEY,     Invalid AppKey)       \
            XX(14,  NOT_FOUND,          Not Found)       \
            XX(15,  UNSUPPORTED,        UNSUPPORTED)       \
            XX(16,  INVALID_FILE,       Invalid File)       \
            XX(17,  OVERFLOW,           Overflow)       \
            XX(18,  INVALID_FPORT,      Invalid fport)       \
            XX(19,  INVALID_ENCODING,   Invalid encoding)       \
            XX(20,  INVALID_DATA,       Invalid data)       \
            XX(21,  TODO,               TODO)       \


        enum ErrStatus
        {
#define XX(num, name, str) ERR_##name = num,
            _ERR_MAP(XX)
#undef XX
        };

        extern string _error_strings[];

        inline const string to_string(ErrStatus err) {
            return _error_strings[err];
        }
    }
}
#endif //AICAST_BACKHAUL_ERRORS_H
