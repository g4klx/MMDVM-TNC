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

#include "IL2PRXFrame.h"

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

CIL2PRXFrame::CIL2PRXFrame() :
m_rs2(2U),
m_rs4(4U),
m_rs6(6U),
m_rs8(8U),
m_rs16(16U),
m_headerByteCount(0U),
m_payloadByteCount(0U),
m_payloadOffset(0U),
m_payloadBlockCount(0U),
m_smallBlockSize(0U),
m_largeBlockSize(0U),
m_largeBlockCount(0U),
m_smallBlockCount(0U),
m_paritySymbolsPerBlock(0U),
m_outOffset(0U)
{
}

CIL2PRXFrame::~CIL2PRXFrame()
{
}

bool CIL2PRXFrame::processHeader(const uint8_t* in, uint8_t* out)
{
  uint8_t buffer[20U];
  ::memcpy(buffer, in, IL2P_HDR_LENGTH + 2U);
  bool ok = decode(buffer, IL2P_HDR_LENGTH, 2U);
  if (!ok)
    return false;

  unscramble(buffer, IL2P_HDR_LENGTH);

  bool type1 = (buffer[1U] & 0x80U) == 0x80U;
  if (type1)
    processType1Header(buffer, out);
  else
    processType0Header(buffer, out);

  // Sanity check
  if (m_payloadByteCount > 1023U)
    return false;

  bool maxFEC = (buffer[0U] & 0x80U) == 0x80U;

  m_outOffset = m_headerByteCount;
  m_payloadOffset = IL2P_HDR_LENGTH + 2U;

  calculatePayloadBlockSize(maxFEC);

  return true;
}

bool CIL2PRXFrame::processPayload(const uint8_t* in, uint8_t* out)
{
  for (uint8_t i = 0U; i < m_largeBlockCount; i++) {
    ::memcpy(out + m_outOffset, in + m_payloadOffset, m_largeBlockSize + m_paritySymbolsPerBlock);
    bool ok = decode(out + m_outOffset, m_largeBlockSize, m_paritySymbolsPerBlock);
    if (!ok)
      return false;

    unscramble(out + m_outOffset, m_largeBlockSize);

    m_payloadByteCount -= m_largeBlockSize;
    m_payloadOffset    += m_largeBlockSize + m_paritySymbolsPerBlock;
    m_outOffset        += m_largeBlockSize;
  }

  for (uint8_t i = 0U; i < m_smallBlockCount; i++) {
    ::memcpy(out + m_outOffset, in + m_payloadOffset, m_smallBlockSize + m_paritySymbolsPerBlock);
    bool ok = decode(out + m_outOffset, m_smallBlockSize, m_paritySymbolsPerBlock);
    if (!ok)
      return false;

    unscramble(out + m_outOffset, m_smallBlockSize);

    m_payloadByteCount -= m_smallBlockSize;
    m_payloadOffset    += m_smallBlockSize + m_paritySymbolsPerBlock;
    m_outOffset        += m_smallBlockSize;
  }

  return true;
}

uint16_t CIL2PRXFrame::getHeaderLength() const
{
  return m_headerByteCount;
}

uint16_t CIL2PRXFrame::getPayloadLength() const
{
  return m_payloadByteCount;
}

uint16_t CIL2PRXFrame::getHeaderParityLength() const
{
  return 2U;
}

uint16_t CIL2PRXFrame::getPayloadParityLength() const
{
  return m_paritySymbolsPerBlock * (m_largeBlockCount + m_smallBlockCount);
}

