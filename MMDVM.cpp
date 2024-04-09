/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2023,2024 by Jonathan Naylor G4KLX
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

#include "Config.h"
#include "Globals.h"

uint8_t m_mode = INITIAL_MODE;

bool m_duplex = (DUPLEX == 1);
bool m_tx = false;

CAX25RX ax25RX;
CAX25TX ax25TX;

CModeNTX modeNTX;
CModeNRX modeNRX;

CSerialPort serial;
CIO io;

void setup()
{
  io.start();

  serial.start();
}

void loop()
{
  serial.process();
  
  io.process();

  // The following is for transmitting
  if (m_mode == 1U)
    ax25TX.process();
  else
    modeNTX.process();
}

int main()
{
  setup();

  for (;;)
    loop();
}

