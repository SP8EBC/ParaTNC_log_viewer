#ifndef FB0C505A_4BEF_41ED_B46A_F97CF41AECDF
#define FB0C505A_4BEF_41ED_B46A_F97CF41AECDF

#include "serial/serial.h"
#include "shared/event_log_t.h"
#include <optional>
#include <string>

class SerialIo {
  public:
	SerialIo (const std::string &portName);

	/**
	 * Opens a serial port passed to a constructor
	 */
	bool open ();

	/**
	 * Closes the serial port
	 */
	bool close ();

	/**
	 * receive single event log send from the controller to the host
	 * pc via serial port.
	 * @note each event is send in raw form, directly read from NVM / SRAM memory in the controller,
	 * where it is stored as packed structure, as defined by event_log_t type. The same structure
	 * definition is used here and in the embedded side in the controller. Embedded code, which
	 persist
	 * any new event on the controller side is here:
	 https://github.com/SP8EBC/ParaTNC/blob/master/src/nvm/nvm_event.c
	 * @warning each log entry is protected by a CRC checksum calculated by implementation in
	 ./src/shared/crc_.c
	 * @return new object resembles received log entry or empty optional in case of error
	 *
	 * snipped of a code with CRC calculation on embedded controler side
	 *
	 * 				// calculate crc checksum for this entry
				const uint32_t crc = calcCRC32std (current,
												   sizeof (event_log_t) - 1,
												   0x04C11DB7,
												   0xFFFFFFFF,
												   0,
												   0,
												   0);
	 *
	 *
	 */
	std::optional<event_log_t> receiveSingleEvent ();

  private:
	static constexpr size_t kPacketSize = sizeof (event_log_t);
	std::string m_portName;
	serial::Serial m_rawSerialPort;
};

#endif /* FB0C505A_4BEF_41ED_B46A_F97CF41AECDF */
