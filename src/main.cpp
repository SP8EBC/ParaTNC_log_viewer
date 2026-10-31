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

#include "event_descriptions_storage.hpp"
#include "hardcoded_event_log_description_json.hpp"
#include "parameters_renderer.hpp"
#include "serial_io.hpp"
#include "serial_port_selector.hpp"
#include "shared/event_log_to_string.h"

/// ==================================================================================================
///	GLOBAL VARIABLES
/// ==================================================================================================

std::string serialPort;
bool skipZeroValues = false;
bool eventNameWidthSet = false;
std::size_t eventNameWidth = 0;
std::size_t maximumLineLn = 230;

/// ==================================================================================================
///	LOCAL DATA TYPES
/// ==================================================================================================

struct RenderedEvent {
	event_log_severity_t severity;
	uint32_t counter;
	const char *source;
	std::string params; //!< must stay before eventName, see renderEvent
	std::string eventName;
};

/// ==================================================================================================
///	LOCAL VARIABLES
/// ==================================================================================================

static const char *empty = "\0\0";
static const std::string emptyStr = "";

/// ==================================================================================================
///	LOCAL FUNCTIONS
/// ==================================================================================================

static std::string formatEventName (std::string eventName)
{
	if (!eventNameWidthSet) {
		return eventName;
	}

	if (eventName.size () > eventNameWidth) {
		eventName.resize (eventNameWidth);
	}
	else if (eventName.size () < eventNameWidth) {
		eventName.append (eventNameWidth - eventName.size (), ' ');
	}

	return eventName;
}

/**
 * Converts raw event into printable fields. Result is built in place in the caller
 * (guaranteed copy elision), strings are only moved, never copied.
 * @param ev event received from serial port
 * @param eventDescriptionJson description of this event loaded from JSON, or nullopt if
 * there is none. Taken by value, so the caller should std::move it in.
 * @return fields ready to be passed to spdlog
 */
static RenderedEvent renderEvent (const event_log_t &ev,
								  const std::optional<EventDescriptionObject> &eventDescriptionJson)
{
	/**
	 * Data out: the function returns a RenderedEvent struct holding severity, counter, source,
	 * params and eventName. C++17 guarantees the struct is built directly in the caller's rendered
	 * variable, so returning it copies nothing.
	 *
	 * Strings are moved, not copied:
	 *
	 * params: the result of getParameterStr goes straight into the struct field. That function
	 * returns const std::string, which can't be moved from, so assigning it to an existing string
	 * would copy it. Building the field directly avoids that. Event name: if there is a JSON
	 * description, its eventString is moved out of the local optional. The fallback name from
	 * event_id_to_str becomes a temporary string, which is then moved through formatEventName.
	 * Before, obj.eventString was copied into event_name, and formatEventName copied it again.
	 *
	 */
	const event_log_source_t source_t = static_cast<event_log_source_t> (ev.source);

	if (eventDescriptionJson.has_value ()) {
		// braced initializer list is evaluated left to right, so params are rendered
		// before eventString is moved out of eventDescriptionJson
		return RenderedEvent{
			static_cast<event_log_severity_t> (ev.severity),
			ev.event_counter_id,
			event_log_source_to_str (source_t),
			ParametersRenderer::getParameterStr (ev, skipZeroValues, eventDescriptionJson),
			formatEventName (eventDescriptionJson.value ().eventString)};
	}
	else {
		return RenderedEvent{
			static_cast<event_log_severity_t> (ev.severity),
			ev.event_counter_id,
			event_log_source_to_str (source_t),
			ParametersRenderer::getParameterStr (ev, skipZeroValues, std::nullopt),
			formatEventName (
				event_id_to_str (static_cast<event_log_source_t> (ev.source), ev.event_id))};
	}
}

/**
 * Converts raw event into printable fields, containing only one selected parameter.
 * Used for long events, which are split into multiple lines (one parameter per line).
 * The line for parameter 0 is a full one, with source and event name. Lines for
 * any other parameter have source and event name left empty, so they look like
 * a continuation of the first line.
 * @param ev event received from serial port
 * @param eventDescriptionJson description of this event loaded from JSON. Must not be
 * nullopt.
 * @param parameter index of parameter to render, 0 is the first one
 * @return fields ready to be passed to spdlog. Params may be empty if selected
 * parameter isn't used by this event
 * @throws std::runtime_error if eventDescriptionJson is nullopt
 */
