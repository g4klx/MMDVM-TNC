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

#include "IL2PTX.h"
#include "Debug.h"

#include <cstring>

const uint16_t IL2P_HDR_LENGTH = 13U;

const uint16_t RS_BLOCK_LENGTH = 255U;

const uint16_t BIT_MASK_TABLE16[] = {0x0001U, 0x0002U, 0x0004U, 0x0008U, 0x0010U, 0x0020U, 0x0040U, 0x0080U, 0x0100U, 0x0200U, 0x0400U, 0x0800U, 0x1000U, 0x2000U, 0x4000U, 0x8000U};
const uint8_t BIT_MASK_TABLE8[] = {0x80U, 0x40U, 0x20U, 0x10U, 0x08U, 0x04U, 0x02U, 0x01U};

#define WRITE_BIT16(p,i,b) p = (b) ? (p | BIT_MASK_TABLE16[(i)]) : (p & ~BIT_MASK_TABLE16[(i)])
#define READ_BIT16(p,i)    (p & BIT_MASK_TABLE16[(i)])

#define WRITE_BIT8(p,i,b) p[(i)>>3] = (b) ? (p[(i)>>3] | BIT_MASK_TABLE8[(i)&7]) : (p[(i)>>3] & ~BIT_MASK_TABLE8[(i)&7])
#define READ_BIT8(p,i)    (p[(i)>>3] & BIT_MASK_TABLE8[(i)&7])

static const struct IL2P_PID {
  uint8_t ax25PID;
  uint8_t il2pPID;
} IL2P_PIDS[] = {
  {0xF0U, 0x0FU},    // No layer 3
  {0xCFU, 0x0EU},    // NET/ROM
  {0xCCU, 0x0BU},    // IP
  {0xCDU, 0x0CU},    // ARP
  {0x08U, 0x06U},    // Segmentation
  {0x01U, 0x03U},    // ROSE
  {0x06U, 0x04U},    // Compressed TCP
  {0x07U, 0x05U},    // Uncompress TCP
  {0xCEU, 0x0DU}     // FlexNet
};

CIL2PTX::CIL2PTX() :
m_rs2(2U),
m_rs4(4U),
m_rs6(6U),
m_rs8(8U),
m_rs16(16U),
m_crc(),
m_hamming(),
m_payloadByteCount(0U),
m_payloadOffset(0U),
m_payloadBlockCount(0U),
m_smallBlockSize(0U),
m_largeBlockSize(0U),
m_largeBlockCount(0U),
m_smallBlockCount(0U),
m_paritySymbolsPerBlock(0U)
{
}

uint16_t CIL2PTX::process(const uint8_t* in, uint16_t inLength, uint8_t* out)
{
  uint16_t crc = m_crc.calculate(in, inLength);

  bool type1 = isIL2PType1(in, inLength);
  if (type1)
    processType1Header(in, inLength, out);
  else
    processType0Header(in, inLength, out);

  calculatePayloadBlockSize();

  // Scramble and RS encode the IL2P header
  scramble(out, IL2P_HDR_LENGTH);
  uint8_t len = encode(out, IL2P_HDR_LENGTH, 2U);
  uint16_t outLength = len;

  for (uint8_t i = 0U; i < m_largeBlockCount; i++) {
    ::memcpy(out + outLength, in + m_payloadOffset, m_largeBlockSize);
    scramble(out + outLength, m_largeBlockSize);
    len = encode(out + outLength, m_largeBlockSize, m_paritySymbolsPerBlock);

    m_payloadOffset += m_largeBlockSize;
    outLength       += len;
  }

  for (uint8_t i = 0U; i < m_smallBlockCount; i++) {
    ::memcpy(out + outLength, in + m_payloadOffset, m_smallBlockSize);
    scramble(out + outLength, m_smallBlockSize);
    len = encode(out + outLength, m_smallBlockSize, m_paritySymbolsPerBlock);

    m_payloadOffset += m_smallBlockSize;
    outLength       += len;
  }

  out[outLength++] = m_hamming.encode(crc >> 12);
  out[outLength++] = m_hamming.encode(crc >> 8);
  out[outLength++] = m_hamming.encode(crc >> 4);
  out[outLength++] = m_hamming.encode(crc >> 0);

  return outLength;
}

