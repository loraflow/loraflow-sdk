### Application-specific constants

APP_NAME := lora_pkt_fwd

### Environment constants

PKTFWD_COMMON_SRC_ROOT := lib/packet_forwarder/lora_pkt_fwd

PKTFWD_COMMON_HEADERS := $(PKTFWD_COMMON_SRC_ROOT)/inc/parson.h

PKTFWD_COMMON_OBJDIR = build/$(TARGET)/objs

PKTFWD_COMMON_CFLAGS := $(LIB_CFLAGS) -O1 -Wall -Wextra -std=c99 -I$(PKTFWD_COMMON_SRC_ROOT)/inc -I$(PKTFWD_COMMON_SRC_ROOT)

$(PKTFWD_COMMON_OBJDIR)/parson.o: $(PKTFWD_COMMON_SRC_ROOT)/src/parson.c $(PKTFWD_COMMON_HEADERS) | $(PKTFWD_COMMON_OBJDIR)
	@+echo "CC $<" ;\
	$(CC) -c $(PKTFWD_COMMON_CFLAGS) $< -o $@

PKTFWD_COMMON_OBJS := $(PKTFWD_COMMON_OBJDIR)/parson.o
