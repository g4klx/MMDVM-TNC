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

#include "TokenStore.h"

#include <cstddef>

const uint8_t MAX_TOKENS = 20U;

CTokenStore::CTokenStore() :
m_store(NULL),
m_count(0U),
m_ptr(0U)
{
  m_store = new uint16_t[MAX_TOKENS];
}

bool CTokenStore::add(uint16_t token)
{
  if (m_count == MAX_TOKENS)
    return false;

  m_store[m_count] = token;
  m_count++;

  return true;
}

void CTokenStore::reset()
{
  m_ptr = 0U;
}

bool CTokenStore::next(uint16_t& token)
{
  if (m_ptr >= m_count)
    return false;

  token = m_store[m_ptr];
  m_ptr++;

  return true;
}

void CTokenStore::clear()
{
  m_count = 0U;
  m_ptr   = 0U;
}

