/*
 *   Copyright (C) 2023 by Jonathan Naylor G4KLX
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
#include "IL2PTX.h"

const uint8_t BIT_MASK_TABLE1[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE1[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE1[(i)&7])
#define READ_BIT1(p,i)    (p[(i)>>3] & BIT_MASK_TABLE1[(i)&7])

const uint8_t BIT_MASK_TABLE2[] = { 0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U };

#define WRITE_BIT2(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE2[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE2[(i)&7])
#define READ_BIT2(p,i)    (p[(i)>>3] & BIT_MASK_TABLE2[(i)&7])

CIL2PTX::CIL2PTX() :
m_frame(false),
m_poBuffer(),
m_poLen(0U),
m_poPtr(0U),
m_tablePtr(0U),
m_tokens()
{
}

void CIL2PTX::process()
{
  if (m_poLen == 0U) {
    for (const auto& token : m_tokens)
      serial.writeKISSAck(token);
    m_tokens.clear();
    return;
  }

  if (!m_duplex) {
    if (m_poPtr == 0U) {
      bool tx = il2pRX.canTX();
      if (!tx)
        return;
    }
  }
/*
  uint16_t space = io.getSpace();

  while (space > AX25_RADIO_SYMBOL_LENGTH) {
    bool b = READ_BIT1(m_poBuffer, m_poPtr) != 0U;
    m_poPtr++;

    writeBit(b);

    space -= AX25_RADIO_SYMBOL_LENGTH;

    if (m_poPtr >= m_poLen) {
      m_poPtr = 0U;
      m_poLen = 0U;
      return;
    }
  }
*/
}

uint8_t CIL2PTX::writeData(const uint8_t* data, uint16_t length)
{
/*
  m_poLen    = 0U;
  m_poPtr    = 0U;
  m_tablePtr = 0U;

  // Add TX delay
  for (uint16_t i = 0U; i < m_txDelay; i++, m_poLen++) {
    bool preamble = NRZI(false);
    WRITE_BIT1(m_poBuffer, m_poLen, preamble);
  }

  // Add the Start Flag
  for (uint16_t i = 0U; i < 8U; i++, m_poLen++) {
    bool b1 = READ_BIT1(START_FLAG, i) != 0U;
    bool b2 = NRZI(b1);
    WRITE_BIT1(m_poBuffer, m_poLen, b2);
  }

  uint8_t ones = 0U;
  for (uint16_t i = 0U; i < (frame.m_length * 8U); i++) {
    bool b1 = READ_BIT2(frame.m_data, i) != 0U;
    bool b2 = NRZI(b1);
    WRITE_BIT1(m_poBuffer, m_poLen, b2);
    m_poLen++;

    if (b1) {
      ones++;
      if (ones == AX25_MAX_ONES) {
        // Bit stuffing
        bool b = NRZI(false);
        WRITE_BIT1(m_poBuffer, m_poLen, b);
        m_poLen++;
        ones = 0U;
      }
    } else {
      ones = 0U;
    }
  }
*/
  return 0U;
}

uint8_t CIL2PTX::writeDataAck(uint16_t token, const uint8_t* data, uint16_t length)
{
  m_tokens.push_back(token);

  return writeData(data, length);
}

void CIL2PTX::writeBit(bool b)
{
/*
  q15_t buffer[AX25_RADIO_SYMBOL_LENGTH];
  for (uint8_t i = 0U; i < AX25_RADIO_SYMBOL_LENGTH; i++) {
    q15_t value = AUDIO_TABLE_DATA[m_tablePtr];

    if (b) {
      // De-emphasise the lower frequency by 6dB
      value >>= 2;
      m_tablePtr += 6U;
    } else {
      m_tablePtr += 11U;
    }

    buffer[i] = value >> 1;

    if (m_tablePtr >= AUDIO_TABLE_LEN)
      m_tablePtr -= AUDIO_TABLE_LEN;
  }

  io.write(buffer, AX25_RADIO_SYMBOL_LENGTH);
*/
}

uint8_t CIL2PTX::getSpace() const
{
  return m_poLen == 0U ? 255U : 0U;
}

