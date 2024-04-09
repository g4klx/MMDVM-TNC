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

#if !defined(MODENRX_H)
#define  MODENRX_H

#include "ModeNDefines.h"
#include "IL2PRX.h"

enum MODENRX_STATE {
  MODENRXS_NONE,
  MODENRXS_HEADER,
  MODENRXS_PAYLOAD,
  MODENRXS_CRC
};

const uint16_t MODEN_MAX_LENGTH_SAMPLES = (1023U + MODEN_HEADER_LENGTH_BYTES + MODEN_HEADER_PARITY_BYTES + 5U * MODEN_PAYLOAD_PARITY_BYTES + MODEN_CRC_LENGTH_BYTES) * MODEN_SYMBOLS_PER_BYTE;

class CModeNRX {
public:
  CModeNRX();

  void reset();

  void samples(q15_t* samples, uint8_t length);

private:
  MODENRX_STATE        m_state;
  arm_fir_instance_q15 m_rrc02Filter;
  q15_t                m_rrc02State[70U];         // NoTaps + BlockSize - 1, 42 + 20 - 1 plus some spare
  uint32_t             m_bitBuffer[MODEN_RADIO_SYMBOL_LENGTH];
  q15_t                m_buffer[MODEN_MAX_LENGTH_SAMPLES];
  uint16_t             m_bitPtr;
  uint16_t             m_dataPtr;
  uint16_t             m_startPtr;
  uint16_t             m_endPtr;
  uint16_t             m_syncPtr;
  bool                 m_invert;
  CIL2PRX              m_frame;
  q31_t                m_maxCorr;
  q15_t                m_centre[16U];
  q15_t                m_centreVal;
  q15_t                m_threshold[16U];
  q15_t                m_thresholdVal;
  uint8_t              m_averagePtr;
  uint8_t              m_countdown;
  uint8_t              m_packet[1100U];

  void processNone(q15_t sample);
  void processHeader(q15_t sample);
  void processPayload(q15_t sample);
  void processCRC(q15_t sample);

  bool correlateSync();
  void calculateLevels(uint16_t startPtr, uint16_t endPtr);
  void samplesToBits(uint16_t startPtr, uint16_t endPtr, uint8_t* buffer);
};

#endif