bool CIL2PTX::isIL2PType1(const uint8_t* frame, uint16_t length) const
{
  // Has any digipeaters?
  if ((frame[13U] & 0x01U) == 0x00U)
    return false;

  // Has SAMBME?
  if ((length == 15U) && ((frame[14U] & 0xEFU) == 0x6FU))
    return false;

  // Has untranslatable PID?
  if ((frame[14U] & 0x01U) == 0x00U || (frame[14U] & 0xEFU) == 0x03U) {	// I or UI frames
    bool found = false;

    for (std::size_t i = 0; i < (sizeof(IL2P_PIDS) / sizeof(struct IL2P_PID)); i++) {
      if (IL2P_PIDS[i].ax25PID == frame[15U]) {
        found = true;
        break;
      }
    }

    if (!found)
      return false;
  }

  // Has untranslatable characters?
  for (uint8_t i = 0U; i < 6U; i++) {
    uint8_t c1 = frame[i + 0U] >> 1;
    uint8_t c2 = frame[i + 7U] >> 1;

    // SIXBIT compatible ASCII is 0x20 to 0x5F
    if ((c1 < 0x20U) || (c2 < 0x20U) || (c1 > 0x5FU) || (c2 > 0x5FU))
      return false;
  }

  // Check the Command/Response bits
  bool c1 = (frame[6U]  & 0x80U) == 0x80U;
  bool c2 = (frame[13U] & 0x80U) == 0x80U;
  if ((c1 && c2) || (!c1 && !c2))
    return false;

  // Check the reserved bits in the SSID
  bool s1 = (frame[6U]  & 0x60U) == 0x60U;
  bool s2 = (frame[13U] & 0x60U) == 0x60U;
  if (!s1 || !s2)
    return false;

  // How do we reject (or do we want to?) extended sequence number RRs, etc?

  return true;  
}

void CIL2PTX::processType0Header(const uint8_t* in, uint16_t length, uint8_t* out)
{
  DEBUG1("IL2PTX: type 0 header");

  ::memset(out, 0x00U, IL2P_HDR_LENGTH);

  out[0U] |= 0x80U;      // Using Max FEC

  out[2U]  = (length & 0x0200U) == 0x0200U ? 0x80U : 0x00U;
  out[3U]  = (length & 0x0100U) == 0x0100U ? 0x80U : 0x00U;
  out[4U]  = (length & 0x0080U) == 0x0080U ? 0x80U : 0x00U;
  out[5U]  = (length & 0x0040U) == 0x0040U ? 0x80U : 0x00U;
  out[6U]  = (length & 0x0020U) == 0x0020U ? 0x80U : 0x00U;
  out[7U]  = (length & 0x0010U) == 0x0010U ? 0x80U : 0x00U;
  out[8U]  = (length & 0x0008U) == 0x0008U ? 0x80U : 0x00U;
  out[9U]  = (length & 0x0004U) == 0x0004U ? 0x80U : 0x00U;
  out[10U] = (length & 0x0002U) == 0x0002U ? 0x80U : 0x00U;
  out[11U] = (length & 0x0001U) == 0x0001U ? 0x80U : 0x00U;

  m_payloadByteCount = length;
  m_payloadOffset    = 0U;
}

