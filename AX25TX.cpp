/*
 *   Copyright (C) 2020,2023 by Jonathan Naylor G4KLX
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
#include "AX25TX.h"

#include "AX25Defines.h"
#include "AX25Frame.h"


const uint8_t START_FLAG[] = { AX25_FRAME_START };
const uint8_t END_FLAG[]   = { AX25_FRAME_END };

const uint8_t BIT_MASK_TABLE1[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE1[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE1[(i)&7])
#define READ_BIT1(p,i)    (p[(i)>>3] & BIT_MASK_TABLE1[(i)&7])

const uint8_t BIT_MASK_TABLE2[] = { 0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U };

#define WRITE_BIT2(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE2[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE2[(i)&7])
#define READ_BIT2(p,i)    (p[(i)>>3] & BIT_MASK_TABLE2[(i)&7])

const uint16_t AUDIO_TABLE_LEN = 240U;

const q15_t AUDIO_TABLE_DATA[] = {
  0, 107, 214, 321, 428, 535, 641, 746, 851, 956, 1060, 1163, 1265, 1367, 1468, 1567, 1666, 1763, 1859, 1954, 2047, 2140,
  2230, 2319, 2407, 2493, 2577, 2659, 2740, 2819, 2896, 2970, 3043, 3114, 3182, 3249, 3313, 3375, 3434, 3492, 3546, 3599,
  3649, 3696, 3741, 3783, 3823, 3860, 3895, 3926, 3955, 3982, 4006, 4026, 4045, 4060, 4073, 4082, 4089, 4094, 4095, 4094,
  4089, 4082, 4073, 4060, 4045, 4026, 4006, 3982, 3955, 3926, 3895, 3860, 3823, 3783, 3741, 3696, 3649, 3599, 3546, 3492,
  3434, 3375, 3313, 3249, 3182, 3114, 3043, 2970, 2896, 2819, 2740, 2659, 2577, 2493, 2407, 2319, 2230, 2140, 2047, 1954,
  1859, 1763, 1666, 1567, 1468, 1367, 1265, 1163, 1060, 956, 851, 746, 641, 535, 428, 321, 214, 107, -0, -107, -214, -321,
  -428, -535, -641, -746, -851, -956, -1060, -1163, -1265, -1367, -1468, -1567, -1666, -1763, -1859, -1954, -2048, -2140,
  -2230, -2319, -2407, -2493, -2577, -2659, -2740, -2819, -2896, -2970, -3043, -3114, -3182, -3249, -3313, -3375, -3434,
  -3492, -3546, -3599, -3649, -3696, -3741, -3783, -3823, -3860, -3895, -3926, -3955, -3982, -4006, -4026, -4045, -4060,
  -4073, -4082, -4089, -4094, -4095, -4094, -4089, -4082, -4073, -4060, -4045, -4026, -4006, -3982, -3955, -3926, -3895,
  -3860, -3823, -3783, -3741, -3696, -3649, -3599, -3546, -3492, -3434, -3375, -3313, -3249, -3182, -3114, -3043, -2970,
  -2896, -2819, -2740, -2659, -2577, -2493, -2407, -2319, -2230, -2140, -2047, -1954, -1859, -1763, -1666, -1567, -1468,
  -1367, -1265, -1163, -1060, -956, -851, -746, -641, -535, -428, -321, -214, -107};

CAX25TX::CAX25TX() :
m_poBuffer(),
m_poLen(0U),
m_poPtr(0U),
m_tablePtr(0U),
m_nrzi(false),
m_level(MODE1_TX_LEVEL * 128),
m_txDelay((TX_DELAY / 10U) * 24U),
m_tokens()
{
}

void CAX25TX::process()
{
  if (!m_duplex) {
    // Nothing left to transmit, send the packet tokens back
    if (!m_tx && (m_poLen == 0U)) {
      for (const auto& token : m_tokens)
        serial.writeKISSAck(token);
      m_tokens.clear();
      return;
    }
  } else {
    // Send the tokens back immediately as the packets can be transmitted immediately too
    for (const auto& token : m_tokens)
      serial.writeKISSAck(token);
    m_tokens.clear();
    if (m_poLen == 0U)
      return;
  }

  if (m_poPtr == 0U) {
    bool tx = io.canTX();
    if (!tx)
      return;
  }

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
}

uint8_t CAX25TX::writeData(const uint8_t* data, uint16_t length)
{
  CAX25Frame frame(data, length);
  frame.addCRC();

  m_poLen    = 0U;
  m_poPtr    = 0U;
  m_nrzi     = false;
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

  // Add the End Flag
  for (uint16_t i = 0U; i < 8U; i++, m_poLen++) {
    bool b1 = READ_BIT1(END_FLAG, i) != 0U;
    bool b2 = NRZI(b1);
    WRITE_BIT1(m_poBuffer, m_poLen, b2);
  }

  return 0U;
}

uint8_t CAX25TX::writeDataAck(uint16_t token, const uint8_t* data, uint16_t length)
{
  m_tokens.push_back(token);

  return writeData(data, length);
}

void CAX25TX::writeBit(bool b)
{
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

    q31_t res = (value >> 1) * m_level;
    buffer[i] = q15_t(__SSAT((res >> 15), 16));

    if (m_tablePtr >= AUDIO_TABLE_LEN)
      m_tablePtr -= AUDIO_TABLE_LEN;
  }

  io.write(buffer, AX25_RADIO_SYMBOL_LENGTH);
}

void CAX25TX::setTXDelay(uint8_t value)
{
  m_txDelay = value * 24U;
}
  
void CAX25TX::setLevel(uint8_t value)
{
  m_level = q15_t(value * 128);
}

bool CAX25TX::NRZI(bool b)
{
    if (!b)
      m_nrzi = !m_nrzi;

    return m_nrzi;
}