void CIL2PRXFrame::calculatePayloadBlockSize(bool max)
{
  if (m_payloadByteCount == 0U) {
    m_payloadBlockCount = 0U;
    m_smallBlockSize = 0U;
    m_largeBlockSize = 0U;
    m_largeBlockCount = 0U;
    m_smallBlockCount = 0U;
    m_paritySymbolsPerBlock = 0U;
    return;
  }

  if (max) {
    m_payloadBlockCount  =  m_payloadByteCount / 239U;
    m_payloadBlockCount += (m_payloadByteCount % 239U) > 0U ? 1U : 0U;

    m_smallBlockSize = m_payloadByteCount / m_payloadBlockCount;
    m_largeBlockSize = m_smallBlockSize + 1U;

    m_largeBlockCount = m_payloadByteCount - (m_payloadBlockCount * m_smallBlockSize);
    m_smallBlockCount = m_payloadBlockCount - m_largeBlockCount;

    m_paritySymbolsPerBlock = 16U;
  } else {
    m_payloadBlockCount  =  m_payloadByteCount / 247U;
    m_payloadBlockCount += (m_payloadByteCount % 247U) > 0U ? 1U : 0U;

    m_smallBlockSize = m_payloadByteCount / m_payloadBlockCount;
    m_largeBlockSize = m_smallBlockSize + 1U;

    m_largeBlockCount = m_payloadByteCount - (m_payloadBlockCount * m_smallBlockSize);
    m_smallBlockCount = m_payloadBlockCount - m_largeBlockCount;

    m_paritySymbolsPerBlock = (m_smallBlockSize / 32U) + 2U;
  }
}

void CIL2PRXFrame::processType0Header(const uint8_t* in, uint8_t* out)
{
  m_headerByteCount  = 0U;
  m_payloadByteCount = 0U;

  m_payloadByteCount |= (in[2U]  & 0x80U) == 0x80U ? 0x0200U : 0x0000U;
  m_payloadByteCount |= (in[3U]  & 0x80U) == 0x80U ? 0x0100U : 0x0000U;
  m_payloadByteCount |= (in[4U]  & 0x80U) == 0x80U ? 0x0080U : 0x0000U;
  m_payloadByteCount |= (in[5U]  & 0x80U) == 0x80U ? 0x0040U : 0x0000U;
  m_payloadByteCount |= (in[6U]  & 0x80U) == 0x80U ? 0x0020U : 0x0000U;
  m_payloadByteCount |= (in[7U]  & 0x80U) == 0x80U ? 0x0010U : 0x0000U;
  m_payloadByteCount |= (in[8U]  & 0x80U) == 0x80U ? 0x0008U : 0x0000U;
  m_payloadByteCount |= (in[9U]  & 0x80U) == 0x80U ? 0x0004U : 0x0000U;
  m_payloadByteCount |= (in[10U] & 0x80U) == 0x80U ? 0x0002U : 0x0000U;
  m_payloadByteCount |= (in[11U] & 0x80U) == 0x80U ? 0x0001U : 0x0000U;
}