static RenderedEvent
renderEventSingleParameter (const event_log_t &ev,
							const std::optional<EventDescriptionObject> &eventDescriptionJson,
							int parameter)
{
	if (!eventDescriptionJson.has_value ()) {
		throw std::runtime_error ("eventDescriptionJson cannot be empty here!!");
	}

	const event_log_source_t source_t = static_cast<event_log_source_t> (ev.source);

	if (parameter == 0) {
		// braced initializer list is evaluated left to right, so params are rendered
		// before eventString is moved out of eventDescriptionJson
		return RenderedEvent{
			static_cast<event_log_severity_t> (ev.severity),
			ev.event_counter_id,
			event_log_source_to_str (source_t),
			ParametersRenderer::getParameterStr (ev, false, eventDescriptionJson, parameter),
			formatEventName (eventDescriptionJson.value ().eventString)};
	}
	else {
		return RenderedEvent{
			static_cast<event_log_severity_t> (ev.severity),
			ev.event_counter_id,
			empty,
			ParametersRenderer::getParameterStr (ev, false, eventDescriptionJson, parameter),
			emptyStr};
	}
}

/**
 * Prints rendered event through spdlog, using log level matching event severity.
 * @param rendered event fields prepared by renderEvent or renderEventGeneric
 */
static void printRenderedEvent (const RenderedEvent &rendered)
{
	switch (rendered.severity) {
	case EVENT_DEBUG:
		spdlog::debug ("{} [{:<11}] = {:<48.48} = || {} ||",
					   rendered.counter,
					   rendered.source,
					   rendered.eventName,
					   rendered.params);
		break;

	case EVENT_INFO:
	case EVENT_INFO_CYCLIC:
	case EVENT_BOOTUP:
	case EVENT_TIMESYNC:
		spdlog::info ("{} [{:<11}] = {:<48.48} = || {} ||",
					  rendered.counter,
					  rendered.source,
					  rendered.eventName,
					  rendered.params);
		break;

	case EVENT_WARNING:
		spdlog::warn ("{} [{:<11}] = {:<48.48} = || {} ||",
					  rendered.counter,
					  rendered.source,
					  rendered.eventName,
					  rendered.params);
		break;

	case EVENT_ERROR:
	case EVENT_ASSERT:
		spdlog::error ("{} [{:<11}] = {:<48.48} = || {} ||",
					   rendered.counter,
					   rendered.source,
					   rendered.eventName,
					   rendered.params);
		break;

	default:
		spdlog::error ("{} [{:<11}] = {:<48.48} = || {} (unknown severity {}) ||",
					   rendered.counter,
					   rendered.source,
					   rendered.eventName,
					   rendered.params,
					   static_cast<int> (rendered.severity));
		break;
	}
}

/// ==================================================================================================
///	GLOBAL FUNCTIONS
/// ==================================================================================================

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

int main (int argc, char *argv[])
{
	parseArgs (argc, argv);

	spdlog::set_level (spdlog::level::debug);
	spdlog::set_pattern ("[%Y-%m-%d %H:%M:%S.%e] [%^%-8l%$] %v");
	SPDLOG_INFO ("============ ParaTNC_log_viewer  =============");

	SerialIo serialIo (serialPort);
	if (serialIo.open () == false) {
		return -1;
	}

	// reads JSON and creates database with event description (what parameters means etc)
	EventDescriptionsStorage jsonEvents (hardcodedJson);

	// infinite loop which receives events from serial port
	while (true) {

		// blocks until single event is received, or CRC error is detected or timeout
		auto event = serialIo.receiveSingleEvent ();

		// if correct event is received this optional has a value
		if (event.has_value ()) {
			bool longEvent = false;

			// contained event
			const event_log_t &ev = event.value ();

			// looks for description in data from JSON file
			std::optional<EventDescriptionObject> eventDescriptionJson =
				jsonEvents.getForEvent (ev);

			// if JSON file contain something about this event
			if (eventDescriptionJson.has_value ()) {
				// get an object value
				const EventDescriptionObject &obj = eventDescriptionJson.value ();

				// estimate how long parameters printout may be (text label + value)
				longEvent = ParametersRenderer::estimateEventPrintoutLn (obj, ev, maximumLineLn);

				if (longEvent) {
					// this parameter might not be used (empty), but always
					// print this line, because it will contain event name
					const RenderedEvent first = renderEventSingleParameter (ev, obj, 0);
					printRenderedEvent (first);

					for (int i = 1; i <= 6; i++) {
						const RenderedEvent event = renderEventSingleParameter (ev, obj, i);
						if (event.params.length () > 0) {
							printRenderedEvent (event);
						}
					}
				}
				else {
					// render using description
					const RenderedEvent rendered = renderEvent (ev, obj);

					// and print on the console
					printRenderedEvent (rendered);
				}
			}
			else {
				// render using generic renderer
				const RenderedEvent rendered = renderEvent (ev, std::nullopt);

				// and print on the console
				printRenderedEvent (rendered);
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