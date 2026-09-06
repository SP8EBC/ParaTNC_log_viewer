#include "serial_port_selector.hpp"

#include "serial/serial.h"

#pragma push_macro("U")
#undef U
// pragma required as a workaround of possible conflict with cpprestsdk.
// more info here: https://github.com/fmtlib/fmt/issues/3330
#include "spdlog/spdlog.h"
#pragma pop_macro("U")

#include <algorithm>
#include <iostream>
#include <vector>

#if defined(__linux__)
#include <fcntl.h>
#include <linux/serial.h>
#include <sys/ioctl.h>
#include <unistd.h>

bool SerialPortSelector::isRealSerialPort (const std::string &port)
{
	static const std::string kLegacyPrefix = "/dev/ttyS";
	if (port.compare (0, kLegacyPrefix.size (), kLegacyPrefix) != 0) {
		return true;
	}

	const int fd = open (port.c_str (), O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		return true;
	}

	struct serial_struct serialInfo {};
	const bool isReal = (ioctl (fd, TIOCGSERIAL, &serialInfo) == 0 && serialInfo.type != PORT_UNKNOWN);
	close (fd);

	return isReal;
}
#else
bool SerialPortSelector::isRealSerialPort (const std::string &port)
{
	// Windows' SetupAPI-based enumeration (see list_ports_win.cc) only ever
	// reports COM ports actually registered on the system, unlike Linux's
	// legacy 8250 driver which always exposes /dev/ttyS0..ttyS31 regardless
	// of hardware presence -- so no extra filtering is needed here.
	(void)port;
	return true;
}
#endif

std::string SerialPortSelector::promptForSerialPort ()
{
	std::vector<serial::PortInfo> ports = serial::list_ports ();
	ports.erase (std::remove_if (ports.begin (),
								  ports.end (),
								  [] (const serial::PortInfo &p) { return !isRealSerialPort (p.port); }),
				 ports.end ());

	if (ports.empty ()) {
		SPDLOG_ERROR ("No serial ports found on this system!");
		exit (-3);
	}

	std::cout << "No serial port given on the command line (--port). Available serial ports:"
			  << std::endl;
	for (std::size_t i = 0; i < ports.size (); i++) {
		std::cout << "  [" << i << "] " << ports[i].port;
		if (!ports[i].description.empty ()) {
			std::cout << " - " << ports[i].description;
		}
		std::cout << std::endl;
	}

	while (true) {
		std::cout << "Select serial port [0-" << (ports.size () - 1) << "]: ";
		std::string line;
		if (!std::getline (std::cin, line)) {
			SPDLOG_ERROR ("Failed to read serial port selection from stdin!");
			exit (-3);
		}

		try {
			const std::size_t idx = static_cast<std::size_t> (std::stoul (line));
			if (idx < ports.size ()) {
				return ports[idx].port;
			}
		}
		catch (const std::exception &) {
			// fallthrough to invalid selection message below
		}

		std::cout << "Invalid selection, please try again." << std::endl;
	}
}
