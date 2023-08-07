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

#if !defined(KISSDEFINES_H)
#define  KISSDEFINES_H

#include <cstdint>

const uint8_t KISS_FEND  = 0xC0U;
const uint8_t KISS_FESC  = 0xDBU;
const uint8_t KISS_TFEND = 0xDCU;
const uint8_t KISS_TFESC = 0xDDU;

const uint8_t KISS_TYPE_DATA           = 0x00U;
const uint8_t KISS_TYPE_TX_DELAY       = 0x01U;
const uint8_t KISS_TYPE_P_PERSISTENCE  = 0x02U;
const uint8_t KISS_TYPE_SLOT_TIME      = 0x03U;
const uint8_t KISS_TYPE_TX_TAIL        = 0x04U;
const uint8_t KISS_TYPE_FULL_DUPLEX    = 0x05U;
const uint8_t KISS_TYPE_DATA_WITH_ACK  = 0x0CU;
const uint8_t KISS_TYPE_ACK            = 0x0CU;
const uint8_t KISS_TYPE_POLL           = 0x0EU;

#endif

