//
// Created by Thinkpad on 2017/11/3.
//

#include <lang/lang-json.h>

namespace lang {
    namespace jsons {

        Json optional(const Json &opt, string key) {
            if (opt.is_object()) {
                try {
                    return opt.at(key);
                } catch(...) {
                }
            }
            return Json{};
        }

        void foreach(const Json &opt, string key, std::function<void(Json&)> fn) {
            auto it = opt.find(key);
            if (it != opt.end()) {
                Json j = it.value();
                if (j.is_array()) {
                    for (Json::iterator it = j.begin(); it != j.end(); ++it) {
                        fn(*it);
                    }
                }
            }
        }

        string normalize(string file) /*throw std::ios_base::failure*/ {
            std::stringstream buffer;
            std::ifstream f(file);
            f.exceptions(std::ifstream::failbit);
            buffer << f.rdbuf();
            JSON_Value * j = json_parse_string_with_comments(buffer.str().c_str());
            if (!j) {
                throw BadJsonException();
            }
            int N = json_serialization_size(j);
            string res;
            res.resize(N);
            json_serialize_to_buffer(j, &res[0], N);
            return res;
        }

        void merge(Json &dst, const Json&src) {
            if (!src.is_null()) {
                if (dst.is_null()) {
                    dst = Json(src);
                } else if (src.is_object()) {
                    for (auto it = src.begin(); it != src.end(); ++it) {
                        if (dst.is_null()) {
                            dst = Json{};
                        }
                        if (dst.find(it.key()) == dst.end()) {
                            dst[it.key()] = it.value();
                        } else {
                            merge(dst[it.key()], it.value());
                        }
                    }
                }
            }
        }
    }
}