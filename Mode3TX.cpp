/*
 *   Copyright (C) 2023,2024 by Jonathan Naylor G4KLX
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

#include "Mode3Defines.h"
#include "Globals.h"
#include "Mode3TX.h"

// Generated using rcosdesign(0.2, 8, 5, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = {0, 0, 0, 0, 850, 219, -720, -1548, -1795, -1172, 237, 1927, 3120, 3073, 1447, -1431, -4544, -6442,
                                 -5735, -1633, 5651, 14822, 23810, 30367, 32767, 30367, 23810, 14822, 5651, -1633, -5735, -6442,
                                 -4544, -1431, 1447, 3073, 3120, 1927, 237, -1172, -1795, -1548, -720, 219, 850}; // numTaps = 45, L = 5
const uint16_t RRC_0_2_FILTER_PHASE_LEN = 9U; // phaseLength = numTaps/L

const q15_t LEVELA =  1362;
const q15_t LEVELB =  454;
const q15_t LEVELC = -454;
const q15_t LEVELD = -1362;

const uint8_t BIT_MASK_TABLE1[] = { 0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U };

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE1[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE1[(i)&7])
#define READ_BIT1(p,i)    (p[(i)>>3] & BIT_MASK_TABLE1[(i)&7])

const uint8_t BIT_MASK_TABLE2[] = { 0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U };

#define WRITE_BIT2(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE2[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE2[(i)&7])
#define READ_BIT2(p,i)    (p[(i)>>3] & BIT_MASK_TABLE2[(i)&7])

CMode3TX::CMode3TX() :
m_fifo(3000U),
m_playOut(0U),
m_modFilter(),
m_modState(),
m_frame(),
m_level(MODE3_TX_LEVEL * 128),
m_txDelay((TX_DELAY / 10U) * 24U),
m_txTail((TX_TAIL / 10U) * 24U),
m_tokens()
{
  ::memset(m_modState, 0x00U, 16U * sizeof(q15_t));

  m_modFilter.L           = MODE3_RADIO_SYMBOL_LENGTH;
  m_modFilter.phaseLength = RRC_0_2_FILTER_PHASE_LEN;
  m_modFilter.pCoeffs     = RRC_0_2_FILTER;
  m_modFilter.pState      = m_modState;
}

void CMode3TX::process()
{
  if (!m_duplex) {
    // Nothing left to transmit, send the packet tokens back
    if (!m_tx && m_fifo.getData() == 0U) {
      m_tokens.reset();
      uint16_t token;
      while (m_tokens.next(token))
        serial.writeKISSAck(token);
      m_tokens.clear();
    }
  } else {
    // Send the tokens back immediately as the packets can be transmitted immediately too
    m_tokens.reset();
    uint16_t token;
    while (m_tokens.next(token))
      serial.writeKISSAck(token);
    m_tokens.clear();
  }

  // Transmit is off but we have data to send
  if (!m_tx && m_fifo.getData() > 0U) {
    bool tx = io.canTX();
    if (!tx)
      return;
  }

  // Are we sending the trailer?
  if (m_playOut > 0U) {
    uint16_t space = io.getSpace();
    while (space > (MODE3_SYMBOLS_PER_BYTE * MODE3_RADIO_SYMBOL_LENGTH)) {
      writeSilence();

      space -= MODE3_SYMBOLS_PER_BYTE * MODE3_RADIO_SYMBOL_LENGTH;
      m_playOut--;

      if (m_playOut == 0U)
        break;
    }

    return;
  }

  if (m_fifo.getData() > 0U) {
    uint16_t space = io.getSpace();
    while (space > (MODE3_SYMBOLS_PER_BYTE * MODE3_RADIO_SYMBOL_LENGTH)) {
      uint8_t c = 0U;
      m_fifo.get(c);

      writeByte(c);

      space -= MODE3_SYMBOLS_PER_BYTE * MODE3_RADIO_SYMBOL_LENGTH;

      if (m_fifo.getData() == 0U) {
        m_playOut = m_txTail;
        return;
      }
    }
  }
}

uint8_t CMode3TX::writeData(const uint8_t* data, uint16_t length)
{
  uint16_t space = m_fifo.getSpace();
  if (space < (length + MODE3_SYNC_LENGTH_BYTES)) {
    DEBUG1("Mode3TX: no space for the packet");
    return 5U;
  }

  // Add the preamble symbols
  if (!m_tx && (m_fifo.getData() == 0U)) {
    for (uint16_t i = 0U; i < m_txDelay; i++)
      m_fifo.put(MODE3_PREAMBLE_BYTE);
  }

  // Add the IL2P sync vector
  for (uint8_t i = 0U; i < MODE3_SYNC_LENGTH_BYTES; i++)
    m_fifo.put(MODE3_SYNC_BYTES[i]);

  uint8_t buffer[2000U];
  uint16_t len = m_frame.process(data, length, buffer);

  for (uint16_t i = 0U; i < len; i++)
    m_fifo.put(buffer[i]);

  // Insert some spacer
  for (uint8_t i = 0U; i < 10U; i++)
    m_fifo.put(MODE3_PREAMBLE_BYTE);

  return 0U;
}

uint8_t CMode3TX::writeDataAck(uint16_t token, const uint8_t* data, uint16_t length)
{
  m_tokens.add(token);

  return writeData(data, length);
}

void CMode3TX::writeByte(uint8_t c)
{
  q15_t inBuffer[MODE3_SYMBOLS_PER_BYTE];

  const uint8_t MASK = 0xC0U;

  for (uint8_t i = 0U; i < 4U; i++, c <<= 2) {
    q15_t value = 0;

    switch (c & MASK) {
      case 0xC0U:
        value = LEVELA;
        break;
      case 0x80U:
        value = LEVELB;
        break;
      case 0x00U:
        value = LEVELC;
        break;
      default:
        value = LEVELD;
        break;
    }

    q31_t res = value * m_level;

    inBuffer[i] = q15_t(__SSAT((res >> 15), 16));
  }

  q15_t outBuffer[MODE3_RADIO_SYMBOL_LENGTH * 4U];
  ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, MODE3_SYMBOLS_PER_BYTE);

  io.write(outBuffer, MODE3_RADIO_SYMBOL_LENGTH * MODE3_SYMBOLS_PER_BYTE);
}

void CMode3TX::writeSilence()
{
  q15_t inBuffer[MODE3_SYMBOLS_PER_BYTE] = {0, 0, 0, 0};
  q15_t outBuffer[MODE3_RADIO_SYMBOL_LENGTH * 4U];

  ::arm_fir_interpolate_q15(&m_modFilter, inBuffer, outBuffer, MODE3_SYMBOLS_PER_BYTE);

  io.write(outBuffer, MODE3_RADIO_SYMBOL_LENGTH * MODE3_SYMBOLS_PER_BYTE);
}

void CMode3TX::setTXDelay(uint8_t value)
{
  m_txDelay = value * 24U;
}
  
void CMode3TX::setTXTail(uint8_t value)
{
  m_txTail = value * 24U;
}
  
void CMode3TX::setLevel(uint8_t value)
{
  m_level = q15_t(value * 128);
}

