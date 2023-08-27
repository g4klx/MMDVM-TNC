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
#include "AX25Twist.h"

// 1200Hz = -12dB, 2200Hz = 0dB; 6279Hz cutoff; cosine.
q15_t dB12[] = {
  89,
  -1473,
  -5396,
  -9722,
  32767,
  -9722,
  -5396,
  -1473,
  89
};

// 1200Hz = -11dB, 2200Hz = 0dB; 6210Hz cutoff; cosine.
q15_t dB11[] = {
  67,
  -1516,
  -5381,
  -9603,
  32767,
  -9603,
  -5381,
  -1516,
  67
};

// 1200Hz = -10dB, 2200Hz = 0dB; 6127Hz cutoff; cosine.
q15_t dB10[] = {
  40,
  -1566,
  -5361,
  -9460,
  32767,
  -9460,
  -5361,
  -1566,
  40
};

// 1200Hz = -9dB, 2200Hz = 0dB; 6027Hz cutoff; cosine.
q15_t dB9[] = {
  9,
  -1624,
  -5334,
  -9287,
  32767,
  -9287,
  -5334,
  -1624,
  9
};

// 1200Hz = -8dB, 2200Hz = 0dB; 5903Hz cutoff; cosine.
q15_t dB8[] = {
  -30,
  -1691,
  -5296,
  -9074,
  32767,
  -9074,
  -5296,
  -1691,
  -30
};

// 1200Hz = -7dB, 2200Hz = 0dB; 5747Hz cutoff; cosine.
q15_t dB7[] = {
  -79,
  -1769,
  -5241,
  -8806,
  32767,
  -8806,
  -5241,
  -1769,
  -79
};

// 1200Hz = -6dB, 2200Hz = 0dB; 5550Hz cutoff; cosine.
q15_t dB6[] = {
  -137,
  -1855,
  -5161,
  -8469,
  32767,
  -8469,
  -5161,
  -1855,
  -137
};

// 1200Hz = -5dB, 2200Hz = 0dB; 5290Hz cutoff; cosine.
q15_t dB5[] = {
  -211,
  -1950,
  -5036,
  -8026,
  32767,
  -8026,
  -5036,
  -1950,
  -211
};

// 1200Hz = -4dB, 2200Hz = 0dB; 4945Hz cutoff; cosine.
q15_t dB4[] = {
  -299,
  -2040,
  -4841,
  -7444,
  32767,
  -7444,
  -4841,
  -2040,
  -299
};

// 1200Hz = -3dB, 2200Hz = 0dB; 4455Hz cutoff; cosine.
q15_t dB3[] = {
  -402,
  -2101,
  -4510,
  -6627,
  32767,
  -6627,
  -4510,
  -2101,
  -402
};

// 1200Hz = -2dB, 2200Hz = 0dB; 3730Hz cutoff; cosine.
q15_t dB2[] = {
  -497,
  -2047,
  -3919,
  -5444,
  32767,
  -5444,
  -3919,
  -2047,
  -497
};

// 1200Hz = -1dB, 2200Hz = 0dB; 2550Hz cutoff; cosine.
q15_t dB1[] = {
  -493,
  -1638,
  -2767,
  -3593,
  32767,
  -3593,
  -2767,
  -1638,
  -493
};

q15_t dB0[] = {
  0,
  0,
  0,
  0,
  32767,
  0,
  0,
  0,
  0
};

// 1200Hz = 0dB, 2200Hz = -1dB; 13169Hz cutoff; boxcar.
q15_t dB_1[] = {
  -3320,
  6906,
  3482,
  -22842,
  32767,
  -22842,
  3482,
  6906,
  -3320
};

// 1200Hz = 0dB, 2200Hz = -2dB; 13084Hz cutoff; boxcar.
q15_t dB_2[] = {
  -3082,
  6962,
  3210,
  -22701,
  32767,
  -22701,
  3210,
  6962,
  -3082
};

// 1200Hz = 0dB, 2200Hz = -3dB; 12972Hz cutoff; boxcar.
q15_t dB_3[] = {
  -2765,
  7022,
  2857,
  -22515,
  32767,
  -22515,
  2857,
  7022,
  -2765
};

// 1200Hz = 0dB, 2200Hz = -4dB; 12822Hz cutoff; boxcar.
q15_t dB_4[] = {
  -2336,
  7079,
  2391,
  -22265,
  32767,
  -22265,
  2391,
  7079,
  -2336
};

// 1200Hz = 0dB, 2200Hz = -5dB; 12617Hz cutoff; boxcar.
q15_t dB_5[] = {
  -1745,
  7116,
  1768,
  -21919,
  32767,
  -21919,
  1768,
  7116,
  -1745
};

// 1200Hz = 0dB, 2200Hz = -6dB; 12340Hz cutoff; boxcar.
q15_t dB_6[] = {
  -950,
  7092,
  954,
  -21447,
  32767,
  -21447,
  954,
  7092,
  -950
};

q15_t* coeffs[] = {
  dB12,
  dB11,
  dB10,
  dB9,
  dB8,
  dB7,
  dB6,
  dB5,
  dB4,
  dB3,
  dB2,
  dB1,
  dB0,
  dB_1,
  dB_2,
  dB_3,
  dB_4,
  dB_5,
  dB_6
};

CAX25Twist::CAX25Twist(int8_t n) :
m_filter(),
m_state()
{
  setTwist(n);
}

void CAX25Twist::process(q15_t* in, q15_t* out, uint8_t length)
{
  ::arm_fir_fast_q15(&m_filter, in, out, length);
}

void CAX25Twist::setTwist(int8_t n)
{
  uint8_t twist = uint8_t(n + 6);

  m_filter.numTaps = 9;
  m_filter.pState  = m_state;
  m_filter.pCoeffs = coeffs[twist];
}

