
copyheaders:
	@if [ ! -f src/types/copy_loragw_hal.h ]; then \
	echo '#ifndef __COPY_LORAGW_HAL_H__' > src/types/copy_loragw_hal.h ;\
	echo '#define __COPY_LORAGW_HAL_H__' >> src/types/copy_loragw_hal.h ;\
	echo '' >> src/types/copy_loragw_hal.h ;\
	echo '/*THIS FILE IS AUTOMATICALLY COPIED. DO NOT MODIFY*/' >> src/types/copy_loragw_hal.h ;\
	echo '' >> src/types/copy_loragw_hal.h ;\
	grep '^#define PROTOCOL_VERSION' lib/packet_forwarder/lora_pkt_fwd/src/lora_pkt_fwd.c >> src/types/copy_loragw_hal.h ;\
	grep '^#define PKT_' lib/packet_forwarder/lora_pkt_fwd/src/lora_pkt_fwd.c >> src/types/copy_loragw_hal.h ;\
	echo '' >> src/types/copy_loragw_hal.h ;\
	echo '#undef _LORAGW_HAL_H' >> src/types/copy_loragw_hal.h ;\
	echo '' >> src/types/copy_loragw_hal.h ;\
	grep -v '#include "config.h"' lib/lora_gateway/libloragw/inc/loragw_hal.h >> src/types/copy_loragw_hal.h ;\
	echo '' >> src/types/copy_loragw_hal.h ;\
	echo '#undef _LORAGW_HAL_H' >> src/types/copy_loragw_hal.h ;\
	echo '' >> src/types/copy_loragw_hal.h ;\
	echo '#endif' >> src/types/copy_loragw_hal.h ;\
	fi
