#include "parameters_renderer.hpp"

#include <sstream>

const std::string ParametersRenderer::getParameterStr (const event_log_t &ev, bool skipZeroValues)
{
	if (haveCustomParametersDescription (ev)) {
		// TODO: dispatch to a per-event custom formatter once implemented
		return genericParameter (ev, skipZeroValues);
	}

	return genericParameter (ev, skipZeroValues);
}

bool ParametersRenderer::haveCustomParametersDescription (const event_log_t &event)
{
	// TODO: fixme
	return false; // for simplicity as for now
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
