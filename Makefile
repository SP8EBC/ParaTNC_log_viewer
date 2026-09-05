OBJS := ./src/main.o ./src/serial_io.o ./src/parameters_renderer.o ./shared/crc_.o ./shared/event_log_to_string.o ./wjwwood_serial/src/serial.o ./wjwwood_serial/src/impl/unix.o
DEFINES := -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
INCLUDES := -I/usr/include/spdlog -I ./wjwwood_serial/include -I./shared -I.
LIBS := -lfmt -lboost_program_options 
LIBS_DIRS := -L/lib/x86_64-linux-gnu -L/lib/i386-linux-gnu -L/usr/lib
WARNS := -Wmaybe-uninitialized -Wpedantic -Wno-return-local-addr -Wparentheses -Wformat=0


all: paratnc_log_viewer

clean:
	rm -rf -- *.o *.lst *.d *.svg paratnc_log_viewer
	rm -rf -- ./src/*.d ./src/*.o ./shared/*.d ./shared/*.o
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
	gcc $(DEFINES) $(INCLUDES) -std=c11 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"