void CIL2PTX::processType1Header(const uint8_t* in, uint16_t length, uint8_t* out)
{
  DEBUG1("IL2PTX: type 1 header");

  ::memset(out, 0x00U, IL2P_HDR_LENGTH);

  m_payloadByteCount = 0U;
  m_payloadOffset    = 0U;

  // Convert the callsigns to SIXBIT
  for (uint8_t i = 0U; i < 6U; i++) {
    out[i + 0U] = (in[i + 0U] >> 1) - 0x20U;  // Destination callsign
    out[i + 6U] = (in[i + 7U] >> 1) - 0x20U;  // Source callsign
  }
 
  out[0U] |= 0x80U;      // Using Max FEC
  out[1U] |= 0x80U;      // It's a type 1 header

  out[12U]  = (in[13U] >> 1) & 0x0FU;  // The source SSID
  out[12U] |= (in[6U]  << 3) & 0xF0U;  // The destination SSID

  bool command = ((in[6U] & 0x80U) == 0x80U) && ((in[13U] & 0x80U) == 0x00U);

  bool hasPID  = false;
  bool hasData = false;

  if ((in[14U] & 0x01U) == 0x00U) {
    // I frame
    out[5U]  |= (in[14U] & 0x10U) == 0x10U ? 0x40U : 0x00U;
    out[6U]  |= (in[14U] & 0x80U) == 0x80U ? 0x40U : 0x00U;
    out[7U]  |= (in[14U] & 0x40U) == 0x40U ? 0x40U : 0x00U;
    out[8U]  |= (in[14U] & 0x20U) == 0x20U ? 0x40U : 0x00U;
    out[9U]  |= (in[14U] & 0x08U) == 0x08U ? 0x40U : 0x00U;
    out[10U] |= (in[14U] & 0x04U) == 0x04U ? 0x40U : 0x00U;
    out[11U] |= (in[14U] & 0x02U) == 0x02U ? 0x40U : 0x00U;
    hasPID    = true;
    hasData   = true;
  } else if ((in[14U] & 0x03U) == 0x01U) {
    // S frame
    // Also a PID of 0x00 but that's default anyway
    out[6U]  |= (in[14U] & 0x80U) == 0x80U ? 0x40U : 0x00U;
    out[7U]  |= (in[14U] & 0x40U) == 0x40U ? 0x40U : 0x00U;
    out[8U]  |= (in[14U] & 0x20U) == 0x20U ? 0x40U : 0x00U;
    out[9U]  |= command                    ? 0x40U : 0x00U;
    out[10U] |= (in[14U] & 0x08U) == 0x08U ? 0x40U : 0x00U;
    out[11U] |= (in[14U] & 0x04U) == 0x04U ? 0x40U : 0x00U;
  } else {
    // U frame
    switch (in[14U] & 0xEFU) {
      case 0x2FU:  // SABM
        out[4U] |= 0x40U;	// A PID of 0x01
        out[5U] |= 0x40U;	// Poll
        out[9U] |= 0x40U;
        break;
      case 0x43U:  // DISC
        out[4U] |= 0x40U;	// A PID of 0x01
        out[5U] |= 0x40U;	// Poll
        out[8U] |= 0x40U;
        out[9U] |= 0x40U;
        break;
      case 0x0FU:  // DM
        out[4U] |= 0x40U;	// A PID of 0x01
        out[5U] |= 0x40U;	// Final
        out[7U] |= 0x40U;
        break;
      case 0x63U:  // UA
        out[4U] |= 0x40U;	// A PID of 0x01
        out[5U] |= 0x40U;	// Final
        out[7U] |= 0x40U;
        out[8U] |= 0x40U;
        break;
      case 0x87U:  // FRMR
        out[4U] |= 0x40U;	// A PID of 0x01
        out[5U] |= 0x40U;	// Final
        out[6U] |= 0x40U;
        hasData  = true;
        break;
      case 0x03U:  // UI
        out[5U] |= (in[14U] & 0x10U) == 0x10U ? 0x40U : 0x00U;
        out[6U] |= 0x40U;
        out[8U] |= 0x40U;
        out[9U] |= command ? 0x40U : 0x00U;
        out[0U] |= 0x40U;  // UI bit
        hasPID   = true;
        hasData  = true;
        break;
      case 0xAFU:  // XID
        out[4U] |= 0x40U;	// A PID of 0x01
        out[5U] |= (in[14U] & 0x10U) == 0x10U ? 0x40U : 0x00U;
        out[6U] |= 0x40U;
        out[7U] |= 0x40U;
        out[9U] |= command ? 0x40U : 0x00U;
        hasData  = true;
        break;
      case 0xE3U:  // TEST
        out[4U] |= 0x40U;	// A PID of 0x01
        out[5U] |= (in[14U] & 0x10U) == 0x10U ? 0x40U : 0x00U;
        out[6U] |= 0x40U;
        out[7U] |= 0x40U;
        out[8U] |= 0x40U;
        out[9U] |= command ? 0x40U : 0x00U;
        hasData  = true;
        break;
    }
  }

  if (hasPID) {      // We have a PID and maybe data
    for (std::size_t i = 0; i < (sizeof(IL2P_PIDS) / sizeof(struct IL2P_PID)); i++) {
      if (IL2P_PIDS[i].ax25PID == in[15U]) {
        out[1U] |= (IL2P_PIDS[i].il2pPID & 0x08U) == 0x08U ? 0x40U : 0x00U;
        out[2U] |= (IL2P_PIDS[i].il2pPID & 0x04U) == 0x04U ? 0x40U : 0x00U;
        out[3U] |= (IL2P_PIDS[i].il2pPID & 0x02U) == 0x02U ? 0x40U : 0x00U;
        out[4U] |= (IL2P_PIDS[i].il2pPID & 0x01U) == 0x01U ? 0x40U : 0x00U;
        break;
      }
    }
  }

  if (hasData) {
    if (hasPID) {
      m_payloadByteCount = length - 16U;
      m_payloadOffset    = 16U;
    } else {
      m_payloadByteCount = length - 15U;
      m_payloadOffset    = 15U;
    }

    if (m_payloadByteCount > 0U) {
      // The default payload length in the header is 0 so we don't need to update it unless we have data
      out[2U]  |= (m_payloadByteCount & 0x0200U) == 0x0200U ? 0x80U : 0x00U;
      out[3U]  |= (m_payloadByteCount & 0x0100U) == 0x0100U ? 0x80U : 0x00U;
      out[4U]  |= (m_payloadByteCount & 0x0080U) == 0x0080U ? 0x80U : 0x00U;
      out[5U]  |= (m_payloadByteCount & 0x0040U) == 0x0040U ? 0x80U : 0x00U;
      out[6U]  |= (m_payloadByteCount & 0x0020U) == 0x0020U ? 0x80U : 0x00U;
      out[7U]  |= (m_payloadByteCount & 0x0010U) == 0x0010U ? 0x80U : 0x00U;
      out[8U]  |= (m_payloadByteCount & 0x0008U) == 0x0008U ? 0x80U : 0x00U;
      out[9U]  |= (m_payloadByteCount & 0x0004U) == 0x0004U ? 0x80U : 0x00U;
      out[10U] |= (m_payloadByteCount & 0x0002U) == 0x0002U ? 0x80U : 0x00U;
      out[11U] |= (m_payloadByteCount & 0x0001U) == 0x0001U ? 0x80U : 0x00U;
    }
  }
}

