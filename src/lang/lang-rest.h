//
// Created by Thinkpad on 2017/11/22.
//

#ifndef AICAST_BACKHAUL_LANG_REST_H
#define AICAST_BACKHAUL_LANG_REST_H

#include <string>
#include <curl/curl.h>
#include <lang/lang-os.h>

namespace lang {
    using std::string;
    class Restful {
        static bool _didinit;
        CURLcode _ccode;
        bool  _attemptd = false;
        const string &_request;
        int    _nread = 0;
        CURL * _curl = nullptr;
        const string _data;
    public:
        string  _response;
        Restful(const string &url, const string &&data);
        ~Restful();
        void post();
        void download(const string &saveas);
        string error();
        int statusCode();

    protected:
        static size_t read_func(void *ptr, size_t size, size_t nmemb, void *obj);
        static size_t write_func(void *ptr, size_t size, size_t nmemb, void*obj);
    };

    bool download(const string &url, const string &saveas);
}
#endif //AICAST_BACKHAUL_LANG_REST_H
