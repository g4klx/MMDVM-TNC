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

#include "KISSDefines.h"
#include "Globals.h"
#include "IL2PRX.h"
#include "Utils.h"

// Generated using rcosdesign(0.2, 8, 5, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = {401, 104, -340, -731, -847, -553, 112, 909, 1472, 1450, 683, -675, -2144, -3040, -2706, -770, 2667, 6995,
                                 11237, 14331, 15464, 14331, 11237, 6995, 2667, -770, -2706, -3040, -2144, -675, 683, 1450, 1472, 909, 112,
                                 -553, -847, -731, -340, 104, 401, 0};
const uint16_t RRC_0_2_FILTER_LEN = 42U;

const q15_t SCALING_FACTOR = 18750;      // Q15(0.55)

const uint8_t MAX_SYNC_BIT_ERRS     = 2U;
const uint8_t MAX_SYNC_SYMBOLS_ERRS = 3U;

const uint8_t BIT_MASK_TABLE[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT1(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE[(i)&7])

const uint8_t  NOAVEPTR = 99U;
const uint16_t NOENDPTR = 9999U;

CIL2PRX::CIL2PRX() :
m_state(IL2PRXS_NONE),
m_rrc02Filter(),
m_rrc02State(),
m_bitBuffer(),
m_buffer(),
m_bitPtr(0U),
m_dataPtr(0U),
m_startPtr(NOENDPTR),
m_endPtr(NOENDPTR),
m_syncPtr(NOENDPTR),
m_invert(false),
m_frame(),
m_maxCorr(0),
m_centre(),
m_centreVal(0),
m_threshold(),
m_thresholdVal(0),
m_countdown(0U),
m_slotCount(0U),
m_dcd(false),
m_canTX(false),
m_packet(),
m_x(1U),
m_a(0xB7U),
m_b(0x73U),
m_c(0xF6U)
{
  ::memset(m_rrc02State, 0x00U, 70U * sizeof(q15_t));
  m_rrc02Filter.numTaps = RRC_0_2_FILTER_LEN;
  m_rrc02Filter.pState  = m_rrc02State;
  m_rrc02Filter.pCoeffs = RRC_0_2_FILTER;

  initRand();
}

void CIL2PRX::reset()
{
  m_state        = IL2PRXS_NONE;
  m_dataPtr      = 0U;
  m_bitPtr       = 0U;
  m_maxCorr      = 0;
  m_averagePtr   = NOAVEPTR;
  m_startPtr     = NOENDPTR;
  m_endPtr       = NOENDPTR;
  m_syncPtr      = NOENDPTR;
  m_slotCount    = 0U;
  m_centreVal    = 0;
  m_thresholdVal = 0;
  m_countdown    = 0U;
  m_invert       = false;
  m_dcd          = false;
  m_canTX        = false;
}

void CIL2PRX::samples(q15_t* samples, uint8_t length)
{
  q15_t vals[RX_BLOCK_SIZE];
  ::arm_fir_fast_q15(&m_rrc02Filter, samples, vals, RX_BLOCK_SIZE);

  for (uint8_t i = 0U; i < length; i++) {
    q15_t sample = vals[i];

    m_bitBuffer[m_bitPtr] <<= 1;
    if (sample < 0)
      m_bitBuffer[m_bitPtr] |= 0x01U;

    m_buffer[m_dataPtr] = sample;

    switch (m_state) {
    case IL2PRXS_HEADER:
      processHeader(sample);
      break;
    case IL2PRXS_PAYLOAD:
      processPayload(sample);
      break;
    default:
      processNone(sample);
      break;
    }

    m_dataPtr++;
    if (m_dataPtr >= IL2P_MAX_LENGTH_SAMPLES)
      m_dataPtr = 0U;

    m_bitPtr++;
    if (m_bitPtr >= IL2P_RADIO_SYMBOL_LENGTH)
      m_bitPtr = 0U;
  }

  m_slotCount += RX_BLOCK_SIZE;
  if (m_slotCount >= m_slotTime) {
    m_slotCount = 0U;

    if (m_dcd)
      m_canTX = false;
    else
      m_canTX = m_pPersist >= rand();
  }
}

void CIL2PRX::processNone(q15_t sample)
{
  bool ret = correlateSync();
  if (ret) {
    // On the first sync, start the countdown to the state change
    if (m_countdown == 0U) {
      io.setDecode(true);
      io.setADCDetection(true);
      m_dcd = true;

      m_averagePtr = NOAVEPTR;

      m_countdown = 5U;
    }
  }

  if (m_countdown > 0U)
    m_countdown--;

  if (m_countdown == 1U) {
    DEBUG4("IL2PRX: sync found pos/centre/threshold", m_syncPtr, m_centreVal, m_thresholdVal);

    m_state     = IL2PRXS_HEADER;
    m_countdown = 0U;
  }
}

void CIL2PRX::processHeader(q15_t sample)
{
  if (m_dataPtr == m_endPtr) {
    calculateLevels(m_startPtr, IL2P_HEADER_LENGTH_SYMBOLS + IL2P_HEADER_PARITY_SYMBOLS + 1U);

    uint8_t frame[IL2P_HEADER_LENGTH_BYTES + IL2P_HEADER_PARITY_BYTES];
    samplesToBits(m_startPtr, IL2P_HEADER_LENGTH_SYMBOLS + IL2P_HEADER_PARITY_SYMBOLS + 1U, frame, 0U, m_centreVal, m_thresholdVal);

    bool ok = m_frame.processHeader(frame, m_packet);
    if (ok) {
      uint16_t length = m_frame.getPayloadLength();
      if (length > 0U) {
        DEBUG2("IL2PRX: header is valid and has a payload", length);

        m_state = IL2PRXS_PAYLOAD;

        length += m_frame.getPayloadParityLength();

        // The payload starts right after the header
        m_startPtr = m_endPtr;
        if (m_startPtr >= IL2P_MAX_LENGTH_SAMPLES)
          m_startPtr -= IL2P_MAX_LENGTH_SAMPLES;

        m_endPtr = m_startPtr + (length * IL2P_SYMBOLS_PER_BYTE * IL2P_RADIO_SYMBOL_LENGTH);
        if (m_endPtr >= IL2P_MAX_LENGTH_SAMPLES)
          m_endPtr -= IL2P_MAX_LENGTH_SAMPLES;
      } else {
        DEBUG1("IL2PRX: header is valid but no payload");

        length = m_frame.getHeaderLength();
        serial.writeKISSData(KISS_TYPE_DATA, m_packet, length);

        io.setDecode(false);
        io.setADCDetection(false);
        m_dcd = false;

        m_state      = IL2PRXS_NONE;
        m_endPtr     = NOENDPTR;
        m_averagePtr = NOAVEPTR;
        m_countdown  = 0U;
        m_maxCorr    = 0;
      }
    } else {
      DEBUG1("IL2PRX: header is invalid");

      io.setDecode(false);
      io.setADCDetection(false);
      m_dcd = false;

      m_state      = IL2PRXS_NONE;
      m_endPtr     = NOENDPTR;
      m_averagePtr = NOAVEPTR;
      m_countdown  = 0U;
      m_maxCorr    = 0;
    }
  }
}

void CIL2PRX::processPayload(q15_t sample)
{
  if (m_dataPtr == m_endPtr) {
    uint16_t payloadLength = m_frame.getPayloadLength();
    uint16_t overallLength = payloadLength + m_frame.getPayloadParityLength();

    calculateLevels(m_startPtr, overallLength * IL2P_SYMBOLS_PER_BYTE + 1U);

    uint8_t frame[1023U + (5U * 16U)];
    samplesToBits(m_startPtr, overallLength * IL2P_SYMBOLS_PER_BYTE + 1U, frame, 0U, m_centreVal, m_thresholdVal);

    bool ok = m_frame.processPayload(frame, m_packet);
    if (ok) {
      DEBUG1("IL2PRX: payload is valid");

      uint16_t length = m_frame.getHeaderLength() + payloadLength;
      serial.writeKISSData(KISS_TYPE_DATA, m_packet, length);

      io.setDecode(false);
      io.setADCDetection(false);
      m_dcd = false;

      m_state      = IL2PRXS_NONE;
      m_endPtr     = NOENDPTR;
      m_averagePtr = NOAVEPTR;
      m_countdown  = 0U;
      m_maxCorr    = 0;
    } else {
      DEBUG1("IL2PRX: payload is invalid");

      io.setDecode(false);
      io.setADCDetection(false);
      m_dcd = false;

      m_state      = IL2PRXS_NONE;
      m_endPtr     = NOENDPTR;
      m_averagePtr = NOAVEPTR;
      m_countdown  = 0U;
      m_maxCorr    = 0;
    }
  }
}

bool CIL2PRX::correlateSync()
{
  if ((countBits32((m_bitBuffer[m_bitPtr] & IL2P_SYNC_SYMBOLS_MASK) ^  IL2P_SYNC_SYMBOLS) <= MAX_SYNC_SYMBOLS_ERRS) ||
      (countBits32((m_bitBuffer[m_bitPtr] & IL2P_SYNC_SYMBOLS_MASK) ^ ~IL2P_SYNC_SYMBOLS) <= MAX_SYNC_SYMBOLS_ERRS)) {

    uint16_t ptr = m_dataPtr + IL2P_MAX_LENGTH_SAMPLES - IL2P_SYNC_LENGTH_SAMPLES + IL2P_RADIO_SYMBOL_LENGTH;
    if (ptr >= IL2P_MAX_LENGTH_SAMPLES)
      ptr -= IL2P_MAX_LENGTH_SAMPLES;

    q31_t corr1 = 0;
    q31_t corr2 = 0;
    q15_t min   =  16000;
    q15_t max   = -16000;

    for (uint8_t i = 0U; i < IL2P_SYNC_LENGTH_SYMBOLS; i++) {
      q15_t val = m_buffer[ptr];

      if (val > max)
        max = val;
      if (val < min)
        min = val;

      switch (IL2P_SYNC_SYMBOLS_VALUES[i]) {
      case +3:
        corr1 -= (val + val + val);
        corr2 += (val + val + val);
        break;
      case +1:
        corr1 -= val;
        corr2 += val;
        break;
      case -1:
        corr1 += val;
        corr2 -= val;
        break;
      default:  // -3
        corr1 += (val + val + val);
        corr2 -= (val + val + val);
        break;
      }

      ptr += IL2P_RADIO_SYMBOL_LENGTH;
      if (ptr >= IL2P_MAX_LENGTH_SAMPLES)
        ptr -= IL2P_MAX_LENGTH_SAMPLES;
    }

    if ((corr1 > m_maxCorr) || (corr2 > m_maxCorr)) {
      if (m_averagePtr == NOAVEPTR) {
        m_centreVal = (max + min) >> 1;

        q31_t v1 = (max - m_centreVal) * SCALING_FACTOR;
        m_thresholdVal = q15_t(v1 >> 15);
      }

      m_invert = (corr2 > m_maxCorr);

      uint16_t startPtr = m_dataPtr + IL2P_MAX_LENGTH_SAMPLES - IL2P_SYNC_LENGTH_SAMPLES + IL2P_RADIO_SYMBOL_LENGTH;
      if (startPtr >= IL2P_MAX_LENGTH_SAMPLES)
        startPtr -= IL2P_MAX_LENGTH_SAMPLES;

      uint8_t sync[IL2P_SYNC_LENGTH_BYTES];
      samplesToBits(startPtr, IL2P_SYNC_LENGTH_SYMBOLS, sync, 0U, m_centreVal, m_thresholdVal);

      uint8_t errs = 0U;
      for (uint8_t i = 0U; i < IL2P_SYNC_LENGTH_BYTES; i++)
        errs += countBits8(sync[i] ^ IL2P_SYNC_BYTES[i]);

      if (errs <= MAX_SYNC_BIT_ERRS) {
        m_maxCorr = (corr1 > corr2) ? corr1 : corr2;
        m_syncPtr = m_dataPtr;

        // The header starts right after the sync vector
        m_startPtr = m_dataPtr + IL2P_RADIO_SYMBOL_LENGTH;
        if (m_startPtr >= IL2P_MAX_LENGTH_SAMPLES)
          m_startPtr -= IL2P_MAX_LENGTH_SAMPLES;

        m_endPtr = m_startPtr + IL2P_HEADER_LENGTH_SAMPLES + IL2P_HEADER_PARITY_SAMPLES;
        if (m_endPtr >= IL2P_MAX_LENGTH_SAMPLES)
          m_endPtr -= IL2P_MAX_LENGTH_SAMPLES;

        return true;
      }
    }
  }

  return false;
}

void CIL2PRX::calculateLevels(uint16_t start, uint16_t count)
{
  q15_t maxPos = -16000;
  q15_t minPos =  16000;
  q15_t maxNeg =  16000;
  q15_t minNeg = -16000;

  for (uint16_t i = 0U; i < count; i++) {
    q15_t sample = m_buffer[start];

    if (sample > 0) {
      if (sample > maxPos)
        maxPos = sample;
      if (sample < minPos)
        minPos = sample;
    } else {
      if (sample < maxNeg)
        maxNeg = sample;
      if (sample > minNeg)
        minNeg = sample;
    }

    start += IL2P_RADIO_SYMBOL_LENGTH;
    if (start >= IL2P_MAX_LENGTH_SAMPLES)
      start -= IL2P_MAX_LENGTH_SAMPLES;
  }

  q15_t posThresh = (maxPos + minPos) >> 1;
  q15_t negThresh = (maxNeg + minNeg) >> 1;

  q15_t centre = (posThresh + negThresh) >> 1;

  q15_t threshold = posThresh - centre;

  DEBUG5("IL2PRX: pos/neg/centre/threshold", posThresh, negThresh, centre, threshold);

  if (m_averagePtr == NOAVEPTR) {
    for (uint8_t i = 0U; i < 16U; i++) {
      m_centre[i] = centre;
      m_threshold[i] = threshold;
    }

    m_averagePtr = 0U;
  } else {
    m_centre[m_averagePtr] = centre;
    m_threshold[m_averagePtr] = threshold;

    m_averagePtr++;
    if (m_averagePtr >= 16U)
      m_averagePtr = 0U;
  }

  m_centreVal = 0;
  m_thresholdVal = 0;

  for (uint8_t i = 0U; i < 16U; i++) {
    m_centreVal += m_centre[i];
    m_thresholdVal += m_threshold[i];
  }

  m_centreVal >>= 4;
  m_thresholdVal >>= 4;
}

void CIL2PRX::samplesToBits(uint16_t start, uint16_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold)
{
  for (uint16_t i = 0U; i < count; i++) {
    q15_t sample = m_buffer[start] - centre;

    if (sample < -threshold) {
      WRITE_BIT1(buffer, offset, m_invert);
      offset++;
      WRITE_BIT1(buffer, offset, !m_invert);
      offset++;
    } else if (sample < 0) {
      WRITE_BIT1(buffer, offset, m_invert);
      offset++;
      WRITE_BIT1(buffer, offset, m_invert);
      offset++;
    } else if (sample < threshold) {
      WRITE_BIT1(buffer, offset, !m_invert);
      offset++;
      WRITE_BIT1(buffer, offset, m_invert);
      offset++;
    } else {
      WRITE_BIT1(buffer, offset, !m_invert);
      offset++;
      WRITE_BIT1(buffer, offset, !m_invert);
      offset++;
    }

    start += IL2P_RADIO_SYMBOL_LENGTH;
    if (start >= IL2P_MAX_LENGTH_SAMPLES)
      start -= IL2P_MAX_LENGTH_SAMPLES;
  }
}

bool CIL2PRX::canTX() const
{
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


void CIL2PRX::initRand() //Can also be used to seed the rng with more entropy during use.
{
  m_a = (m_a ^ m_c ^ m_x);
  m_b = (m_b + m_a);
  m_c = (m_c + ((m_b >> 1) ^ m_a));
}

uint8_t CIL2PRX::rand()
{
  m_x++;                           //x is incremented every round and is not affected by any other variable

  m_a = (m_a ^ m_c ^ m_x);         //note the mix of addition and XOR
  m_b = (m_b + m_a);               //And the use of very few instructions
  m_c = (m_c + ((m_b >> 1) ^ m_a));  //the right shift is to ensure that high-order bits from b can affect  

  return uint8_t(m_c);             //low order bits of other variables
}

