//
// Created by Thinkpad on 2017/11/22.
//
#ifdef CONFIG_SUPPORT_REST

#include <lang/lang-rest.h>
#include "lang-log.h"

namespace lang {


    bool Restful::_didinit;

    size_t Restful::read_func(void *ptr, size_t size, size_t nmemb, void *obj) {
        Restful *pthis = static_cast<Restful *>(obj);
        int nb = MIN((int)(pthis->_request.length() - pthis->_nread), (int)(size * nmemb));
        if (nb > 0) {
            memcpy(ptr, pthis->_request.c_str() + pthis->_nread, nb);
            pthis->_nread += nb;
        }
        DEBUGF("read {} bytes nread {}", nb, pthis->_nread);
        return nb;
    }

    size_t Restful::write_func(void *ptr, size_t size, size_t nmemb, void*obj) {
        Restful *pthis = static_cast<Restful *>(obj);
        pthis->_response.append((char*) ptr, size * nmemb);
        DEBUGF("write {} bytes", size * nmemb);
        return size * nmemb;
    }

    Restful::Restful(const string &url, const string &&data):_request(data),_data(data) {
        if (!_didinit) {
            lang::os::exclusively([]() {
                if (!_didinit) {
                    curl_global_init(CURL_GLOBAL_ALL);
                    _didinit = true;
                }
            });
        }
        _curl = curl_easy_init();
        if (_curl) {
            _nread = 0;
            curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
            //curl_easy_setopt(_curl, CURLOPT_READFUNCTION, read_func);
            //curl_easy_setopt(_curl, CURLOPT_READDATA, this);
            //curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, write_func);
            //curl_easy_setopt(_curl, CURLOPT_WRITEDATA, this);
        }
    }
    Restful::~Restful() {
        if (_curl) {
            curl_easy_cleanup(_curl);
        }
    }

    void Restful::post() {
        if (_curl) {
            struct curl_slist *list = nullptr;
            list = curl_slist_append(list, "Content-Type:application/json");
            //list = curl_slist_append(list, "Expect:");
            curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, list);
            curl_easy_setopt(_curl, CURLOPT_POST, 1l);
            curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, _data.c_str());
            _ccode = curl_easy_perform(_curl);
            curl_slist_free_all(list); /* free the list again */
            _attemptd = true;
            DEBUGF("post {} code {}", _data, _ccode);
        }
    }

    int Restful::statusCode() {
        int code = 0;
        if (_curl && _attemptd) {
            curl_easy_getinfo (_curl, CURLINFO_RESPONSE_CODE, &code);
        }
        return code;
    }

    string Restful::error() {
        if (!_attemptd) {
            return "unsupported";
        }
        if (_ccode == CURLE_OK) {
            return "";
        }
        return curl_easy_strerror(_ccode);
    }

    bool download(const string &url, const string &saveas) {
        DEBUGF("downloading {} to {}", url, saveas);
        int code = -1;
        FILE *fp = ::fopen(saveas.c_str(), "wb");
        if (!fp) {
            ERRNOF("fopen {}", saveas);
        } else {
            CURL *curl = curl_easy_init();
            if (curl) {
                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
                const CURLcode res = curl_easy_perform(curl);
                curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &code);
                curl_easy_cleanup(curl);
                if (res == CURLE_OK && code == 200) {
                    INFOF("download {} OK", saveas);
                } else {
                    ERRORF("download {} code {} err {} ({})", saveas, code, res, curl_easy_strerror(res));
                }
            }
            ::fclose(fp);
        }
        return code == 200;
    }
}

#endif //CONFIG_SUPPORT_REST
