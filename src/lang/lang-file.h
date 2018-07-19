//
// Created by Thinkpad on 2017/9/23.
//

#ifndef AICAST_BACKHAUL_LANG_FILE_H
#define AICAST_BACKHAUL_LANG_FILE_H

#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <fstream>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <string>
#include <sys/stat.h>
#include <assert.h>

namespace lang {
    namespace io {

        using std::string;

        // exceptions
        class file_error { } ;
        class open_error : public file_error { } ;
        class close_error : public file_error { } ;
        class write_error : public file_error { } ;

        class file
        {
        public:
            file( const char* filename )
                    :
                    m_file_handle(std::fopen(filename, "w+"))
            {
                if( m_file_handle == NULL )
                {
                    throw open_error() ;
                }
            }

            ~file()
            {
                std::fclose(m_file_handle) ;
            }

            void write( string &str )
            {
                if (std::fwrite(str.c_str(), str.size(), 1, m_file_handle) == 0) {
                    throw write_error() ;
                }
            }

            void write( const char* buffer, std::size_t num_chars )
            {
                if( num_chars != 0 &&
                    std::fwrite(buffer, num_chars, 1, m_file_handle) == 0 )
                {
                    throw write_error() ;
                }
            }

        private:
            std::FILE* m_file_handle ;

            // copy and assignment not implemented; prevent their use by
            // declaring private.
            file( const file & ) ;
            file & operator=( const file & ) ;
        };


        inline long fsize(const std::string& filename)
        {
            FILE* f = std::fopen(filename.c_str(), "rb");
            if (!f) {
                return -1;
            }
            fseek(f, 0, SEEK_END); // seek to end of file
            long size = ftell(f); // get current file pointer
            fclose(f);
            return size;
        }

        inline bool is_file(const std::string& s)
        {
            struct stat st;
            return stat(s.c_str(), &st) >= 0 && S_ISREG(st.st_mode);
        }

        inline bool is_dir(const std::string& s)
        {
            struct stat st;
            return stat(s.c_str(), &st) >= 0 && S_ISDIR(st.st_mode);
        }

        inline void read_file(const std::string& path, std::string& out)
        {
            std::ifstream fs(path, std::ios_base::binary);
            fs.seekg(0, std::ios_base::end);
            auto size = fs.tellg();
            fs.seekg(0);
            out.resize(static_cast<size_t>(size));
            fs.read(&out[0], size);
        }

        inline std::string file_extension(const std::string& path)
        {
            std::smatch m;
            auto pat = std::regex("\\.([a-zA-Z0-9]+)$");
            if (std::regex_search(path, m, pat)) {
                return m[1].str();
            }
            return std::string();
        }

        inline const char* content_type(const std::string& path)
        {
            auto ext = file_extension(path);
            if (ext == "txt") {
                return "text/plain";
            } else if (ext == "html") {
                return "text/html";
            } else if (ext == "js") {
                return "text/javascript";
            } else if (ext == "css") {
                return "text/css";
            } else if (ext == "xml") {
                return "text/xml";
            } else if (ext == "jpeg" || ext == "jpg") {
                return "image/jpg";
            } else if (ext == "png") {
                return "image/png";
            } else if (ext == "gif") {
                return "image/gif";
            } else if (ext == "svg") {
                return "image/svg+xml";
            } else if (ext == "ico") {
                return "image/x-icon";
            } else if (ext == "json") {
                return "application/json";
            } else if (ext == "pdf") {
                return "application/pdf";
            } else if (ext == "xhtml") {
                return "application/xhtml+xml";
            }
            return "application/octet-stream";
        }
    }
}
#endif //AICAST_BACKHAUL_LANG_FILE_H
