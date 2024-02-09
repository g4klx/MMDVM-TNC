/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2023,2024 by Jonathan Naylor G4KLX
 *   Copyright (C) 2015 by Jim Mclaughlin KI6ZUM
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
#include "IO.h"

const q15_t DC_OFFSET = 2048;

const uint32_t OVERFLOW_COUNT = 12000U;

CIO::CIO() :
m_rxBuffer(RX_RINGBUFFER_SIZE),
m_txBuffer(TX_RINGBUFFER_SIZE),
m_rxLevel(RX_LEVEL * 128),
m_pPersist(P_PERSISTENCE),
m_slotTime((SLOT_TIME / 10U) * 240U),
m_dcd(false),
m_ledCount(0U),
m_ledValue(true),
m_rxCount(0U),
m_rxValue(false),
m_txCount(0U),
m_txValue(false),
m_slotCount(0U),
m_canTX(false),
m_x(1U),
m_a(0xB7U),
m_b(0x73U),
m_c(0xF6U)
{
}

void CIO::selfTest()
{
  bool ledValue = false;

  for (uint8_t i = 0U; i < 6U; i++) {
    ledValue = !ledValue;

    // We exclude PTT to avoid trigger the transmitter
    setLEDInt(ledValue);
    setCOSInt(ledValue);
#if defined(MODE_LEDS)
    setMode1Int(ledValue);
    setMode2Int(ledValue);
    setMode3Int(ledValue);
    setMode4Int(ledValue);
#endif
    delayInt(250);
  }

#if defined(MODE_LEDS)
  setMode1Int(false);
  setMode2Int(false);
  setMode3Int(false);
  setMode4Int(false);

  setMode1Int(true);

  delayInt(250);
  setMode2Int(true);

  delayInt(250);
  setMode3Int(true);

  delayInt(250);
  setMode4Int(true);

  delayInt(250);
  setMode4Int(false);

  delayInt(250);
  setMode3Int(false);

  delayInt(250);
  setMode2Int(false);

  delayInt(250);
  setMode1Int(false);
#endif
}

void CIO::start()
{
  initRand();

  initInt();

  selfTest();

  startInt();
}

void CIO::process()
{
#if defined(CONSTANT_SRV_LED)
  setLEDInt(true);
#else
  if (m_ledCount >= 24000U) {
    m_ledCount = 0U;
    m_ledValue = !m_ledValue;
    setLEDInt(m_ledValue);
  }
#endif

  // Switch off the transmitter if needed
  if (m_txBuffer.getData() == 0U && m_tx) {
    m_tx = false;
    setPTTInt(false);
    DEBUG1("TX OFF");
  }

  if (m_rxCount == 0U) {
    bool value = m_rxBuffer.hasOverflowed();
    if (value)
      m_rxCount = OVERFLOW_COUNT;

    if (value != m_rxValue) {
      showRXOverflow(value);
      m_rxValue = value;
    }
  } else {
    m_rxBuffer.hasOverflowed();
  }

  if (m_txCount == 0U) {
    bool value = m_txBuffer.hasOverflowed();
    if (value)
      m_txCount = OVERFLOW_COUNT;

    if (value != m_txValue) {
      showTXOverflow(value);
      m_txValue = value;
    }
  } else {
    m_txBuffer.hasOverflowed();
  }

  if (m_rxBuffer.getData() >= RX_BLOCK_SIZE) {
    // Only do the CSMA calculations when in simplex mode
    if (!m_duplex) {
      if (m_dcd) {
        m_slotCount = 0U;
      } else {
        m_slotCount += RX_BLOCK_SIZE;
        if (m_slotCount >= m_slotTime) {
          m_slotCount = 0U;
          m_canTX = (m_pPersist >= rand());
        }
      }
    }

    q15_t samples[RX_BLOCK_SIZE];

    for (uint16_t i = 0U; i < RX_BLOCK_SIZE; i++) {
      uint16_t sample;
      m_rxBuffer.get(sample);

      q15_t res1 = q15_t(sample) - DC_OFFSET;
      q31_t res2 = res1 * m_rxLevel;
      samples[i] = q15_t(__SSAT((res2 >> 15), 16));
    }

    // Only pass the receive signal when the receiver is meant to be running
    if (!m_tx || (m_tx && m_duplex)) {
      switch (m_mode) {
        case 1U:
          ax25RX.samples(samples, RX_BLOCK_SIZE);
          break;

        case 2U:
          mode2RX.samples(samples, RX_BLOCK_SIZE);
          break;

        default:
          break;
      }
    }
  }
}

