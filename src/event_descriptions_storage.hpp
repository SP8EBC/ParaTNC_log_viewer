#ifndef C3D1F7A2_8E4B_4C6D_9A15_2B7E0F3C8D41
#define C3D1F7A2_8E4B_4C6D_9A15_2B7E0F3C8D41

#include "event_description_object.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>

class EventDescriptionsStorage {
  public:
	/**
	 * Constructor of Descriptions Storage, which
	 * parses JSON (read previously from a text file
	 * or simply hardcoded in the application source)
	 * and generates maps of single events
	 *
	 * @param jsonContent a JSON with event descriptions
	 */
	EventDescriptionsStorage (const std::string_view &jsonContent);

    /**
     * Returns a copy of Log Event Description, fetched from appropriate
     * map using source and event id of given event
     * @param event event received from the device
     */
    std::optional<EventDescriptionObject> getForEvent(const event_log_t &event);

  private:
	/**
	 * Returns a pointer to the map holding events for given source,
	 * or nullptr if the source is unknown
	 */
	std::map<uint8_t, EventDescriptionObject> *getMapForSource (event_log_source_t src);

	/**
	 * If for some reason a constructor will fail to parse
	 * given input JSON, this will be set to false and
	 * application will not decode any incoming
	 * event.
	 */
	bool m_ConstructorSuccess;

	std::map<uint8_t, EventDescriptionObject> m_SourceMain;
	std::map<uint8_t, EventDescriptionObject> m_SourceWxHandler;
	std::map<uint8_t, EventDescriptionObject> m_SourcePowerSave;
	std::map<uint8_t, EventDescriptionObject> m_SourcePacketTxHandler;
	std::map<uint8_t, EventDescriptionObject> m_SourceAprsis;
	std::map<uint8_t, EventDescriptionObject> m_SourceKiss;
	std::map<uint8_t, EventDescriptionObject> m_SourceAprsRf;
	std::map<uint8_t, EventDescriptionObject> m_SourceGsmGprs;
	std::map<uint8_t, EventDescriptionObject> m_SourceTcpip;
	std::map<uint8_t, EventDescriptionObject> m_SourceHttpClient;
	std::map<uint8_t, EventDescriptionObject> m_SourceModbus;
	std::map<uint8_t, EventDescriptionObject> m_SourceUmb;
	std::map<uint8_t, EventDescriptionObject> m_SourceDrvAnemometer;
	std::map<uint8_t, EventDescriptionObject> m_SourceDrvI2c;
	std::map<uint8_t, EventDescriptionObject> m_SourceDrvUart;
	std::map<uint8_t, EventDescriptionObject> m_SourceDrvSpi;
	std::map<uint8_t, EventDescriptionObject> m_SourceDrvFanet;
};

#endif /* C3D1F7A2_8E4B_4C6D_9A15_2B7E0F3C8D41 */
