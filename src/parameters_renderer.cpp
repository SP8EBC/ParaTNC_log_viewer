#include "parameters_renderer.hpp"

#include "shared/events_definitions/events_aprs_rf.h"
#include "shared/events_definitions/events_aprsis.h"
#include "shared/events_definitions/events_drv_anemometer.h"
#include "shared/events_definitions/events_drv_i2c.h"
#include "shared/events_definitions/events_drv_spi.h"
#include "shared/events_definitions/events_drv_uart.h"
#include "shared/events_definitions/events_fanet.h"
#include "shared/events_definitions/events_gsm_gprs.h"
#include "shared/events_definitions/events_http_client.h"
#include "shared/events_definitions/events_kiss.h"
#include "shared/events_definitions/events_main.h"
#include "shared/events_definitions/events_modbus.h"
#include "shared/events_definitions/events_packet_tx_handler.h"
#include "shared/events_definitions/events_pwr_save.h"
#include "shared/events_definitions/events_tcpip.h"
#include "shared/events_definitions/events_umb.h"
#include "shared/events_definitions/events_wx_handler.h"

#include <sstream>

/**
 * Returns true if this label is valid and a parameter should be even printed
 */
inline bool isParamEnabled (const std::string &label)
{
	if (label.size () == 0) {
		return false;
	}
	else if (label[0] == '0') {
		return false;
	}
	else {
		return true;
	}
}

/**
 * Renders all parameters (param, param2...) into a printable string to be
 * displayed by spdlog
 * @note if parameter descriptionFromJson is empty (std::nullopt) this function
 * renders string with default names
 * @param event to process
 * @param skipZeroValues if true, parameters equal to zero are omitted from the output
 * @param descriptionFromJson optional event description loaded from JSON, used to
 * name and format parameters
 * @return string to be displayed by spdlog
 */
const std::string ParametersRenderer::getParameterStr (
	const event_log_t &ev, bool skipZeroValues,
	const std::optional<EventDescriptionObject> &descriptionFromJson,
	std::optional<int> parameterNumber)
{
	if (!descriptionFromJson.has_value () && parameterNumber.has_value ()) {
		// printing single parameter per line is used only for long labels
		// read from JSON
		throw std::runtime_error (
			"ParametersRenderer::getParameterStr, missing descriptionFromJson");
	}

	// call to a function which is kinda-sorta copy-paste from ParaMETEO firmware
	// it checks if there is hardcoded string for this event. ParaMETEO (and config
	// tool for this moment) doesn't use any kind of JSON database.
	//
	// TODO: currently this always return nullopt
	std::optional<std::string> customParameter = haveCustomParametersDescription (ev);
	if (customParameter.has_value ()) {
		// TODO: this if is always false, but put something here to make compiler happy
		return genericParameter (ev, skipZeroValues);
	}
	else {
		// check if JSON had anything for this event
		if (descriptionFromJson.has_value ()) {
			if (parameterNumber.has_value ()) {
				return singleParameter (ev, descriptionFromJson.value (), parameterNumber.value ());
			}
			else {
				return parameterFromJsonEventDescription (descriptionFromJson.value (), ev);
			}
		}
		else {
			// neither hardcoded specific formatter is present, nor JSON database knowns anything
			// about this event. display it in generic form
			return genericParameter (ev, skipZeroValues);
		}
	}
}

/**
 * Renders single parameter for the event. This function is used only in case of
 * an event with a description found in JSON data, which is so long (which
 * parameters names are so long) that it can't fit in single console line.
 * @param event to process
 * @param descriptionFromJson data from JSON database, which by design is guaranteed to be
 * available in the moment of a call to this member function
 * @param parameter which one to render
 * @return string to be displayed by spdlog
 */
const std::string ParametersRenderer::singleParameter (
	const event_log_t &event, const EventDescriptionObject &descriptionFromJson, int parameter)
{
	std::ostringstream oss;
	auto append = [&] (const std::string label, unsigned long value, bool isHex) {
		if (!isParamEnabled (label)) {
			return;
		}

		if (isHex) {
			oss << std::hex << label << ": 0x" << value;
		}
		else {
			oss << std::dec << label << ": " << value;
		}
	};

	switch (parameter) {
	case 0:
		append (descriptionFromJson.param,
				static_cast<unsigned int> (event.param),
				descriptionFromJson.asHexParam);
		break;
	case 1:
		append (descriptionFromJson.param2,
				static_cast<unsigned int> (event.param2),
				descriptionFromJson.asHexParam2);
		break;
	case 2:
		append (descriptionFromJson.wparam, event.wparam, descriptionFromJson.asHexWparam);
		break;
	case 3:
		append (descriptionFromJson.wparam2, event.wparam2, descriptionFromJson.asHexWparam2);
		break;
	case 4:
		append (descriptionFromJson.wparam3, event.wparam3, descriptionFromJson.asHexWparam3);
		break;
	case 5:
		append (descriptionFromJson.lparam, event.lparam, descriptionFromJson.asHexLparam);
		break;
	case 6:
		append (descriptionFromJson.lparam2, event.lparam2, descriptionFromJson.asHexLParam2);
		break;
	default: break; // will result in empty string
	}

	return oss.str ();
}

