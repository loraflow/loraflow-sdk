//
// Created by Thinkpad on 2017/11/24.
//

#ifndef AICAST_BACKHAUL_AISIGN_H
#define AICAST_BACKHAUL_AISIGN_H

#include <string>
#include <cstring>
#include <crypt/crypt-aes.h>
#include <crypt/crypt-sha2.h>
#include <lang/lang-strings.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#define K1 ((uint8_t)('A'*'i'))
#define K2 ((uint8_t)('S'*'z'))
#define K3 ((uint8_t)('N'*'o'))
#define KGF(a,b,c) ((uint8_t)((a)*7+(b)*37+(c)*57))
#define _AISIGN_PADDING_ RSA_PKCS1_PADDING

namespace haul {
    namespace aisign {

        using namespace std;
        using namespace cry::aes;

        class Signer {
            SecBuf<32>  key;
        public:
            Signer() {
                key.bin[0] = KGF(K1,K2,K3);
                key.bin[1] = KGF(K2,K3,key.bin[0]);
                key.bin[2] = KGF(K3,key.bin[0],key.bin[1]);
                for (int i=3; i< (int)sizeof(key.bin); i++) {
                    key.bin[i] = KGF(key.bin[i-3], key.bin[i-2], key.bin[i-1]);
                }
            }
            string _pub_dec(const string &key, const string &data) {
                std::string result;
                RSA* rsa = RSA_new();
                BIO *bufio = BIO_new_mem_buf((void*)key.c_str(), key.length()+1024);
                PEM_read_bio_RSA_PUBKEY(bufio, &rsa, 0, nullptr);
                uint8_t * buff = new uint8_t[RSA_size(rsa) + 1024];
                int ret = RSA_public_decrypt(data.length(), (const uint8_t *)data.c_str(), buff, rsa, _AISIGN_PADDING_);
                if (ret >= 0) {
                    result = std::string((char*)buff, ret);
                }
                delete [] buff;
                RSA_free(rsa);
                BIO_free(bufio);
                //CRYPTO_cleanup_all_ex_data();
                return result;
            }
            string _pri_enc(const string &key, const string &data) {
                std::string result;
                RSA* rsa = RSA_new();
                BIO *bufio = BIO_new_mem_buf((void*)key.c_str(), key.length());
                PEM_read_bio_RSAPrivateKey(bufio, &rsa, 0, nullptr);
                uint8_t * buff = new uint8_t[RSA_size(rsa) + 1];
                int ret = RSA_private_encrypt(data.length(), (const uint8_t*)data.c_str(), buff, rsa, _AISIGN_PADDING_);
                if (ret >= 0) {
                    result = std::string((char*)buff, ret);
                }
                delete [] buff;
                RSA_free(rsa);
                BIO_free(bufio);
                //CRYPTO_cleanup_all_ex_data();
                return result;
            }
            string sign(const string &privkey, const void *data, int len) {
                int M = ((2+len+15)>>4)<<4;    //M is round to 16Bytes
                uint8_t *temp = new uint8_t[M];
                temp[0] = len;
                temp[1] = len>>8;
                memcpy(temp+2, data, len);
                AesCBC<32> cbc(key);
                cbc.encrypt(temp, M);
                string ctext((char*)temp, M);
                delete []temp;
                return _pri_enc(privkey, _hash(ctext) + ctext);
            }
            string verify(const string &pubkey, const string sign) {
                string res;
                string text = _pub_dec(pubkey, sign);
                if (text.length()/16*16 == text.length()) {
                    string part2 = text.substr(16);
                    if (_hash(part2) == text.substr(0, 16)) {
                        int M = part2.length();
                        uint8_t *temp = new uint8_t[M];
                        memcpy(temp, part2.c_str(), M);
                        AesCBC<32> cbc(key);
                        cbc.decrypt(temp, M);
                        int nb = temp[0] + temp[1] * 256;
                        if (nb < M-2) {
                            res = string((char*)temp+2, nb);
                        }
                    }
                }
                return res;
            }

        protected:
            string _hash(string&ctext) {
                string &&h = picosha2::hash256_hex_string(ctext+string((char*)key.bin, sizeof(key.bin)));
                uint8_t hbin[16];
                lang::strings::str2hex(h.substr(0, 32), hbin, sizeof(hbin));
                return string((char*)hbin, sizeof(hbin));
            }
        };
    }
}
#endif //AICAST_BACKHAUL_AISIGN_H
