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

#if !defined(GLOBALS_H)
#define  GLOBALS_H

#if defined(STM32F4XX)
#include "stm32f4xx.h"
#elif defined(STM32F7XX)
#include "stm32f7xx.h"
#else
#error "Unknown processor type"
#endif

#if defined(STM32F7XX)
#define  ARM_MATH_CM7
#elif defined(STM32F4XX)
#define  ARM_MATH_CM4
#else
#error "Unknown processor type"
#endif

#include <arm_math.h>

#include "SerialPort.h"
#include "AX25RX.h"
#include "AX25TX.h"
#include "Mode2RX.h"
#include "Mode2TX.h"
#include "Mode3RX.h"
#include "Mode3TX.h"
#include "Debug.h"
#include "IO.h"

const uint16_t RX_BLOCK_SIZE = 2U;

const uint16_t TX_RINGBUFFER_SIZE = 1000U;
const uint16_t RX_RINGBUFFER_SIZE = 2432U;

const uint16_t TX_BUFFER_LEN = 4000U;

extern uint8_t m_mode;

extern bool m_duplex;
extern bool m_tx;

extern CSerialPort serial;
extern CIO io;

extern CAX25RX ax25RX;
extern CAX25TX ax25TX;

extern CMode2TX mode2TX;
extern CMode2RX mode2RX;

extern CMode3TX mode3TX;
extern CMode3RX mode3RX;

#endif

