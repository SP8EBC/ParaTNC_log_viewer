# ParaTNC_log_viewer




### Summary of what Claude did to make Windows target
Summary
New Windows build target: make windows cross-compiles with i686-w64-mingw32-g++, producing paratnc_log_viewer.exe. Added to Makefile.

Required external DLLs (verified via objdump -p on the actual built EXE, and confirmed it runs under Wine):

KERNEL32.dll
ADVAPI32.dll (registry access, used by COM port enumeration)
SETUPAPI.dll (device enumeration, used by serial::list_ports())
msvcrt.dll
All four ship with every Windows install since XP — no MinGW runtime DLLs (libstdc++-6.dll, libgcc_s_*.dll, libwinpthread-1.dll) and no Boost/fmt DLLs, since those are statically linked in (-static -static-libgcc -static-libstdc++ plus static .a libs).

What made this nontrivial:

Boost.ProgramOptions and fmt (compiled libs, needed because this system's spdlog package forces SPDLOG_FMT_EXTERNAL) had no i686-w64-mingw32 build anywhere on the system. Per your choice, I cross-built both from source (fmt 9.1.0, Boost 1.83.0 — matching the versions already installed for the native Linux build) and checked the resulting .a files into mingw32_libs/lib/, with full rebuild instructions in mingw32_libs/BUILD_NOTES.md. Their headers aren't duplicated — mingw32_libs/include/ holds symlinks to the system Boost/fmt/spdlog headers (verified byte-identical to the sources those libs were built from).
Had to scope that header fallback (-idirafter) to exactly those three symlinked dirs, not all of /usr/include — an earlier attempt leaked glibc's sys/single_threaded.h into the mingw build and broke the link with an undefined __libc_single_threaded (a glibc-only runtime symbol libstdc++ doesn't need on mingw).
SerialPortSelector::isRealSerialPort()'s phantom-/dev/ttyS* filtering (the ioctl-based check from earlier) is Linux-only and now #ifdef-guarded in serial_port_selector.cpp — on Windows, SetupAPI enumeration never invents nonexistent COM ports, so it's a no-op stub there.
.gitignore's blanket *.a rule got a scoped exception for mingw32_libs/lib/*.a.