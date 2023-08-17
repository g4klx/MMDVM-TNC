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
/* Reed-Solomon encoder/decoder
 * Copyright 2002, Phil Karn, KA9Q
 * May be used under the terms of the GNU Lesser General Public License (LGPL)
 */

#if !defined(IL2PRS_H)
#define	IL2PRS_H

#include <cstdint>

class CIL2PRS {
public:
  CIL2PRS(uint8_t nroots);
  ~CIL2PRS();

  void encode(uint8_t* data, uint8_t* parity) const;
  int  decode(uint8_t* data, uint8_t* eras_pos) const;

private:
  uint8_t        m_nroots;       /* Number of generator roots = number of parity symbols */
  const uint8_t* m_alphaTo;      /* log lookup table */
  const uint8_t* m_indexOf;      /* Antilog lookup table */
  const uint8_t* m_genpoly;      /* Generator polynomial */
};

#endif

