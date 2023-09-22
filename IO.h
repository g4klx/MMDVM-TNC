/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2023 by Jonathan Naylor G4KLX
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

#if !defined(IO_H)
#define  IO_H

#include "Globals.h"

#include "RingBuffer.h"


class CIO {
public:
  CIO();

  void start();

  void process();

  void write(q15_t* samples, uint16_t length);

  void showMode();

  uint16_t getSpace() const;

  void setDecode(bool dcd);
  
  void interrupt();

  void setRXLevel(uint8_t value);
  void setPPersist(uint8_t value);
  void setSlotTime(uint8_t value);

  uint8_t getCPU() const;

  void getUDID(uint8_t* buffer);

  void selfTest();

  bool canTX() const;

private:
  CRingBuffer<uint16_t>  m_rxBuffer;
  CRingBuffer<uint16_t>  m_txBuffer;

  q15_t                  m_rxLevel;
  uint8_t                m_pPersist;
  uint32_t               m_slotTime;

  bool                   m_dcd;

  uint32_t               m_ledCount;
  bool                   m_ledValue;

  uint32_t               m_slotCount;
  bool                   m_canTX;
  uint8_t                m_x;
  uint8_t                m_a;
  uint8_t                m_b;
  uint8_t                m_c;
  
  void    initRand();
  uint8_t rand();

  // Hardware specific routines
  void initInt();
  void startInt();

  void setLEDInt(bool on);
  void setPTTInt(bool on);
  void setCOSInt(bool on);

  void setMode1Int(bool on);
  void setMode2Int(bool on);
  void setMode3Int(bool on);
  void setMode4Int(bool on);
  
  void delayInt(unsigned int dly);
};

#endif
