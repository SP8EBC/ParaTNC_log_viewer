#include "serial_io.hpp"
#include "shared/crc_.h"
#include <cstring>
#include <iomanip>
#include <iostream>

namespace {
constexpr uint32_t SERIAL_BAUDRATE = 115200;
constexpr uint32_t SERIAL_READ_TIMEOUT_MS = 10000;

constexpr uint32_t EVENT_CRC_POLY = 0x04C11DB7;
constexpr uint32_t EVENT_CRC_SEED = 0xFFFFFFFF;
}

SerialIo::SerialIo (const std::string &portName) : m_portName{portName}
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
        std::cout << "--- serial port is not open" << std::endl;
		return std::nullopt;
	}

	std::vector<uint8_t> buffer;

	size_t bytesRead = 0;

	try {
		bytesRead = m_rawSerialPort.read (buffer, kPacketSize);
	}
	catch (const std::exception &ex) {
        std::cout << "--- std::exception while reading from serial port" << std::endl;
        std::cout << ex.what() << std::endl;
		return std::nullopt;
	}

	if (bytesRead != kPacketSize) {
		std::cout << "--- either a read timeout occurred or a malformed / incomplete frame was received" << std::endl;
		return std::nullopt;
	}

	event_log_t event;
	std::memcpy (&event, buffer.data (), kPacketSize);

	const uint32_t crc = calcCRC32std (buffer.data (), kPacketSize - 1, EVENT_CRC_POLY,
										EVENT_CRC_SEED, 0, 0, 0);

	if (static_cast<uint8_t> (crc & 0xFF) != event.crc_checksum) {
        std::cout << "--- CRC checksum is not correct, calculated: " << std::hex << (crc & 0xFF) << ", expected: " << event.crc_checksum  << std::endl;

        std::cout << std::hex << std::setfill ('0');
        for (size_t i = 0; i < buffer.size (); i++) {
	        std::cout << std::setw (2) << static_cast<unsigned int> (buffer[i]) << ' ';
        }
        std::cout << std::dec << std::endl;

		return std::nullopt;
	}

	return event;
}
