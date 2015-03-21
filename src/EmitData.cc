/*
	songbits: A cloud-base music player
	Copyright (C) 2015  Wan Wai Ho

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation version 2
	of the License.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
	02110-1301, USA.
*/

#include "EmitData.hh"
#include "Range.hh"

#include <cassert>
#include <iostream>

namespace json {

EmitData::EmitData() : m_start(nullptr)
{
}

void EmitData::Clear()
{
	m_tmp.clear();
	m_start = nullptr;
}

void EmitData::Save(const char *p)
{
	assert(p);
	assert(!m_start);
	m_start = p;
}

EmitData::Buf EmitData::Get(const char *p)
{
	assert(p);
	assert(IsSaved());
	
	if (m_tmp.empty())
	{
		assert(m_start);
		Buf b{ m_start, p };
		return b;
	}
	else
	{
		if (m_start)
			Stash(p);
		
		return Buf{ &*m_tmp.begin(), &*m_tmp.end() };
	}
}

void EmitData::Stash(const char *p)
{
	assert(p);
	assert(m_start);

	m_tmp.insert(m_tmp.end(), m_start, p);
	m_start = nullptr;
}

bool EmitData::IsSaved() const
{
	return m_start ;
}

bool EmitData::IsStashed() const
{
	return !m_tmp.empty();
}

} // end of namespace
