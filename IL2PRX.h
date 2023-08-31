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

#if !defined(IL2PRX_H)
#define	IL2PRX_H

#include "IL2PRS.h"
#include "AX25CRC.h"
#include "Hamming.h"

#include <cstdint>

class CIL2PRX {
public:
  CIL2PRX();

  bool processHeader(const uint8_t* in, uint8_t* out);
  bool processPayload(const uint8_t* in, uint8_t* out);

  uint16_t getHeaderLength() const;
  uint16_t getPayloadLength() const;

  uint16_t getPayloadParityLength() const;

  bool hasCRC() const;
  bool checkCRC(const uint8_t* frame, const uint8_t* crc) const;

private:
  CIL2PRS  m_rs2;
  CIL2PRS  m_rs4;
  CIL2PRS  m_rs6;
  CIL2PRS  m_rs8;
  CIL2PRS  m_rs16;
  CAX25CRC m_crc;
  CHamming m_hamming;
  uint16_t m_headerByteCount;
  uint16_t m_payloadByteCount;
  uint8_t  m_payloadBlockCount;
  uint8_t  m_smallBlockSize;
  uint8_t  m_largeBlockSize;
  uint8_t  m_largeBlockCount;
  uint8_t  m_smallBlockCount;
  uint8_t  m_paritySymbolsPerBlock;
  uint16_t m_outOffset;
  bool     m_hasCRC;

  void calculatePayloadBlockSize();

  void processType0Header(const uint8_t* in, uint8_t* out);
  void processType1Header(const uint8_t* in, uint8_t* out);

  void unscramble(uint8_t* buffer, uint16_t length) const;

  bool decode(uint8_t* buffer, uint16_t length, uint8_t numSymbols) const;
};

#endif

