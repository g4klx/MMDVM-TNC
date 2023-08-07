/*
 *   Copyright (C) 2013,2015-2021,2023 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016 by Colin Durbridge G4EML
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "Config.h"
#include "Globals.h"
#include "KISSDefines.h"

#if defined(MADEBYMAKEFILE)
#include "GitVersion.h"
#endif

#include "SerialPort.h"
#include "Version.h"


#if EXTERNAL_OSC == 12000000
#define TCXO "12.0000 MHz"
#elif EXTERNAL_OSC == 12288000
#define TCXO "12.2880 MHz"
#elif EXTERNAL_OSC == 14400000
#define TCXO "14.4000 MHz"
#elif EXTERNAL_OSC == 19200000
#define TCXO "19.2000 Mhz"
#else
#define TCXO "NO TCXO"
#endif

#if defined(DRCC_DVM_NQF)
#define	HW_TYPE	"MMDVM DRCC_DVM_NQF"
#elif defined(DRCC_DVM_HHP446)
#define	HW_TYPE	"MMDVM DRCC_DVM_HHP(446)"
#elif defined(DRCC_DVM_722)
#define HW_TYPE "MMDVM RB_STM32_DVM(722)"
#elif defined(DRCC_DVM_446)
#define HW_TYPE "MMDVM RB_STM32_DVM(446)"
#else
#define	HW_TYPE	"MMDVM"
#endif

#if defined(GITVERSION)
#define concat(h, a, b, c) h " " a " " b " GitID #" c ""
const char HARDWARE[] = concat(HW_TYPE, VERSION, TCXO, GITVERSION);
#else
#define concat(h, a, b, c, d) h " " a " " b " (Build: " c " " d ")"
const char HARDWARE[] = concat(HW_TYPE, VERSION, TCXO, __TIME__, __DATE__);
#endif

const uint8_t PROTOCOL_VERSION   = 2U;

// Parameters for batching serial data
const int      MAX_SERIAL_DATA  = 250;
const uint16_t MAX_SERIAL_COUNT = 100U;

CSerialPort::CSerialPort() :
m_buffer(),
m_ptr(0U),
m_debug(false)
{
}

void CSerialPort::start()
{
  beginInt(1U, SERIAL_SPEED);
}

void CSerialPort::process()
{
  while (availableForReadInt(1U)) {
    uint8_t c = readInt(1U);

    if (m_ptr == 0U) {
      if (c == KISS_FEND) {
        // Handle the frame start correctly
        m_buffer[m_ptr++] = c;
      }
    } else {
      // Any other bytes are added to the buffer
      m_buffer[m_ptr++] = c;

      if (c == KISS_FEND) {
        if (m_ptr > 2U)
          processMessage();
        m_ptr = 0U;
      }
    }
  }
}

void CSerialPort::processMessage()
{
  switch (m_buffer[1U]) {
    case KISS_TYPE_DATA:
      break;
    case KISS_TYPE_TX_DELAY:
      break;
    case KISS_TYPE_P_PERSISTENCE:
      break;
    case KISS_TYPE_SLOT_TIME:
      break;
    case KISS_TYPE_TX_TAIL:
      break;
    case KISS_TYPE_FULL_DUPLEX:
      break;
    case KISS_TYPE_DATA_WITH_ACK:
      break;
    default:
      DEBUG2("Unhandled KISS frame type", m_buffer[1U]);
      break;
  }
}

void CSerialPort::writeAX25Data(uint8_t type, const uint8_t* data, uint16_t length)
{
  uint8_t  buffer[512U];
  uint16_t pos = 0U;

  buffer[pos++] = KISS_FEND;

  buffer[pos++] = type;

  for (uint16_t i = 0U; i < length; i++) {
    switch (data[i]) {
      case KISS_FEND:
        buffer[pos++] = KISS_FESC;
        buffer[pos++] = KISS_TFEND;
        break;
      case KISS_FESC:
        buffer[pos++] = KISS_FESC;
        buffer[pos++] = KISS_TFESC;
        break;
      default:
        buffer[pos++] = data[i];
        break;
    }
  }

  buffer[pos++] = KISS_FEND;

  writeInt(1U, buffer, pos);
}

void CSerialPort::writeDebug(const char* text)
{
/*
  if (!m_debug)
    return;

  uint8_t reply[130U];

  reply[0U] = MMDVM_FRAME_START;
  reply[1U] = 0U;
  reply[2U] = MMDVM_DEBUG1;

  uint8_t count = 3U;
  for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
    reply[count] = text[i];

  reply[1U] = count;

  writeInt(1U, reply, count, true);
*/
}

