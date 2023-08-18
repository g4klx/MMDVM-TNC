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

#include "IL2PRXFrame.h"

class CIL2PRX {
public:
  CIL2PRX();

  void samples(q15_t* samples, uint8_t length);

  bool canTX() const;

private:

  arm_fir_instance_q15 m_rrc02Filter;
  q15_t                m_rrc02State[70U];         // NoTaps + BlockSize - 1, 42 + 20 - 1 plus some spare
  CIL2PRXFrame         m_frame;
  uint32_t             m_slotCount;
  bool                 m_dcd;
  bool                 m_canTX;
  uint8_t              m_x;
  uint8_t              m_a;
  uint8_t              m_b;
  uint8_t              m_c;

  void initRand();
  uint8_t rand();
};

#endif

