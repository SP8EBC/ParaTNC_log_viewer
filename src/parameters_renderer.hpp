#ifndef FC4A2BF5_2699_4F23_84A5_BD9EA9935180
#define FC4A2BF5_2699_4F23_84A5_BD9EA9935180
#include "event_description_object.hpp"
#include "event_log_t.h"

#include <optional>
#include <string>

class ParametersRenderer {
  public:
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
	static const std::string getParameterStr (
		const event_log_t &event, bool skipZeroValues = false,
		const std::optional<EventDescriptionObject> &descriptionFromJson = std::nullopt,
		std::optional<int> parameterNumber = std::nullopt);

	/**
	 * Estimates if the printout of this event will be long enough that it shall be split
	 * to multiple lines. Each parameter in multiple line
	 */
	static bool estimateEventPrintoutLn (const EventDescriptionObject &obj,
										 const event_log_t &event, const int maximumLineLn);

  private:
	/**
	 * Returns a string if this class has hardcoded member function, which can decode
	 * exact meaning of each parameter, rescale it and print into this returned string
	 * @param event to check
	 * @return string with custom-formatted parameters or nullopt if no custom format is available
	 */
	static std::optional<std::string> haveCustomParametersDescription (const event_log_t &event);

	/**
	 * Renders a specific representation of an event parameters using description from JSON file.
	 * Only parameters which are used to store something will be printed, each with own,
	 * name. By default, a tool which create this JSON puts a variable name put into a call
	 * to log function
	 * @param obj description from JSON
	 * @param event event log to stringify
	 */
	static std::string parameterFromJsonEventDescription (const EventDescriptionObject &obj,
														  const event_log_t &event);

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
	static const std::string
	singleParameter (const event_log_t &event,
						   const EventDescriptionObject &descriptionFromJson, int parameter);

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
	static std::string genericParameter (const event_log_t &event, bool skipZeroValues = false);
};

#endif /* FC4A2BF5_2699_4F23_84A5_BD9EA9935180 */
