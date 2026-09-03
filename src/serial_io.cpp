#include "serial_io.hpp"
#include "shared/crc_.h"
#include <cstring>
#include <iostream>

namespace {
constexpr uint32_t SERIAL_BAUDRATE = 115200;
constexpr uint32_t SERIAL_READ_TIMEOUT_MS = 1000;

constexpr uint32_t EVENT_CRC_POLY = 0x04C11DB7;
constexpr uint32_t EVENT_CRC_SEED = 0xFFFFFFFF;
}

SerialIo::SerialIo (std::string &portName) : m_portName{portName}
{
	m_rawSerialPort.setPort (m_portName);
	m_rawSerialPort.setBaudrate (SERIAL_BAUDRATE);

	serial::Timeout timeout = serial::Timeout::simpleTimeout (SERIAL_READ_TIMEOUT_MS);
	m_rawSerialPort.setTimeout (timeout);
}

bool SerialIo::open ()
{
	if (m_rawSerialPort.isOpen ()) {
		return true;
	}

	try {
		m_rawSerialPort.open ();
	}
	catch (const std::exception &) {
		return false;
	}

	return m_rawSerialPort.isOpen ();
}

bool SerialIo::close ()
{
	if (!m_rawSerialPort.isOpen ()) {
		return true;
	}

	try {
		m_rawSerialPort.close ();
	}
	catch (const std::exception &) {
		return false;
	}

	return !m_rawSerialPort.isOpen ();
}

std::optional<event_log_t> SerialIo::receiveSingleEvent ()
{
	if (!m_rawSerialPort.isOpen ()) {
		return std::nullopt;
	}

	std::vector<uint8_t> buffer;

	size_t bytesRead = 0;

	try {
		bytesRead = m_rawSerialPort.read (buffer, sizeof (event_log_t));
	}
	catch (const std::exception &ex) {
        std::cout << "--- std::exception while reading from serial port" << std::endl;
        std::cout << ex.what() << std::endl;
		return std::nullopt;
	}

	if (bytesRead != sizeof (event_log_t)) {
		std::cout << "--- either a read timeout occurred or a malformed / incomplete frame was received" << std::endl;
		return std::nullopt;
	}

	event_log_t event;
	std::memcpy (&event, buffer.data (), sizeof (event_log_t));

	const uint32_t crc = calcCRC32std (buffer.data (), sizeof (event_log_t) - 1, EVENT_CRC_POLY,
										EVENT_CRC_SEED, 0, 0, 0);

	if (static_cast<uint8_t> (crc & 0xFF) != event.crc_checksum) {
		return std::nullopt;
	}

	return event;
}
