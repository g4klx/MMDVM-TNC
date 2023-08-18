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

#if !defined(IL2PTX_H)
#define  IL2PTX_H

#include <vector>

#include "IL2PTXFrame.h"
#include "RingBuffer.h"

class CIL2PTX {
public:
  CIL2PTX();

  uint8_t writeData(const uint8_t* data, uint16_t length);
  uint8_t writeDataAck(uint16_t token, const uint8_t* data, uint16_t length);

  void process();

  void setTXDelay(uint8_t value);
  void setLevel(uint8_t value);

  uint8_t getSpace() const;

private:
  CRingBuffer<uint8_t>             m_fifo;
  arm_fir_interpolate_instance_q15 m_modFilter;
  q15_t                            m_modState[16U];    // blockSize + phaseLength - 1, 4 + 9 - 1 plus some spare
  CIL2PTXFrame                     m_frame;
  q15_t                            m_level;
  uint16_t                         m_txDelay;
  std::vector<uint16_t>            m_tokens;

  void writeByte(uint8_t c);
};

#endif

