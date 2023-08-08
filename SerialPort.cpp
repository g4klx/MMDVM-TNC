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
m_inFrame(false),
m_isEscaped(false),
m_debug(false)
{
}

void CSerialPort::start()
{
  beginInt(1U, SERIAL_SPEED);
#if defined(SERIAL_DEBUGGING)
  beginInt(3U, DEBUGGING_SPEED);
#endif

  ax25TX.setTXDelay(TX_DELAY / 10U);
  ax25RX.setPPersistence(P_PERSISTENCE);
  ax25RX.setSlotTime(SLOT_TIME / 10U);
}

void CSerialPort::process()
{
  while (availableForReadInt(1U)) {
    uint8_t c = readInt(1U);

    if (!m_inFrame) {
      if (c == KISS_FEND) {
        // Handle the frame start
        m_inFrame   = true;
        m_isEscaped = false;
        m_ptr       = 0U;
      }
    } else {
      // Any other bytes are added to the buffer-ish
      switch (c) {
        case KISS_TFESC:
          m_buffer[m_ptr++] = m_isEscaped ? KISS_FESC : KISS_TFESC;
          m_isEscaped = false;
          break;
        case KISS_TFEND:
          m_buffer[m_ptr++] = m_isEscaped ? KISS_FEND : KISS_TFEND;
          m_isEscaped = false;
          break;
        case KISS_FESC:
          m_isEscaped = true;
          break;
        case KISS_FEND:
          if (m_ptr > 0U)
            processMessage();
          m_inFrame   = false;
          m_isEscaped = false;
          m_ptr       = 0U;
          break;
        default:
          m_buffer[m_ptr++] = c;
          break;
      }
    }
  }
}

void CSerialPort::processMessage()
{
  switch (m_buffer[0U]) {
    case KISS_TYPE_DATA:
      ax25TX.writeData(m_buffer + 1U, m_ptr - 1U);
      break;
    case KISS_TYPE_TX_DELAY:
      ax25TX.setTXDelay(m_buffer[1U]);
      DEBUG2("Setting TX delay to", m_buffer[1U]);
      break;
    case KISS_TYPE_P_PERSISTENCE:
      ax25RX.setPPersistence(m_buffer[1U]);
      DEBUG2("Setting p-persistence to", m_buffer[1U]);
      break;
    case KISS_TYPE_SLOT_TIME:
      ax25RX.setSlotTime(m_buffer[1U]);
      DEBUG2("Setting slot time to", m_buffer[1U]);
      break;
//  case KISS_TYPE_TX_TAIL:
//    break;
    case KISS_TYPE_FULL_DUPLEX:
      m_duplex = m_buffer[1U] != 0U;
      DEBUG2("Setting full duplex to", m_buffer[1U]);
      break;
    case KISS_TYPE_SET_HARDWARE:
      m_mode = m_buffer[1U];
      DEBUG2("Setting mode to", m_buffer[1U]);
      break;
    case KISS_TYPE_DATA_WITH_ACK: {
        uint16_t token = (m_buffer[1U] << 8) + (m_buffer[2U] << 0);
        ax25TX.writeDataAck(token, m_buffer + 3U, m_ptr - 3U);
      }
      break;
    default:
      DEBUG2("Unhandled KISS frame type", m_buffer[0U]);
      break;
  }
}

void CSerialPort::writeKISSData(uint8_t type, const uint8_t* data, uint16_t length)
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

void CSerialPort::writeKISSAck(uint16_t token)
{
  writeKISSData(KISS_TYPE_ACK, (uint8_t*)&token, sizeof(uint16_t));
}

void CSerialPort::writeDebug(const char* text)
{
#if defined(SERIAL_DEBUGGING)
  writeInt(3U, (uint8_t*)text, ::strlen(text));
#endif
}

void CSerialPort::writeDebug(const char* text, int16_t n1)
{
#if defined(SERIAL_DEBUGGING)
  writeDebug(text);
  writeDebug(" ");
  writeDebug(n1);
#endif
}

void CSerialPort::writeDebug(const char* text, int16_t n1, int16_t n2)
{
#if defined(SERIAL_DEBUGGING)
  writeDebug(text);
  writeDebug(" ");
  writeDebug(n1);
  writeDebug(" ");
  writeDebug(n2);
#endif
}

void CSerialPort::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3)
{
#if defined(SERIAL_DEBUGGING)
  writeDebug(text);
  writeDebug(" ");
  writeDebug(n1);
  writeDebug(" ");
  writeDebug(n2);
  writeDebug(" ");
  writeDebug(n3);
#endif
}

void CSerialPort::writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4)
{
#if defined(SERIAL_DEBUGGING)
  writeDebug(text);
  writeDebug(" ");
  writeDebug(n1);
  writeDebug(" ");
  writeDebug(n2);
  writeDebug(" ");
  writeDebug(n3);
  writeDebug(" ");
  writeDebug(n4);
#endif
}

void CSerialPort::writeDebug(int16_t num)
{
  if (num == 0) {
    writeDebug("0");
    return;
  }

  bool isNegative = false;

  if (num < 0) {
    isNegative = true;
    num = -num;
  }

  char buffer[10U];
  uint8_t pos = 0U;

  while (num != 0) {
    int16_t rem = num % 10;
    buffer[pos++] = rem + '0';
    num /= 10;
  }

  if (isNegative)
    buffer[pos++] = '-';

  buffer[pos] = '\0';

  reverse(buffer, pos);

  writeDebug(buffer);
}

void CSerialPort::reverse(char* buffer, uint8_t length) const
{
  uint8_t start = 0U;
  uint8_t end = length - 1U;

  while (start < end) {
    char temp     = buffer[start];
    buffer[start] = buffer[end];
    buffer[end]   = temp;

    end--;
    start++;
  }
}
