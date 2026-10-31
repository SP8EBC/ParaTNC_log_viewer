#include "event_descriptions_storage.hpp"
#include "event_description_object.hpp"

#include "nlohmann/json.hpp"
#include "spdlog/spdlog.h"

#include <stdexcept>

using json = nlohmann::json;

/**
 * Converts single object from 'events' array into EventDescriptionObject. Throws
 * nlohmann::json::exception if any field is missing or has wrong type and
 * std::out_of_range if severity or source has an unknown value
 */
static EventDescriptionObject parseSingleEvent (const json &j)
{
	EventDescriptionObject out;

	const int severity = j.at ("severity").get<int> ();
	if (severity < EVENT_DEBUG || severity > EVENT_TIMESYNC) {
		throw std::out_of_range ("unknown severity: " + std::to_string (severity));
	}

	const int source = j.at ("source").get<int> ();
	if (source < EVENT_SRC_MAIN || source > EVENT_SRC_FANET) {
		throw std::out_of_range ("unknown source: " + std::to_string (source));
	}

	const int eventId = j.at ("eventId").get<int> ();
	if (eventId < 0 || eventId > UINT8_MAX) {
		throw std::out_of_range ("eventId out of range: " + std::to_string (eventId));
	}

	out.severity = static_cast<event_log_severity_t> (severity);
	out.source = static_cast<event_log_source_t> (source);
	out.eventId = static_cast<uint8_t> (eventId);
	out.eventString = j.at ("eventString").get<std::string> ();
	out.param = j.at ("param").get<std::string> ();
	out.param2 = j.at ("param2").get<std::string> ();
	out.wparam = j.at ("wparam").get<std::string> ();
	out.wparam2 = j.at ("wparam2").get<std::string> ();
	out.wparam3 = j.at ("wparam3").get<std::string> ();
	out.lparam = j.at ("lparam").get<std::string> ();
	out.lparam2 = j.at ("lparam2").get<std::string> ();
	out.asHexParam = j.at ("asHexParam").get<bool> ();
	out.asHexParam2 = j.at ("asHexParam2").get<bool> ();
	out.asHexWparam = j.at ("asHexWparam").get<bool> ();
	out.asHexWparam2 = j.at ("asHexWparam2").get<bool> ();
	out.asHexWparam3 = j.at ("asHexWparam3").get<bool> ();
	out.asHexLparam = j.at ("asHexLparam").get<bool> ();
	out.asHexLParam2 = j.at ("asHexLparam2").get<bool> ();

	return out;
}

EventDescriptionsStorage::EventDescriptionsStorage (const std::string_view &jsonContent)
	: m_ConstructorSuccess (false)
{
	try {
		const json root = json::parse (jsonContent);

		SPDLOG_INFO("JSON event log description: {}", root.at("name").get<std::string>());
		SPDLOG_INFO("JSON event log creation date: {}", root.at("lastEditUtcISO8601").get<std::string>());

		for (const json &event : root.at ("events")) {
			EventDescriptionObject description = parseSingleEvent (event);

			std::map<uint8_t, EventDescriptionObject> *map = getMapForSource (description.source);

			// map is never null here as source has been already validated
			const auto [it, inserted] = map->emplace (description.eventId, description);
			// exact copies of an already stored description are harmless
			if (!inserted && !(it->second == description)) {
				SPDLOG_WARN ("Duplicated event description for source {}, eventId {}. Ignoring '{}'",
							  static_cast<int> (description.source),
							  description.eventId,
							  description.eventString);
			}
		}

		m_ConstructorSuccess = true;
	}
	catch (const std::exception &ex) {
		SPDLOG_ERROR ("Failed to parse event descriptions JSON: {}", ex.what ());
		m_ConstructorSuccess = false;
	}
}

std::optional<EventDescriptionObject>
EventDescriptionsStorage::getForEvent (const event_log_t &event)
{
	if (!m_ConstructorSuccess) {
		return std::nullopt;
	}

	const std::map<uint8_t, EventDescriptionObject> *map =
		getMapForSource (static_cast<event_log_source_t> (event.source));
	if (map == nullptr) {
		return std::nullopt;
	}

	const auto it = map->find (event.event_id);
	if (it == map->end ()) {
		return std::nullopt;
	}

	return it->second;
}

std::map<uint8_t, EventDescriptionObject> *
EventDescriptionsStorage::getMapForSource (event_log_source_t src)
{
	switch (src) {
	case EVENT_SRC_MAIN: return &m_SourceMain;
	case EVENT_SRC_WX_HANDLER: return &m_SourceWxHandler;
	case EVENT_SRC_PWR_SAVE: return &m_SourcePowerSave;
	case EVENT_SRC_PACKET_TX_HANDLER: return &m_SourcePacketTxHandler;
	case EVENT_SRC_APRSIS: return &m_SourceAprsis;
	case EVENT_SRC_KISS: return &m_SourceKiss;
	case EVENT_SRC_APRS_RF: return &m_SourceAprsRf;
	case EVENT_SRC_GSM_GPRS: return &m_SourceGsmGprs;
	case EVENT_SRC_TCPIP: return &m_SourceTcpip;
	case EVENT_SRC_HTTP_CLIENT: return &m_SourceHttpClient;
	case EVENT_SRC_MODBUS: return &m_SourceModbus;
	case EVENT_SRC_UMB: return &m_SourceUmb;
	case EVENT_SRC_DRV_ANEMOMETER: return &m_SourceDrvAnemometer;
	case EVENT_SRC_DRV_I2C: return &m_SourceDrvI2c;
	case EVENT_SRC_DRV_UART: return &m_SourceDrvUart;
	case EVENT_SRC_DRV_SPI: return &m_SourceDrvSpi;
	case EVENT_SRC_FANET: return &m_SourceDrvFanet;
	}

	return nullptr;
}
