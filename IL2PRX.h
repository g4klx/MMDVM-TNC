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

#if !defined(IL2PRX_H)
#define  IL2PRX_H

#include "IL2PDefines.h"
#include "IL2PRXFrame.h"

enum IL2PRX_STATE {
  IL2PRXS_NONE,
  IL2PRXS_HEADER,
  IL2PRXS_PAYLOAD
};

const uint16_t IL2P_MAX_LENGTH_SAMPLES = (1023U + IL2P_HEADER_LENGTH_BYTES + IL2P_HEADER_PARITY_BYTES + 5U * 16U) * IL2P_SYMBOLS_PER_BYTE;

class CIL2PRX {
public:
  CIL2PRX();

  void reset();

  void samples(q15_t* samples, uint8_t length);

  bool canTX() const;

private:
  IL2PRX_STATE         m_state;
  arm_fir_instance_q15 m_rrc02Filter;
  q15_t                m_rrc02State[70U];         // NoTaps + BlockSize - 1, 42 + 20 - 1 plus some spare
  uint32_t             m_bitBuffer[IL2P_RADIO_SYMBOL_LENGTH];
  q15_t                m_buffer[IL2P_MAX_LENGTH_SAMPLES];
  uint16_t             m_bitPtr;
  uint16_t             m_dataPtr;
  uint16_t             m_startPtr;
  uint16_t             m_endPtr;
  uint16_t             m_syncPtr;
  bool                 m_invert;
  CIL2PRXFrame         m_frame;
  q31_t                m_maxCorr;
  q15_t                m_centre[16U];
  q15_t                m_centreVal;
  q15_t                m_threshold[16U];
  q15_t                m_thresholdVal;
  uint8_t              m_averagePtr;
  uint8_t              m_countdown;
  uint32_t             m_slotCount;
  bool                 m_dcd;
  bool                 m_canTX;
  uint8_t              m_packet[1100U];
  uint8_t              m_x;
  uint8_t              m_a;
  uint8_t              m_b;
  uint8_t              m_c;

  void processNone(q15_t sample);
  void processHeader(q15_t sample);
  void processPayload(q15_t sample);
  bool correlateSync();
  void calculateLevels(uint16_t start, uint16_t count);
  void samplesToBits(uint16_t start, uint16_t count, uint8_t* buffer, uint16_t offset, q15_t centre, q15_t threshold);

  void initRand();
  uint8_t rand();
};

#endif

