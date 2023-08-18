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
#include "IL2PRX.h"

// Generated using rcosdesign(0.2, 8, 5, 'sqrt') in MATLAB
static q15_t RRC_0_2_FILTER[] = {401, 104, -340, -731, -847, -553, 112, 909, 1472, 1450, 683, -675, -2144, -3040, -2706, -770, 2667, 6995,
                                 11237, 14331, 15464, 14331, 11237, 6995, 2667, -770, -2706, -3040, -2144, -675, 683, 1450, 1472, 909, 112,
                                 -553, -847, -731, -340, 104, 401, 0};
const uint16_t RRC_0_2_FILTER_LEN = 42U;

CIL2PRX::CIL2PRX() :
m_rrc02Filter(),
m_rrc02State(),
m_frame(),
m_slotCount(0U),
m_dcd(false),
m_canTX(false),
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

void CIL2PRX::samples(q15_t* samples, uint8_t length)
{
  q15_t vals[RX_BLOCK_SIZE];
  ::arm_fir_fast_q15(&m_rrc02Filter, samples, vals, RX_BLOCK_SIZE);

/*
  q15_t output[RX_BLOCK_SIZE];
  ::arm_fir_fast_q15(&m_filter, samples, output, RX_BLOCK_SIZE);

  bool ret = m_demod1.process(output, length, frame);
  if (ret) {
    if (frame.m_fcs != m_lastFCS || m_count > 2U) {
      m_lastFCS = frame.m_fcs;
      m_count   = 0U;
      serial.writeKISSData(KISS_TYPE_DATA, frame.m_data, frame.m_length - 2U);
    }
    DEBUG1("Decoder 1 reported");
  }

  m_slotCount += RX_BLOCK_SIZE;
  if (m_slotCount >= m_slotTime) {
    m_slotCount = 0U;

    bool dcd = m_demod1.isDCD();
    if (dcd) {
      if (!m_dcd) {
        io.setDecode(true);
        io.setADCDetection(true);
        m_dcd = true;
      }

      m_canTX = false;
    } else {
      if (m_dcd) {
        io.setDecode(false);
        io.setADCDetection(false);
        m_dcd = false;
      }

      m_canTX = m_pPersist >= rand();
    }
  }
*/

  
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
  m_c = (m_c + (m_b >> 1) ^ m_a);
}

uint8_t CIL2PRX::rand()
{
  m_x++;                           //x is incremented every round and is not affected by any other variable

  m_a = (m_a ^ m_c ^ m_x);         //note the mix of addition and XOR
  m_b = (m_b + m_a);               //And the use of very few instructions
  m_c = (m_c + (m_b >> 1) ^ m_a);  //the right shift is to ensure that high-order bits from b can affect  

  return uint8_t(m_c);             //low order bits of other variables
}

