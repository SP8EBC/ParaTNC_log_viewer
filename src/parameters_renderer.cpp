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

const std::string ParametersRenderer::getParameterStr (const event_log_t &ev, bool skipZeroValues)
{
	std::optional<std::string> cutomParemter = haveCustomParametersDescription (ev);
	if (cutomParemter.has_value ()) {}
	else {
		// TODO: dispatch to a per-event custom formatter once implemented
		return genericParameter (ev, skipZeroValues);
	}

	return genericParameter (ev, skipZeroValues);
}

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
