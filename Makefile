OBJS := ./src/main.o ./src/serial_io.o ./src/shared/crc_.o ./wjwwood_serial/src/serial.o ./wjwwood_serial/src/impl/unix.o
DEFINES := -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
INCLUDES := -I/usr/include/spdlog -I ./wjwwood_serial/include -I.
LIBS := -lfmt -lboost_program_options 
LIBS_DIRS := -L/lib/x86_64-linux-gnu -L/lib/i386-linux-gnu -L/usr/lib
WARNS := -Wmaybe-uninitialized -Wpedantic -Wno-return-local-addr -Wparentheses


all: paratnc_log_viewer

clean:
	rm -rf -- *.o *.lst *.d *.svg paratnc_log_viewer
	rm -rf -- ./wjwwood_serial/src/*.d ./wjwwood_serial/src/*.o ./wjwwood_serial/src/impl/*.o ./wjwwood_serial/src/impl/*.d

paratnc_log_viewer: $(OBJS)
	@echo 'linking $@'
	g++ -o $(@) $(OBJS) $(LIBS_DIRS) $(LIBS) 
	objdump --source --all-headers --demangle --line-numbers --wide "$@" > "$@.lst"

%.o : %.cc
	@echo 'Building: $<'
	g++ $(DEFINES) $(INCLUDES) -std=c++20 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"

%.o : %.cpp
	@echo 'Building: $<'
	g++ $(DEFINES) $(INCLUDES) -std=c++20 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"

%.o : %.c
	@echo 'Building: $<'
	gcc $(DEFINES) $(INCLUDES) -std=c11 -Os -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"