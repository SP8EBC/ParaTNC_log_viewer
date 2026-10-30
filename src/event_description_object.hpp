#ifndef B6E8A981_2605_4EF8_892F_538BD72403EA
#define B6E8A981_2605_4EF8_892F_538BD72403EA

#include "shared/event_log_t.h"
#include <string>

/**
 * This structure resembles an object from an array in JSON file
 *
 * {
 * 	  "severity": 3,
 * 	  "source": 4,
 * 	  "eventId": 8,
 * 	  "eventString": "Wrong TCPIP state to connect to server",
 * 	  "param": "gsm_comm_state_get_current()",
 * 	  "param2": "0",
 * 	  "wparam": "0",
 * 	  "wparam2": "0",
 * 	  "wparam3": "",
 * 	  "lparam": "0",
 * 	  "lparam2": "0",
 * 	  "asHexParam": true,
 * 	  "asHexParam2": true,
 * 	  "asHexWparam": true,
 * 	  "asHexWparam2": true,
 * 	  "asHexWparam3": true,
 * 	  "asHexLparam": true,
 * 	  "asHexLparam2": true
 * 	},
 *
 * in the case above only 'param' is printed. The rest (both label and value) is hidden because
 * either label is an empty string (like wparam3) or it is set to '0'
 */
struct EventDescriptionObject {
	event_log_severity_t severity; //!< "severity": 3, -> direct cast to enum value. Exception is
								   //!< thrown on unknown value
	event_log_source_t source; //!<  "source": 4,  -> direct cast to enum value. Exception is thrown
							   //!<  on unknown value
	uint8_t eventId;		   //!< "eventId": 8, -> normally not displayed
	std::string eventString;   //!< "eventString": "Wrong TCPIP state to connect to server",
	std::string param;		   //!< This is a label of 'param' not the value itself!!
	std::string param2;		   //!< This is a label of 'param2' not the value itself!!
	std::string wparam;		   //!< This is a label of 'wparam' not the value itself!!
	std::string wparam2;	   //!< This is a label of 'wparam' not the value itself!!
	std::string wparam3;	   //!< This is a label of 'wparam' not the value itself!!
	std::string lparam;		   //!< This is a label of 'lparam' not the value itself!!
	std::string lparam2;	   //!< This is a label of 'lparam2' not the value itself!!
	bool asHexParam;  //!< if set to true, a value of 'param' is displayed as hex, not decimal
	bool asHexParam2; //!< same as above but for 'param2'
	bool asHexWparam;
	bool asHexWparam2;
	bool asHexWparam3;
	bool asHexLparam;
	bool asHexLParam2;

	bool operator== (const EventDescriptionObject &) const = default;
};

#endif /* B6E8A981_2605_4EF8_892F_538BD72403EA */
