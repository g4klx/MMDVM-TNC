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
#include "KISSDefines.h"
#include "Globals.h"
#include "AX25RX.h"

/*
 * Generated with Scipy Filter, 120 coefficients, 1100-2300Hz bandpass,
 * Hann window, starting and ending 0 value coefficients removed.
 *
 * np.array(
 *  firwin2(120,
 *      [
 *          0.0,
 *          1000.0/(sample_rate/2),
 *          1100.0/(sample_rate/2),
 *          2350.0/(sample_rate/2),
 *          2500.0/(sample_rate/2),
 *          1.0
 *      ],
 *      [0,0,1,1,0,0],
 *      antisymmetric = False,
 *      window='hann') * 32768,
 *  dtype=int)[10:-10]
 */

const uint32_t FILTER_LEN = 100U;

q15_t FILTER_COEFFS[] = {
	-5, 0, 8, 18, 30, 41, 51, 58, 61, 59, 53, 43, 32, 20, 12, 10, 16, 33, 62, 101, 149, 200, 249, 289, 311, 307, 271, 199, 88,
	-60, -239, -442, -654, -859, -1042, -1184, -1270, -1287, -1224, -1080, -857, -562, -213,
	172, 570, 954, 1299, 1581, 1781, 1885, 1885, 1781, 1581, 1299, 954, 570, 172,
	-213, -562, -857, -1080, -1224, -1287, -1270, -1184, -1042, -859, -654, -442, -239, -60,
	88, 199, 271, 307, 311, 289, 249, 200, 149, 101, 62, 33, 16, 10, 12, 20, 32, 43, 53, 59, 61, 58, 51, 41, 30, 18, 8, 0, -5};

CAX25RX::CAX25RX() :
m_filter(),
m_state(),
m_demod1(3),
m_demod2(6),
m_demod3(9),
m_lastFCS(0U),
m_count(0U),
m_slotCount(0U),
m_canTX(false),
m_x(1U),
m_a(0xB7U),
m_b(0x73U),
m_c(0xF6U)
{
  m_filter.numTaps = FILTER_LEN;
  m_filter.pState  = m_state;
  m_filter.pCoeffs = FILTER_COEFFS;

  initRand();
}

void CAX25RX::samples(q15_t* samples, uint8_t length)
{
  q15_t output[RX_BLOCK_SIZE];
  ::arm_fir_fast_q15(&m_filter, samples, output, RX_BLOCK_SIZE);

  m_count++;

  CAX25Frame frame;

  bool ret = m_demod1.process(output, length, frame);
  if (ret) {
    if (frame.m_fcs != m_lastFCS || m_count > 2U) {
      m_lastFCS = frame.m_fcs;
      m_count   = 0U;
      serial.writeKISSData(KISS_TYPE_DATA, frame.m_data, frame.m_length - 2U);
    }
    DEBUG1("Decoder 1 reported");
  }

  ret = m_demod2.process(output, length, frame);
  if (ret) {
    if (frame.m_fcs != m_lastFCS || m_count > 2U) {
      m_lastFCS = frame.m_fcs;
      m_count   = 0U;
      serial.writeKISSData(KISS_TYPE_DATA, frame.m_data, frame.m_length - 2U);
    }
    DEBUG1("Decoder 2 reported");
  }

  ret = m_demod3.process(output, length, frame);
  if (ret) {
    if (frame.m_fcs != m_lastFCS || m_count > 2U) {
      m_lastFCS = frame.m_fcs;
      m_count   = 0U;
      serial.writeKISSData(KISS_TYPE_DATA, frame.m_data, frame.m_length - 2U);
    }
    DEBUG1("Decoder 3 reported");
  }

  m_slotCount += RX_BLOCK_SIZE;
  if (m_slotCount >= m_slotTime) {
    m_slotCount = 0U;

    bool dcd1 = m_demod1.isDCD();
    bool dcd2 = m_demod2.isDCD();
    bool dcd3 = m_demod3.isDCD();
    
    if (dcd1 || dcd2 || dcd3) {
      if (!m_dcd) {
        io.setDecode(true);
        io.setADCDetection(true);
      }

      m_canTX = false;
    } else {
      if (m_dcd) {
        io.setDecode(false);
        io.setADCDetection(false);
      }

      m_canTX = m_pPersist >= rand();
    }
  }
}

bool CAX25RX::canTX() const
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


void CAX25RX::initRand() //Can also be used to seed the rng with more entropy during use.
{
  m_a = (m_a ^ m_c ^ m_x);
  m_b = (m_b + m_a);
  m_c = (m_c + (m_b >> 1) ^ m_a);
}

uint8_t CAX25RX::rand()
{
  m_x++;                           //x is incremented every round and is not affected by any other variable

  m_a = (m_a ^ m_c ^ m_x);         //note the mix of addition and XOR
  m_b = (m_b + m_a);               //And the use of very few instructions
  m_c = (m_c + (m_b >> 1) ^ m_a);  //the right shift is to ensure that high-order bits from b can affect  

  return uint8_t(m_c);             //low order bits of other variables
}

