//
// Created by Thinkpad on 2017/9/18.
//

#ifndef AICAST_BACKHAUL_ENDIAN_H
#define AICAST_BACKHAUL_ENDIAN_H

namespace lang {

    class LittleEndian {
    public:
        static inline void put_u16(uint8_t *&ptr, uint16_t val) {
            *ptr++ = val;
            *ptr++ = val>>8;
        }
        static inline void put_u24(uint8_t *&ptr, uint32_t val) {
            *ptr++ = val;
            *ptr++ = val>>8;
            *ptr++ = val>>16;
        }
        static inline void put_u32(uint8_t *&ptr, uint32_t val) {
            *ptr++ = val;
            *ptr++ = val>>8;
            *ptr++ = val>>16;
            *ptr++ = val>>24;
        }
        static inline void put_u16p(uint8_t *ptr, uint16_t val) {
            *ptr++ = val;
            *ptr++ = val>>8;
        }
        static inline void put_u24p(uint8_t *ptr, uint32_t val) {
            *ptr++ = val;
            *ptr++ = val>>8;
            *ptr++ = val>>16;
        }
        static inline void put_u32p(uint8_t *ptr, uint32_t val) {
            *ptr++ = val;
            *ptr++ = val>>8;
            *ptr++ = val>>16;
            *ptr++ = val>>24;
        }
        static uint16_t get_u16(const uint8_t *&ptr) {
            uint16_t val = ((uint16_t)(ptr[0]) + ((uint16_t)(ptr[1])<<8));
            ptr+=2;
            return val;
        }
        static uint32_t get_u24(const uint8_t *&ptr) {
            uint32_t val = ((uint32_t)(ptr[0]) + ((uint32_t)(ptr[1])<<8) +
                            ((uint32_t)(ptr[2])<<16));
            ptr+=3;
            return val;
        }
        static uint32_t get_u32(const uint8_t *&ptr) {
            uint32_t val = ((uint32_t)(ptr[0]) + ((uint32_t)(ptr[1])<<8) +
                            ((uint32_t)(ptr[2])<<16) + ((uint32_t)(ptr[3])<<24));
            ptr+=4;
            return val;
        }
        static uint16_t get_u16p(const uint8_t *ptr) {
            return ((uint16_t)(ptr[0]) + ((uint16_t)(ptr[1])<<8));
        }
        static uint32_t get_u24p(const uint8_t *ptr) {
            return ((uint32_t)(ptr[0]) + ((uint32_t)(ptr[1])<<8) +
                            ((uint32_t)(ptr[2])<<16));
        }
        static uint32_t get_u32p(const uint8_t *ptr) {
            return ((uint32_t)(ptr[0]) + ((uint32_t)(ptr[1])<<8) +
                            ((uint32_t)(ptr[2])<<16) + ((uint32_t)(ptr[3])<<24));
        }
    };
}

#endif //AICAST_BACKHAUL_ENDIAN_H
