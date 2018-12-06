//
// Created by Thinkpad on 2017/9/6.
//

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

#define DGRAM_SIZE_BIG 65507
#define DGRAM_TIMEOUT_MILLS 1000

#define PROTOCOL_VERSION1    1           /* v1.3 */
#define PROTOCOL_VERSION2    2           /* v1.3 */
#define PKT_PUSH_DATA   0
#define PKT_PUSH_ACK    1
#define PKT_PULL_DATA   2
#define PKT_PULL_RESP   3
#define PKT_PULL_ACK    4
#define PKT_TX_ACK      5
#define PKT_AISENZ_MIN   65
#define PKT_AISENZ_AUTH  65
#define PKT_AISENZ_DOWN  66
#define PKT_AISENZ_UP    67
#define PKT_AISENZ_FLASH 68

#define PKT_HEARTBEAT   69
#define PKT_CONACK      70

#ifndef HEARTBEAT_TIMEOUT
#define HEARTBEAT_TIMEOUT 300
#endif

#ifndef HELLO_TIMEOUT
#define HELLO_TIMEOUT (3600*24)
#endif

#ifndef CONFIG_HEAP_KBYTES
#define CONFIG_HEAP_KBYTES 10240
#endif
#define ErrOK       "OK"
#define ErrTODO     "TODO"
#define ErrMEM      "MEM"
#define ErrIGN      "IGN"
#define ErrBusy     "Busy"
#define ErrProto    "Proto"
#define ErrPublish  "Publish"
#define ErrBadJSON  "BadJSON"
#endif //AICAST_BACKHAUL_CONSTANTS_H
