/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2021,2023,2024 by Jonathan Naylor G4KLX
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

#if !defined(SERIALPORT_H)
#define  SERIALPORT_H

#include "Config.h"
#include "Globals.h"

#if !defined(SERIAL_SPEED)
#define SERIAL_SPEED 115200
#endif


class CSerialPort {
public:
  CSerialPort();

  void start();

  void process();

  void writeKISSData(uint8_t type, const uint8_t* data, uint16_t length);
  void writeKISSAck(uint16_t token);

  void writeDebug(const char* text);
  void writeDebug(const char* text, int16_t n1);
  void writeDebug(const char* text, int16_t n1, int16_t n2);
  void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3);
  void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4);
  void writeDebug(const char* text, int16_t n1, int16_t n2, int16_t n3, int16_t n4, int16_t n5);

private:
  uint8_t  m_buffer[2000U];
  uint16_t m_ptr;
  bool     m_inFrame;
  bool     m_isEscaped;
  bool     m_debug;

  void processMessage();

#if defined(SERIAL_DEBUGGING)
  void writeDebugInt(const char* text);
  void writeDebugInt(int16_t num);
  void reverse(char* buffer, uint8_t length) const;
#endif

  // Hardware versions
  void    beginInt(uint8_t n, int speed);
  int     availableForReadInt(uint8_t n);
  int     availableForWriteInt(uint8_t n);
  uint8_t readInt(uint8_t n);
  void    writeInt(uint8_t n, const uint8_t* data, uint16_t length, bool flush = false);
};

#endif