void CIL2PTX::calculatePayloadBlockSize()
{
  if (m_payloadByteCount == 0U) {
    m_payloadBlockCount = 0U;
    m_smallBlockSize = 0U;
    m_largeBlockSize = 0U;
    m_largeBlockCount = 0U;
    m_smallBlockCount = 0U;
    m_paritySymbolsPerBlock = 0U;
  } else {
    m_payloadBlockCount  =  m_payloadByteCount / 239U;
    m_payloadBlockCount += (m_payloadByteCount % 239U) > 0U ? 1U : 0U;

    m_smallBlockSize = m_payloadByteCount / m_payloadBlockCount;
    m_largeBlockSize = m_smallBlockSize + 1U;

    m_largeBlockCount = m_payloadByteCount - (m_payloadBlockCount * m_smallBlockSize);
    m_smallBlockCount = m_payloadBlockCount - m_largeBlockCount;

    m_paritySymbolsPerBlock = 16U;
  }
}

void CIL2PTX::scramble(uint8_t* buffer, uint16_t length) const
{
  const uint16_t bitLength = length * 8U;

  uint16_t sr = 0x000FU;

  uint16_t pos = 0U;
  for (uint16_t i = 0U; i < bitLength; i++) {
    bool b  = READ_BIT8(buffer, i) != 0U;
    bool fb = READ_BIT16(sr, 0U) != 0U;

    sr >>= 1;

    WRITE_BIT16(sr, 8U, b);

    if (fb)
      sr ^= 0x0108U;

    b = READ_BIT16(sr, 3U) != 0U;

    if (i > 4U) {
      WRITE_BIT8(buffer, pos, b);
      pos++;
    }
  }

  for (uint8_t i = 0U; i < 5U; i++, pos++) {
    bool fb = READ_BIT16(sr, 0U) != 0U;

    sr >>= 1;

    if (fb)
      sr ^= 0x0108U;

    bool b = READ_BIT16(sr, 3U) != 0U;
    WRITE_BIT8(buffer, pos, b);
  }
}

uint8_t CIL2PTX::encode(uint8_t* buffer, uint16_t length, uint8_t numSymbols) const
{
  uint8_t rsBlock[RS_BLOCK_LENGTH];
  ::memset(rsBlock, 0x00U, RS_BLOCK_LENGTH);
  ::memcpy(rsBlock + RS_BLOCK_LENGTH - length - numSymbols, buffer, length);

  uint8_t parity[16U];

  switch (numSymbols) {
    case 2U:
      m_rs2.encode(rsBlock, parity);
      break;
    case 4U:
      m_rs4.encode(rsBlock, parity);
      break;
    case 6U:
      m_rs6.encode(rsBlock, parity);
      break;
    case 8U:
      m_rs8.encode(rsBlock, parity);
      break;
    case 16U:
      m_rs16.encode(rsBlock, parity);
      break;
  }

  ::memcpy(buffer + length, parity, numSymbols);

  return length + numSymbols;
}

