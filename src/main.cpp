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

#include "parameters_renderer.hpp"
#include "serial_io.hpp"
#include "shared/event_log_to_string.h"

int main (int argc, char *argv[])
{
	spdlog::set_level (spdlog::level::debug);
	spdlog::set_pattern ("[%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] %v");
	SPDLOG_INFO ("============ MODBUS2APRS  =============");

    SerialIo serialIo ("/dev/ttyS0");
    if (serialIo.open() == false)
    {
        return -1;
    }

    while(true)
    {
        auto event = serialIo.receiveSingleEvent();
        if (event.has_value()) {
            const event_log_t & ev = event.value();

            const event_log_source_t source_t = static_cast<event_log_source_t>(ev.source);
            const event_log_severity_t severity_t = static_cast<event_log_severity_t>(ev.severity);

            const uint32_t counter = ev.event_counter_id;
            const char * source = event_log_source_to_str(source_t);
            const char * event_name = event_id_to_str(source_t, ev.event_id);
            const std::string params = ParametersRenderer::getParameterStr (ev);

            switch (severity_t)
            {
                case EVENT_DEBUG:
                    spdlog::debug ("{} [{:<11}] ={}= || {} ||", counter, source, event_name, params);
                    break;

                case EVENT_INFO:
                case EVENT_INFO_CYCLIC:
                case EVENT_BOOTUP:
                case EVENT_TIMESYNC:
                    spdlog::info ("{} [{:<11}] = {} = || {} ||", counter, source, event_name, params);
                    break;

                case EVENT_WARNING:
                    spdlog::warn ("{} [{:<11}] = {} = || {} ||", counter, source, event_name, params);
                    break;

                case EVENT_ERROR:
                case EVENT_ASSERT:
                    spdlog::error ("{} [{:<11}] = {} = || {} ||", counter, source, event_name, params);
                    break;

                default:
                    spdlog::error ("{} [{:<11}] = {} = || {} (unknown severity {}) ||", counter, source, event_name,
                                    params, static_cast<int> (severity_t));
                    break;
            }
        }
    }

    return 0;
}

extern "C" int variant_validate_is_within_flash (const void *address)
{
    (void)(address);
    return 1;
}