/**
 * Estimates if the printout of this event will be long enough that it shall be split
 * to multiple lines. Each parameter in multiple line
 */
bool ParametersRenderer::estimateEventPrintoutLn (const EventDescriptionObject &obj,
												  const event_log_t &event, const int maximumLineLn)
{
	// position at which first parameters starts on screen
	constexpr size_t firstParamColumn = 112;

	size_t ln = 0;

	if (maximumLineLn <= firstParamColumn) {
		// generally 'maximumLineLn' should not be so small
		// buy this is a parameter given by a user from the console
		return true;
	}

	if (isParamEnabled (obj.param)) {
		ln += obj.param.length ();
		ln += 3; // this is 8bit value
	}

	if (isParamEnabled (obj.param2)) {
		ln += obj.param2.length ();
		ln += 3; // this is 8bit value
	}

	if (isParamEnabled (obj.wparam)) {
		ln += obj.wparam.length ();
		ln += 5; // this is 16bit value
	}

	if (isParamEnabled (obj.wparam2)) {
		ln += obj.wparam2.length ();
		ln += 5; // this is 16bit value
	}

	if (isParamEnabled (obj.wparam3)) {
		ln += obj.wparam3.length ();
		ln += 5; // this is 32bit value
	}

	if (isParamEnabled (obj.lparam)) {
		ln += obj.lparam.length ();
		ln += 8; // this is 32bit value
	}

	if (isParamEnabled (obj.lparam2)) {
		ln += obj.lparam2.length ();
		ln += 8; // this is 32bit value
	}

	if (maximumLineLn <= ln) {
		return true;
	}
	else {
		return false;
	}
}

/**
 * Returns a string if this class has hardcoded member function, which can decode
 * exact meaning of each parameter, rescale it and print into this returned string
 * @param event to check
 * @return string with custom-formatted parameters or nullopt if no custom format is available
 */
std::optional<std::string>
ParametersRenderer::haveCustomParametersDescription (const event_log_t &event)
{
	const event_log_source_t src = static_cast<event_log_source_t> (event.source);
	const event_log_severity_t svrty = static_cast<event_log_severity_t> (event.severity);
	const uint32_t id = event.event_id;

	std::string out;

	if (src == EVENT_SRC_MAIN && svrty == EVENT_TIMESYNC && id == EVENTS_MAIN_TIMESYNC_NTP) {
		// storeTimesyncEntryInExport(eventLogEntry,timestamp);
		return out;
	}
	else if (src == EVENT_SRC_MAIN && svrty == EVENT_ERROR &&
			 id == EVENTS_MAIN_POSTMORTEM_HARDFAULT) {
		return out;
		// storeHardfaultException(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_MAIN && svrty == EVENT_ERROR &&
			 id == EVENTS_MAIN_POSTMORTEM_SUPERVISOR) {
		return out;

		// storeSupervisorFault(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_MAIN && svrty == EVENT_INFO_CYCLIC && id == EVENTS_MAIN_CYCLIC) {
		return out;

		// storeCyclic(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_MAIN && svrty == EVENT_ERROR &&
			 id == EVENTS_MAIN_CONFIG_FIRST_CRC_FAIL) {
		return out;

		// storeConfigFirstCrcFail(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_MAIN && svrty == EVENT_ERROR &&
			 id == EVENTS_MAIN_CONFIG_SECOND_CRC_FAIL) {
		return out;

		// storeConfigSecondCrcFail(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_MAIN && id == EVENTS_MAIN_CONFIG_FIRST_RESTORE) {
		return out;

		// storeConfigFirstRestore(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_MAIN && id == EVENTS_MAIN_CONFIG_SECOND_RESTORE) {
		return out;

		// storeConfigSecondRestore(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_GSM_GPRS && svrty == EVENT_BOOTUP &&
			 id == EVENTS_GSM_GPRS_REGISTERED_NETWORK) {
		return out;

		// storeGsmRegisteredNetwork(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_GSM_GPRS && svrty == EVENT_BOOTUP && id == EVENTS_GSM_GPRS_IMSI) {
		return out;

		// storeGsmImsi(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_GSM_GPRS && svrty == EVENT_BOOTUP &&
			 id == EVENTS_GSM_GPRS_IP_ADDRESS) {
		return out;

		// storeGsmIpAddress(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_MAIN && svrty == EVENT_BOOTUP && id == EVENTS_MAIN_BOOTUP_COMPLETE) {
		return out;

		// storeBootupComplete(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_FANET && svrty == EVENT_ERROR &&
			 id == EVENTS_FANET_FAIL_TO_SEND_METEO) {
		return out;

		// storeFanetFail(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_KISS && id == EVENTS_DEFINITIONS_KISS_WARN_ERASING_STARTUP) {
		return out;

		// storeErasingStartup(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_KISS && id == EVENTS_DEFINITIONS_KISS_WARN_FLASHING_STARTUP) {
		return out;

		// storeFlashingStartup(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_PWR_SAVE && id == EVENTS_PWR_SAVE_SWITCHING_MODE) {
		return out;

		// storeSwitchingPowersavingMode(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_PWR_SAVE && id == EVENTS_PWR_SAVE_WOKEN_UP_AFTER_LAST_SLEEP) {
		return out;

		// storeWokenUpAfterLastSleep(eventLogEntry,timestamp);
	}
	else if (src == EVENT_SRC_PWR_SAVE && id == EVENTS_PWR_SAVE_GO_TO_SLEEP) {
		return out;

		// storeGoToSleep(eventLogEntry,timestamp);
	}

	// TODO: fixme
	return std::nullopt; // for simplicity as for now
}

