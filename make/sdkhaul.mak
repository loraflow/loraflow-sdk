
#define CONFIG_USE_UDPFRONT
#define CONFIG_USE_LRMODFRONT
#define CONFIG_USE_EXTNS
#define CONFIG_SUPPORT_EMBEDNS
#define CONFIG_SUPPORT_WEB

TARGET = sdkhaul
TOOLCHAIN = native
PROD_MACROS = CONFIG_USE_UDPFRONT CONFIG_USE_EXTNS

include make/main.mak
