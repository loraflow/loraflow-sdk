
-include $(OBJS:.o=.dep)

FILEDEPS = src/types/copy_loragw_hal.h Makefile $(wildcard make/*.mak)

src/types/copy_loragw_hal.h: copyheaders

src/gen/aipubkey.cpp:
	@if [ ! -f src/gen/aipubkey.cpp ]; then \
	xxd -i res/pubkey.pem > src/gen/aipubkey.cpp; \
	fi

clean:
	@rm -rf $(OBJS) build/$(TARGET)
	@rm -f src/types/copy_loragw_hal.h

build/$(TARGET):
	@mkdir -p $@ $@/objs

build/$(TARGET)/objs/%.o: %.c build/$(TARGET)/objs/%.d $(FILEDEPS)
	@+echo "CC $<" ;\
	$(CC) -c $(CFLAGS) -std=c99 $< -o $@ -MD -MF $(@:.o=.dep)

build/$(TARGET)/objs/%.o: %.cpp build/$(TARGET)/objs/%.d $(FILEDEPS)
	@+echo "C++ $<" ;\
	$(CPP) -c $(CPPFLAGS) $< -o $@ -MD -MF $(@:.o=.dep)

build/$(TARGET)/objs/%.o: %.s build/$(TARGET)/objs/%.d $(FILEDEPS)
	@+echo "CC $<" ;\
	$(CC) -c -mcpu=cortex-m4 -mthumb $< -o $@ -MD -MF $(@:.o=.dep)

build/$(TARGET)/$(TARGET).elf: build/$(TARGET) $(OBJS) $(LIBS)
	@+echo "Linking $@";\
	$(LD) $(OBJS) $(LFLAGS) -o $@

build/$(TARGET)/$(TARGET).bin: build/$(TARGET)/$(TARGET).elf
	@+echo "Copying $@..." ;\
	$(CP) $(CPFLAGS) $< $@ ;\
	$(OD) $(ODFLAGS) $< > build/$(TARGET)/$(TARGET).lst

build/$(TARGET)/objs/%.d: ;
