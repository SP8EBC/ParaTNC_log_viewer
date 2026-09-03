#include <boost/program_options.hpp>
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <string>


#pragma push_macro("U")
#undef U
// pragma required as a workaround of possible conflict with cpprestsdk.
// more info here: https://github.com/fmtlib/fmt/issues/3330
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#pragma pop_macro("U")

int main (int argc, char *argv[])
{
	spdlog::set_level (spdlog::level::debug);
	SPDLOG_INFO ("============ MODBUS2APRS  =============");
    return 0;
}