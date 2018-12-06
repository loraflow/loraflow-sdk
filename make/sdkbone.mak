
#define CONFIG_USE_UDPFRONT
#define CONFIG_USE_LRMODFRONT
#define CONFIG_USE_EXTNS
#define CONFIG_SUPPORT_EMBEDNS
#define CONFIG_SUPPORT_WEB
#define _GLIBCXX_HAS_GTHREADS


TARGET = sdkbone
PROD_MACROS = CONFIG_USE_UDPFRONT CONFIG_USE_EXTNS CONFIG_FORCE_RADIO0TX CONFIG_SUPPORT_HWAUTH CONFIG_SUPPORT_REST CONFIG_FIX_CPPSTD
PROD_LIBS = curl ssl crypto

include make/main.mak