/**
 * Renders generic representation of event parameters in form of:
 *  p: xxx, p2: xxx, wp: xxxx, wp2: xxx, wp3: xxx, lp: xxx, lp2: xxx
 *
 * where 'xxx' are hexadecimal values of each parameter. '0x' prefix is
 * not printed to save space!
 *
 * @param event to print
 * @param skipZeroValues if true, parameters equal to zero are omitted from the output
 * @return string with generic rendering
 */
std::string ParametersRenderer::genericParameter (const event_log_t &event, bool skipZeroValues)
{
	std::ostringstream oss;
	oss << std::hex;

	bool first = true;
	auto append = [&] (const char *label, unsigned long value) {
		if (skipZeroValues && value == 0) {
			return;
		}

		if (!first) {
			oss << ", ";
		}

		oss << label << value;
		first = false;
	};

	append ("p: ", static_cast<unsigned int> (event.param));
	append ("p2: ", static_cast<unsigned int> (event.param2));
	append ("wp: ", event.wparam);
	append ("wp2: ", event.wparam2);
	append ("wp3: ", event.wparam3);
	append ("lp: ", event.lparam);
	append ("lp2: ", event.lparam2);

	return oss.str ();
}

/**
 * Renders a specific representation of an event parameters using description from JSON file.
 * Only parameters which are used to store something will be printed, each with own,
 * name. By default, a tool which create this JSON puts a variable name put into a call
 * to log function
 * @param obj description from JSON
 * @param event event log to stringify
 */
std::string
ParametersRenderer::parameterFromJsonEventDescription (const EventDescriptionObject &obj,
													   const event_log_t &event)
{
	std::ostringstream oss;
	bool first = true;
	auto append = [&] (const std::string label, unsigned long value, bool isHex) {
		if (!isParamEnabled (label)) {
			return;
		}

		if (!first) {
			oss << ", ";
		}

		if (isHex) {
			oss << std::hex << label << ": 0x" << value;
		}
		else {
			oss << std::dec << label << ": " << value;
		}

		first = false;
	};

	append (obj.param, static_cast<unsigned int> (event.param), obj.asHexParam);
	append (obj.param2, static_cast<unsigned int> (event.param2), obj.asHexParam2);
	append (obj.wparam, event.wparam, obj.asHexWparam);
	append (obj.wparam2, event.wparam2, obj.asHexWparam2);
	append (obj.wparam3, event.wparam3, obj.asHexWparam3);
	append (obj.lparam, event.lparam, obj.asHexLparam);
	append (obj.lparam2, event.lparam2, obj.asHexLParam2);

	return oss.str ();
}
