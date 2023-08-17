/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2023 by Jonathan Naylor G4KLX
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

// Generated using rcosdesign(0.2, 8, 5, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = {401, 104, -340, -731, -847, -553, 112, 909, 1472, 1450, 683, -675, -2144, -3040, -2706, -770, 2667, 6995,
                                 11237, 14331, 15464, 14331, 11237, 6995, 2667, -770, -2706, -3040, -2144, -675, 683, 1450, 1472, 909, 112,
                                 -553, -847, -731, -340, 104, 401, 0};
const uint16_t RRC_0_2_FILTER_LEN = 42U;

const uint16_t DC_OFFSET = 2048U;

CIO::CIO() :
m_started(false),
m_rxBuffer(RX_RINGBUFFER_SIZE),
m_txBuffer(TX_RINGBUFFER_SIZE),
m_rrc02Filter1(),
m_rrc02State1(),
m_rxLevel(128 * 128),
m_mode1TXLevel(128 * 128),
m_mode2TXLevel(128 * 128),
m_ledCount(0U),
m_ledValue(true),
m_detect(false),
m_adcOverflow(0U),
m_dacOverflow(0U)
{
  ::memset(m_rrc02State1, 0x00U, 70U * sizeof(q15_t));
  m_rrc02Filter1.numTaps = RRC_0_2_FILTER_LEN;
  m_rrc02Filter1.pState  = m_rrc02State1;
  m_rrc02Filter1.pCoeffs = RRC_0_2_FILTER;

  initInt();
  
  selfTest();
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
  if (m_started)
    return;

  startInt();

  m_started = true;
}

void CIO::process()
{
  m_ledCount++;
  if (m_started) {
#if defined(CONSTANT_SRV_LED)
    setLEDInt(true);
#else
    if (m_ledCount >= 24000U) {
      m_ledCount = 0U;
      m_ledValue = !m_ledValue;
      setLEDInt(m_ledValue);
    }
#endif
  } else {
    if (m_ledCount >= 240000U) {
      m_ledCount = 0U;
      m_ledValue = !m_ledValue;
      setLEDInt(m_ledValue);
    }
    return;
  }

  // Switch off the transmitter if needed
  if (m_txBuffer.getData() == 0U && m_tx) {
    m_tx = false;
    setPTTInt(false);
    DEBUG1("TX OFF");
  }

  if (m_rxBuffer.getData() >= RX_BLOCK_SIZE) {
    q15_t    samples[RX_BLOCK_SIZE];
    uint8_t  control[RX_BLOCK_SIZE];

    for (uint16_t i = 0U; i < RX_BLOCK_SIZE; i++) {
      uint16_t sample;
      m_rxBuffer.get(sample);

      // Detect ADC overflow
      if (m_detect && (sample == 0U || sample == 4095U))
        m_adcOverflow++;

      q15_t res1 = q15_t(sample) - DC_OFFSET;
      q31_t res2 = res1 * m_rxLevel;
      samples[i] = q15_t(__SSAT((res2 >> 15), 16));
    }

    switch (m_mode) {
      case 1U:
        ax25RX.samples(samples, RX_BLOCK_SIZE);
        break;

      case 2U: {
          q15_t vals[RX_BLOCK_SIZE];
          ::arm_fir_fast_q15(&m_rrc02Filter1, samples, vals, RX_BLOCK_SIZE);
          il2pRX.samples(vals, RX_BLOCK_SIZE);
        }
        break;
    }
  }
}

void CIO::write(q15_t* samples, uint16_t length)
{
  if (!m_started)
    return;

  // Switch the transmitter on if needed
  if (!m_tx) {
    m_tx = true;
    setPTTInt(true);
    DEBUG1("TX ON");
  }

  q15_t txLevel = 0;
  switch (m_mode) {
    case 1U:
      txLevel = m_mode1TXLevel;
      break;
    case 2U:
      txLevel = m_mode2TXLevel;
      break;
  }

  for (uint16_t i = 0U; i < length; i++) {
    q31_t res1 = samples[i] * txLevel;
    q15_t res2 = q15_t(__SSAT((res1 >> 15), 16));
    uint16_t res3 = uint16_t(res2 + DC_OFFSET);

    // Detect DAC overflow
    if (res3 > 4095U)
      m_dacOverflow++;

    m_txBuffer.put(res3);
  }
}

void CIO::showMode()
{
#if defined(MODE_LEDS)
  switch (m_mode) {
    case 1U:
      setMode1Int(true);
      setMode2Int(false);
      setMode3Int(false);
      setMode4Int(false);
      break;
    case 2U:
      setMode1Int(false);
      setMode2Int(true);
      setMode3Int(false);
      setMode4Int(false);
      break;
    default:
      setMode1Int(false);
      setMode2Int(false);
      setMode3Int(false);
      setMode4Int(false);
      break;
  }
#endif
}

uint16_t CIO::getSpace() const
{
  return m_txBuffer.getSpace();
}

void CIO::setDecode(bool dcd)
{
  if (dcd != m_dcd)
    setCOSInt(dcd ? true : false);

  m_dcd = dcd;
}

void CIO::setADCDetection(bool detect)
{
  m_detect = detect;
}

void CIO::setParameters(uint8_t rxLevel, uint8_t mode1TXLevel, uint8_t mode2TXLevel)
{
  m_rxLevel      = q15_t(rxLevel * 128);
  m_mode1TXLevel = q15_t(mode1TXLevel * 128);
  m_mode2TXLevel = q15_t(mode2TXLevel * 128);
}

void CIO::getOverflow(bool& adcOverflow, bool& dacOverflow)
{
  adcOverflow = m_adcOverflow > 0U;
  dacOverflow = m_dacOverflow > 0U;

  m_adcOverflow = 0U;
  m_dacOverflow = 0U;
}

bool CIO::hasTXOverflow()
{
  return m_txBuffer.hasOverflowed();
}

bool CIO::hasRXOverflow()
{
  return m_rxBuffer.hasOverflowed();
}

