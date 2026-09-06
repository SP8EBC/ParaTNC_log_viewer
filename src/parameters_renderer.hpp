#ifndef FC4A2BF5_2699_4F23_84A5_BD9EA9935180
#define FC4A2BF5_2699_4F23_84A5_BD9EA9935180
#include "event_log_t.h"

#include <optional>
#include <string>

class ParametersRenderer {
  public:
	/**
	 * Renders all parameters (param, param2...) into a printable string to be
	 * displayed by spdlog
	 * @param event to process
	 * @param skipZeroValues if true, parameters equal to zero are omitted from the output
	 * @return string to be displayed by spdlog
	 */
	static const std::string getParameterStr (const event_log_t &event,
											  bool skipZeroValues = false);

  private:
	/**
	 * Returns a string if this class has hardcoded member function, which can decode
	 * exact meaning of each parameter, rescale it and print into this returned string
	 * @param event to check
	 * @return string with custom-formatted parameters or nullopt if no custom format is available 
	 */
	static std::optional<std::string> haveCustomParametersDescription (const event_log_t &event);

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