void CIL2PRXFrame::processType1Header(const uint8_t* in, uint8_t* out)
{
  m_payloadByteCount = 0U;

  ::memset(out, 0x00U, 15U);

  // Convert the callsigns to shifted ASCII
  for (uint8_t i = 0U; i < 6U; i++) {
    out[i + 0U] = ((in[i + 0U] & 0x3FU) + 0x20U) << 1;  // Destination callsign
    out[i + 7U] = ((in[i + 6U] & 0x3FU) + 0x20U) << 1;  // Source callsign
  }

  out[6U]   = (in[12U] & 0xF0U) >> 3;	// The destination SSID
  out[6U]  |= 0x60U;			// Set R bits

  out[13U]  = (in[12U] & 0x0FU) << 1;	// The source SSID
  out[13U] |= 0x60U;			// Set R bits
  out[13U] |= 0x01U;			// End of address marker

  uint8_t control = 0U;
  control |= (in[5U]  & 0x40U) == 0x40U ? 0x40U : 0x00U;
  control |= (in[6U]  & 0x40U) == 0x40U ? 0x20U : 0x00U;
  control |= (in[7U]  & 0x40U) == 0x40U ? 0x10U : 0x00U;
  control |= (in[8U]  & 0x40U) == 0x40U ? 0x08U : 0x00U;
  control |= (in[9U]  & 0x40U) == 0x40U ? 0x04U : 0x00U;
  control |= (in[10U] & 0x40U) == 0x40U ? 0x02U : 0x00U;
  control |= (in[11U] & 0x40U) == 0x40U ? 0x01U : 0x00U;

  uint8_t pid = 0U;
  pid |= (in[1U] & 0x40U) == 0x40U ? 0x08U : 0x00U;
  pid |= (in[2U] & 0x40U) == 0x40U ? 0x04U : 0x00U;
  pid |= (in[3U] & 0x40U) == 0x40U ? 0x02U : 0x00U;
  pid |= (in[4U] & 0x40U) == 0x40U ? 0x01U : 0x00U;

  bool hasPID  = false;
  bool hasData = false;

  if (pid == 0x00U) {
    // S frame
    switch (control & 0x03U) {
      case 0x00U:		// RR
        out[14U] = 0x01U;
        break;
      case 0x01U:		// RNR
        out[14U] = 0x05U;
        break;
      case 0x02U:		// REJ
        out[14U] = 0x09U;
        break;
      case 0x03U:		// SREJ
        out[14U] = 0x0DU;
        break;
    }
    out[14U] |= (control & 0x20U) == 0x20U ? 0x80U : 0x00U;
    out[14U] |= (control & 0x20U) == 0x10U ? 0x40U : 0x00U;
    out[14U] |= (control & 0x20U) == 0x08U ? 0x20U : 0x00U;
    out[6U]  |= (control & 0x04U) == 0x04U ? 0x80U : 0x00U;
  } else if (pid == 0x01U) {
    // U frame except for UI
    switch (control & 0x38U) {
      case 0x00U:		// SABM
        out[14U] = 0x3FU;
        out[6U] |= 0x80U;	// Command
        break;
      case 0x08U:		// DISC
        out[14U] = 0x53U;
        out[6U] |= 0x80U;	// Command
        break;
      case 0x10U:		// DM
        out[14U]  = 0x1FU;
        break;
      case 0x18U:		// UA
        out[14U]  = 0x73U;
        break;
      case 0x20U:		// FRMR
        out[14U]  = 0x97U;
        hasData   = true;
        break;
      case 0x30U:		// XID
        out[14U]  = 0xAFU;
        out[14U] |= (control & 0x40U) == 0x40U ? 0x10U : 0x00U;
        out[6U]  |= (control & 0x04U) == 0x04U ? 0x80U : 0x00U;
        out[13U] |= (control & 0x04U) == 0x00U ? 0x80U : 0x00U;
        hasData   = true;
        break;
      case 0x38U:		// TEST
        out[14U]  = 0xE3U;
        out[14U] |= (control & 0x40U) == 0x40U ? 0x10U : 0x00U;
        out[6U]  |= (control & 0x04U) == 0x04U ? 0x80U : 0x00U;
        out[13U] |= (control & 0x04U) == 0x00U ? 0x80U : 0x00U;
        hasData   = true;
        break;
    }
  } else if ((in[0U] & 0x40U) == 0x40U) {
      // UI frame
      out[14U]  = 0x03U;
      out[14U] |= (control & 0x40U) == 0x40U ? 0x10U : 0x00U;
      out[6U]  |= (control & 0x04U) == 0x04U ? 0x80U : 0x00U;
      out[13U] |= (control & 0x04U) == 0x00U ? 0x80U : 0x00U;
      hasPID  = true;
      hasData = true;
  } else {
      // I frame
      out[14U]  = 0x00U;
      out[14U] |= (control & 0x40U) == 0x40U ? 0x10U : 0x00U;
      out[14U] |= (control & 0x20U) == 0x20U ? 0x80U : 0x00U;
      out[14U] |= (control & 0x10U) == 0x10U ? 0x40U : 0x00U;
      out[14U] |= (control & 0x08U) == 0x08U ? 0x20U : 0x00U;
      out[14U] |= (control & 0x04U) == 0x04U ? 0x08U : 0x00U;
      out[14U] |= (control & 0x02U) == 0x02U ? 0x04U : 0x00U;
      out[14U] |= (control & 0x01U) == 0x01U ? 0x02U : 0x00U;
      out[6U]  |= (control & 0x04U) == 0x04U ? 0x80U : 0x00U;
      out[13U] |= (control & 0x04U) == 0x00U ? 0x80U : 0x00U;
      hasPID  = true;
      hasData = true;
  }

  if (hasPID) {      // We have a PID
    for (std::size_t i = 0; i < (sizeof(IL2P_PIDS) / sizeof(struct IL2P_PID)); i++) {
      if (IL2P_PIDS[i].il2pPID == pid) {
        out[15U] = IL2P_PIDS[i].ax25PID;
        break;
      }
    }
  }

  if (hasData) {
    m_payloadByteCount |= (in[2U]  & 0x80U) == 0x80U ? 0x0200U : 0x0000U;
    m_payloadByteCount |= (in[3U]  & 0x80U) == 0x80U ? 0x0100U : 0x0000U;
    m_payloadByteCount |= (in[4U]  & 0x80U) == 0x80U ? 0x0080U : 0x0000U;
    m_payloadByteCount |= (in[5U]  & 0x80U) == 0x80U ? 0x0040U : 0x0000U;
    m_payloadByteCount |= (in[6U]  & 0x80U) == 0x80U ? 0x0020U : 0x0000U;
    m_payloadByteCount |= (in[7U]  & 0x80U) == 0x80U ? 0x0010U : 0x0000U;
    m_payloadByteCount |= (in[8U]  & 0x80U) == 0x80U ? 0x0008U : 0x0000U;
    m_payloadByteCount |= (in[9U]  & 0x80U) == 0x80U ? 0x0004U : 0x0000U;
    m_payloadByteCount |= (in[10U] & 0x80U) == 0x80U ? 0x0002U : 0x0000U;
    m_payloadByteCount |= (in[11U] & 0x80U) == 0x80U ? 0x0001U : 0x0000U;
  }

  m_headerByteCount = hasPID ? 16U : 15U;
}

