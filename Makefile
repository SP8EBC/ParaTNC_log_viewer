OBJS := ./src/main.o ./src/serial_io.o ./src/serial_port_selector.o ./src/parameters_renderer.o ./shared/crc_.o ./shared/event_log_to_string.o ./wjwwood_serial/src/serial.o ./wjwwood_serial/src/impl/unix.o ./wjwwood_serial/src/impl/list_ports/list_ports_linux.o
DEFINES := -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG
INCLUDES := -I/usr/include/spdlog -I ./wjwwood_serial/include -I./shared -I.
LIBS := -lfmt -lboost_program_options
LIBS_DIRS := -L/lib/x86_64-linux-gnu -L/lib/i386-linux-gnu -L/usr/lib
WARNS := -Wmaybe-uninitialized -Wpedantic -Wno-return-local-addr -Wparentheses -Wformat=0

# -----------------------------------------------------------------------------
# Windows (i686-w64-mingw32) cross-compile target
#
# Boost.ProgramOptions and fmt are only available on this system as native
# x86_64 Linux builds, so program_options and fmt link against prebuilt
# i686-w64-mingw32 static libs checked into ./mingw32_libs/lib (see
# mingw32_libs/BUILD_NOTES.md for how they were built / how to rebuild them).
# Their headers are still taken from the system Boost/fmt packages, which
# were verified to be byte-identical to the sources those libs were built
# from.
# -----------------------------------------------------------------------------
WIN_CC := i686-w64-mingw32-gcc
WIN_CXX := i686-w64-mingw32-g++
WIN_BUILD_DIR := build-win32
WIN_OBJS := $(WIN_BUILD_DIR)/src/main.o $(WIN_BUILD_DIR)/src/serial_io.o $(WIN_BUILD_DIR)/src/serial_port_selector.o $(WIN_BUILD_DIR)/src/parameters_renderer.o $(WIN_BUILD_DIR)/shared/crc_.o $(WIN_BUILD_DIR)/shared/event_log_to_string.o $(WIN_BUILD_DIR)/wjwwood_serial/src/serial.o $(WIN_BUILD_DIR)/wjwwood_serial/src/impl/win.o $(WIN_BUILD_DIR)/wjwwood_serial/src/impl/list_ports/list_ports_win.o
WIN_DEFINES := -DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG -DBOOST_ALL_NO_LIB
# mingw32_libs/include holds symlinks to exactly the system boost/fmt/spdlog
# header dirs (see mingw32_libs/BUILD_NOTES.md) -- deliberately NOT the whole
# of /usr/include, since that also contains glibc's own sys/, bits/, stdio.h,
# etc. which must never be visible to the mingw compiler (one of those,
# sys/single_threaded.h, previously got picked up and broke the link with an
# undefined `__libc_single_threaded`, a glibc-only runtime symbol).
WIN_INCLUDES := -idirafter ./mingw32_libs/include -I ./wjwwood_serial/include -I./shared -I.
WIN_LIBS_DIRS := -L./mingw32_libs/lib
WIN_LIBS := -lfmt -lboost_program_options -lsetupapi -ladvapi32
# static-link the mingw runtime itself so the only DLLs the EXE needs are
# the standard Windows system ones (see mingw32_libs/BUILD_NOTES.md)
WIN_LDFLAGS := -static -static-libgcc -static-libstdc++

all: paratnc_log_viewer

windows: paratnc_log_viewer.exe

clean:
	rm -rf -- *.o *.lst *.svg paratnc_log_viewer paratnc_log_viewer.exe
	rm -rf -- ./src/*.d ./src/*.o ./shared/*.d ./shared/*.o
	rm -rf -- ./wjwwood_serial/src/*.d ./wjwwood_serial/src/*.o ./wjwwood_serial/src/impl/*.o ./wjwwood_serial/src/impl/*.d
	rm -rf -- ./wjwwood_serial/src/impl/list_ports/*.o ./wjwwood_serial/src/impl/list_ports/*.d
	rm -rf -- $(WIN_BUILD_DIR)

paratnc_log_viewer: $(OBJS)
	@echo 'linking $@'
	g++ -o $(@) $(OBJS) $(LIBS_DIRS) $(LIBS)
	objdump --source --all-headers --demangle --line-numbers --wide "$@" > "$@.lst"

paratnc_log_viewer.exe: $(WIN_OBJS)
	@echo 'linking $@'
	$(WIN_CXX) -o $(@) $(WIN_OBJS) $(WIN_LDFLAGS) $(WIN_LIBS_DIRS) $(WIN_LIBS)

%.o : %.cc
	@echo 'Building: $<'
	g++ $(DEFINES) $(INCLUDES) -std=c++20 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"

%.o : %.cpp
	@echo 'Building: $<'
	g++ $(DEFINES) $(INCLUDES) -std=c++20 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"

%.o : %.c
	@echo 'Building: $<'
	gcc $(DEFINES) $(INCLUDES) -std=c11 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"

$(WIN_BUILD_DIR)/%.o : %.cc
	@mkdir -p "$(dir $@)"
	@echo 'Building (win32): $<'
	$(WIN_CXX) $(WIN_DEFINES) $(WIN_INCLUDES) -std=c++20 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"

$(WIN_BUILD_DIR)/%.o : %.cpp
	@mkdir -p "$(dir $@)"
	@echo 'Building (win32): $<'
	$(WIN_CXX) $(WIN_DEFINES) $(WIN_INCLUDES) -std=c++20 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"

$(WIN_BUILD_DIR)/%.o : %.c
	@mkdir -p "$(dir $@)"
	@echo 'Building (win32): $<'
	$(WIN_CC) $(WIN_DEFINES) $(WIN_INCLUDES) -std=c11 -Os $(WARNS) -c -fmessage-length=0 -MMD -MP -MF"$(patsubst %.o,%.d,$(@))" -MT"$(@)" -o "$(@)" "$(<)"
