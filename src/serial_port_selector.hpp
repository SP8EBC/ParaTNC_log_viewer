#ifndef A3D6E2B1_9C4F_4C1A_8F0E_5B6D2E7A1C90
#define A3D6E2B1_9C4F_4C1A_8F0E_5B6D2E7A1C90

#include <string>

class SerialPortSelector {
  public:
	/**
	 * Lists serial ports available on the system and asks the user to pick one interactively.
	 * @return port name chosen by the user (e.g. "/dev/ttyUSB0")
	 */
	static std::string promptForSerialPort ();

  private:
	/**
	 * The legacy 8250/16550 UART driver always registers /dev/ttyS0..ttyS31 at boot,
	 * regardless of whether real hardware is wired up behind each one. This checks
	 * TIOCGSERIAL to tell a real UART from a phantom node; ports other than /dev/ttyS*
	 * (e.g. ttyUSB*, ttyACM*) only ever exist when real hardware is attached, so they
	 * are always reported as present.
	 */
	static bool isRealSerialPort (const std::string &port);
};

#endif /* A3D6E2B1_9C4F_4C1A_8F0E_5B6D2E7A1C90 */