void CSerialPort::writeDebug(const char* text, int16_t n1)
{
/*
  if (!m_debug)
    return;

  uint8_t reply[130U];

  reply[0U] = MMDVM_FRAME_START;
  reply[1U] = 0U;
  reply[2U] = MMDVM_DEBUG2;

  uint8_t count = 3U;
  for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
    reply[count] = text[i];

  reply[count++] = (n1 >> 8) & 0xFF;
  reply[count++] = (n1 >> 0) & 0xFF;

  reply[1U] = count;

  writeInt(1U, reply, count, true);
*/
}

void CSerialPort::writeDebug(const char* text, int16_t n1, int16_t n2)
{
/*
  if (!m_debug)
    return;

  uint8_t reply[130U];

  reply[0U] = MMDVM_FRAME_START;
  reply[1U] = 0U;
  reply[2U] = MMDVM_DEBUG3;

  uint8_t count = 3U;
  for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
    reply[count] = text[i];

  reply[count++] = (n1 >> 8) & 0xFF;
  reply[count++] = (n1 >> 0) & 0xFF;

  reply[count++] = (n2 >> 8) & 0xFF;
  reply[count++] = (n2 >> 0) & 0xFF;

  reply[1U] = count;

  writeInt(1U, reply, count, true);
*/
}

void CSerialPort::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3)
{
/*
  if (!m_debug)
    return;

  uint8_t reply[130U];

  reply[0U] = MMDVM_FRAME_START;
  reply[1U] = 0U;
  reply[2U] = MMDVM_DEBUG4;

  uint8_t count = 3U;
  for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
    reply[count] = text[i];

  reply[count++] = (n1 >> 8) & 0xFF;
  reply[count++] = (n1 >> 0) & 0xFF;

  reply[count++] = (n2 >> 8) & 0xFF;
  reply[count++] = (n2 >> 0) & 0xFF;

  reply[count++] = (n3 >> 8) & 0xFF;
  reply[count++] = (n3 >> 0) & 0xFF;

  reply[1U] = count;

  writeInt(1U, reply, count, true);
*/
}

void CSerialPort::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4)
{
/*
  if (!m_debug)
    return;

  uint8_t reply[130U];

  reply[0U] = MMDVM_FRAME_START;
  reply[1U] = 0U;
  reply[2U] = MMDVM_DEBUG5;

  uint8_t count = 3U;
  for (uint8_t i = 0U; text[i] != '\0'; i++, count++)
    reply[count] = text[i];

  reply[count++] = (n1 >> 8) & 0xFF;
  reply[count++] = (n1 >> 0) & 0xFF;

  reply[count++] = (n2 >> 8) & 0xFF;
  reply[count++] = (n2 >> 0) & 0xFF;

  reply[count++] = (n3 >> 8) & 0xFF;
  reply[count++] = (n3 >> 0) & 0xFF;

  reply[count++] = (n4 >> 8) & 0xFF;
  reply[count++] = (n4 >> 0) & 0xFF;

  reply[1U] = count;

  writeInt(1U, reply, count, true);
*/
}

void CSerialPort::writeDebugDump(const uint8_t* data, uint16_t length)
{
/*
  uint8_t reply[512U];

  reply[0U] = MMDVM_FRAME_START;

  if (length > 252U) {
    reply[1U] = 0U;
    reply[2U] = (length + 4U) - 255U;
    reply[3U] = MMDVM_DEBUG_DUMP;

    for (uint16_t i = 0U; i < length; i++)
      reply[i + 4U] = data[i];

    writeInt(1U, reply, length + 4U);
  } else {
    reply[1U] = length + 3U;
    reply[2U] = MMDVM_DEBUG_DUMP;

    for (uint16_t i = 0U; i < length; i++)
      reply[i + 3U] = data[i];

    writeInt(1U, reply, length + 3U);
  }
*/
}


