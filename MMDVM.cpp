/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2023 by Jonathan Naylor G4KLX
 *   Copyright (C) 2016 by Mathis Schmieder DB9MAT
 *   Copyright (C) 2016 by Colin Durbridge G4EML
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

#if defined(STM32F4XX) || defined(STM32F7XX) || defined(STM32F105xC)

#include "Config.h"
#include "Globals.h"

uint8_t m_mode = INITIAL_MODE;

bool m_duplex       = false;
uint32_t m_slotTime = (SLOT_TIME / 10U) * 240U;
uint8_t  m_pPersist = P_PERSISTENCE;

bool m_tx  = false;
bool m_dcd = false;

CAX25RX ax25RX;
CAX25TX ax25TX;

CIL2PTX il2pTX;
CIL2PRX il2pRX;

CSerialPort serial;
CIO io;

void setup()
{
  serial.start();
}

void loop()
{
  serial.process();
  
  io.process();

  // The following is for transmitting
  switch (m_mode) {
    case 1U:
      ax25TX.process();
      break;
    case 2U:
      il2pTX.process();
      break;
  }
}

int main()
{
  setup();

  for (;;)
    loop();
}

#endif
