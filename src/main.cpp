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
#include <boost/program_options.hpp>

#include "parameters_renderer.hpp"
#include "serial_io.hpp"
#include "serial_port_selector.hpp"
#include "shared/event_log_to_string.h"

std::string serialPort;
bool skipZeroValues = false;
bool eventNameWidthSet = false;
std::size_t eventNameWidth = 0;

void parseArgs (int argc, char *argv[])
{
	boost::program_options::options_description optionsDescription ("");

	// ----------------
	// GENERAL OPTIONS
	// ----------------
	boost::program_options::options_description generalOptions ("General Options");
	boost::program_options::options_description_easy_init port = generalOptions.add_options ();
	port ("port,P",
		  boost::program_options::value<std::string> (&serialPort),
		  " : Serial port used for communication");
	optionsDescription.add (generalOptions);

	// ------------------
	// FORMATTING OPTIONS
	// ------------------
	boost::program_options::options_description formattingOptions ("Formatting");
	boost::program_options::options_description_easy_init formatOpts =
		formattingOptions.add_options ();
	formatOpts ("skip-zero-params,Z",
				boost::program_options::bool_switch (&skipZeroValues),
				" : Skip event parameters equal to zero in printed output");
	formatOpts ("event-name-width,W",
				boost::program_options::value<int> (),
				" : Fixed width for printed event name column; shorter names are space "
				"padded, longer ones trimmed. If not given, event name is printed as-is");
	optionsDescription.add (formattingOptions);

	// PARSING
	boost::program_options::variables_map odVariablesMap;
	try {
		boost::program_options::store (
			boost::program_options::parse_command_line (argc, argv, optionsDescription),
			odVariablesMap);
		boost::program_options::notify (odVariablesMap);
	}
	catch (boost::wrapexcept<boost::program_options::unknown_option> &ex) {
		SPDLOG_ERROR ("Missing or wrong parameter!");
		std::cout << ex.what () << std::endl << optionsDescription << std::endl;
		exit (-2);
	}

	if (serialPort.size () < 4) {
		std::cout << optionsDescription << std::endl;

		serialPort = SerialPortSelector::promptForSerialPort ();
	}

	if (odVariablesMap.count ("event-name-width")) {
		const int width = odVariablesMap["event-name-width"].as<int> ();
		if (width < 0) {
			SPDLOG_ERROR ("event-name-width must not be negative!");
			std::cout << optionsDescription << std::endl;
			exit (-4);
		}

		eventNameWidth = static_cast<std::size_t> (width);
		eventNameWidthSet = true;
	}
}

std::string formatEventName (const char *eventName)
{
	if (!eventNameWidthSet) {
		return std::string (eventName);
	}

	std::string result (eventName);
	if (result.size () > eventNameWidth) {
		result.resize (eventNameWidth);
	}
	else if (result.size () < eventNameWidth) {
		result.append (eventNameWidth - result.size (), ' ');
	}

	return result;
}

int main (int argc, char *argv[])
{
	parseArgs (argc, argv);

	spdlog::set_level (spdlog::level::debug);
	spdlog::set_pattern ("[%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] %v");
	SPDLOG_INFO ("============ MODBUS2APRS  =============");

	SerialIo serialIo (serialPort);
	if (serialIo.open () == false) {
		return -1;
	}

	while (true) {
		auto event = serialIo.receiveSingleEvent ();
		if (event.has_value ()) {
			const event_log_t &ev = event.value ();

			const event_log_source_t source_t = static_cast<event_log_source_t> (ev.source);
			const event_log_severity_t severity_t = static_cast<event_log_severity_t> (ev.severity);

			const uint32_t counter = ev.event_counter_id;
			const char *source = event_log_source_to_str (source_t);
			const char *event_name = event_id_to_str (source_t, ev.event_id);
			const std::string event_name_str = formatEventName (event_name);
			const std::string params = ParametersRenderer::getParameterStr (ev, skipZeroValues);

			switch (severity_t) {
			case EVENT_DEBUG:
				spdlog::debug ("{} [{:<11}] ={}= || {} ||",
							   counter,
							   source,
							   event_name_str,
							   params);
				break;

			case EVENT_INFO:
			case EVENT_INFO_CYCLIC:
			case EVENT_BOOTUP:
			case EVENT_TIMESYNC:
				spdlog::info ("{} [{:<11}] = {} = || {} ||",
							  counter,
							  source,
							  event_name_str,
							  params);
				break;

			case EVENT_WARNING:
				spdlog::warn ("{} [{:<11}] = {} = || {} ||",
							  counter,
							  source,
							  event_name_str,
							  params);
				break;

			case EVENT_ERROR:
			case EVENT_ASSERT:
				spdlog::error ("{} [{:<11}] = {} = || {} ||",
							   counter,
							   source,
							   event_name_str,
							   params);
				break;

			default:
				spdlog::error ("{} [{:<11}] = {} = || {} (unknown severity {}) ||",
							   counter,
							   source,
							   event_name_str,
							   params,
							   static_cast<int> (severity_t));
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