void CIL2PRXFrame::unscramble(uint8_t* buffer, uint16_t length) const
{
  const uint16_t bitLength = length * 8U;

  uint16_t sr = 0x01F0U;

  for (uint16_t i = 0U; i < bitLength; i++) {
    bool b = READ_BIT8(buffer, i) != 0U;

    if (b)
      sr ^= 0x0211U;

    b = READ_BIT16(sr, 0U) != 0U;
    
    sr >>= 1;

    WRITE_BIT8(buffer, i, b);
  }
}

bool CIL2PRXFrame::decode(uint8_t* buffer, uint16_t length, uint8_t numSymbols) const
{
  uint16_t n = length + numSymbols;

  uint8_t rsBlock[RS_BLOCK_LENGTH];
  ::memset(rsBlock, 0x00U, RS_BLOCK_LENGTH - n);
  ::memcpy(rsBlock + RS_BLOCK_LENGTH - n, buffer, n);

  uint8_t derrlocs[16U];
  ::memset(derrlocs, 0x00U, 16U * sizeof(uint8_t));
  int derrors = 0;

  switch (numSymbols) {
    case 2U:
      derrors = m_rs2.decode(rsBlock, derrlocs);
      break;
    case 4U:
      derrors = m_rs4.decode(rsBlock, derrlocs);
      break;
    case 6U:
      derrors = m_rs6.decode(rsBlock, derrlocs);
      break;
    case 8U:
      derrors = m_rs8.decode(rsBlock, derrlocs);
      break;
    case 16U:
      derrors = m_rs16.decode(rsBlock, derrlocs);
      break;
  }

  ::memcpy(buffer, rsBlock + RS_BLOCK_LENGTH - n, length);

  // It is possible to have a situation where too many errors are
  // present but the algorithm could get a good code block by "fixing"
  // one of the padding bytes that should be 0.
  for (int i = 0; i < derrors; i++) {
    if (derrlocs[i] < (RS_BLOCK_LENGTH - n))
      return false;
  }

  return true;
}