void CIO::write(q15_t* samples, uint16_t length)
{
  // Switch the transmitter on if needed
  if (!m_tx) {
    m_tx = true;
    setPTTInt(true);
    DEBUG1("TX ON");
  }

  for (uint16_t i = 0U; i < length; i++) {
    uint16_t res = uint16_t(samples[i] + DC_OFFSET);
    m_txBuffer.put(res);
  }
}

void CIO::showMode()
{
#if defined(MODE_LEDS)
  switch (m_mode) {
    case 1U:
      setMode1Int(true);
      setMode2Int(false);
      // setMode3Int(false);
      // setMode4Int(false);
      break;
    case 2U:
      setMode1Int(false);
      setMode2Int(true);
      // setMode3Int(false);
      // setMode4Int(false);
      break;
    default:
      setMode1Int(false);
      setMode2Int(false);
      // setMode3Int(false);
      // setMode4Int(false);
      break;
  }
#endif
}

void CIO::showRXOverflow(bool on)
{
#if defined(MODE_LEDS)
  setMode3Int(on);
#endif
}

void CIO::showTXOverflow(bool on)
{
#if defined(MODE_LEDS)
  setMode4Int(on);
#endif
}

uint16_t CIO::getSpace() const
{
  return m_txBuffer.getSpace();
}

void CIO::setDecode(bool dcd)
{
  if (dcd != m_dcd)
    setCOSInt(dcd);

  m_dcd = dcd;
}

void CIO::setRXLevel(uint8_t value)
{
  m_rxLevel = q15_t(value * 128);
}

void CIO::setPPersist(uint8_t value)
{
  m_pPersist = value;
}

void CIO::setSlotTime(uint8_t value)
{
  m_slotTime = value * 240U;
}

bool CIO::canTX() const
{
  if (m_duplex)
    return true;

  if (m_dcd)
    return false;

  return m_canTX;
}

// Taken from https://www.electro-tech-online.com/threads/ultra-fast-pseudorandom-number-generator-for-8-bit.124249/
//X ABC Algorithm Random Number Generator for 8-Bit Devices:
//This is a small PRNG, experimentally verified to have at least a 50 million byte period
//by generating 50 million bytes and observing that there were no overapping sequences and repeats.
//This generator passes serial correlation, entropy , Monte Carlo Pi value, arithmetic mean,
//And many other statistical tests. This generator may have a period of up to 2^32, but this has
//not been verified.
//
// By XORing 3 bytes into the a,b, and c registers, you can add in entropy from 
//an external source easily.
//
//This generator is free to use, but is not suitable for cryptography due to its short period(by //cryptographic standards) and simple construction. No attempt was made to make this generator 
// suitable for cryptographic use.
//
//Due to the use of a constant counter, the generator should be resistant to latching up.
//A significant performance gain is had in that the x variable is only ever incremented.
//
//Only 4 bytes of ram are needed for the internal state, and generating a byte requires 3 XORs , //2 ADDs, one bit shift right , and one increment. Difficult or slow operations like multiply, etc 
//were avoided for maximum speed on ultra low power devices.


void CIO::initRand() //Can also be used to seed the rng with more entropy during use.
{
  m_a = (m_a ^ m_c ^ m_x);
  m_b = (m_b + m_a);
  m_c = (m_c + (m_b >> 1) ^ m_a);
}

uint8_t CIO::rand()
{
  m_x++;                           //x is incremented every round and is not affected by any other variable

  m_a = (m_a ^ m_c ^ m_x);         //note the mix of addition and XOR
  m_b = (m_b + m_a);               //And the use of very few instructions
  m_c = (m_c + (m_b >> 1) ^ m_a);  //the right shift is to ensure that high-order bits from b can affect  

  return uint8_t(m_c);             //low order bits of other variables
}

