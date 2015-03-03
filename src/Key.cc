/*
	autojson: A JSON parser base on the automaton provided by json.org
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

#include "Key.hh"

#include <cassert>
#include <ostream>

namespace json {

Key::Key(const std::string& k) : m_type(key), m_key(k) {}
Key::Key(std::size_t idx) : m_type(index), m_index(idx) {}
	
std::size_t Key::Index() const
{
	return m_index;
}

std::string Key::Str() const
{
	return m_key;
}

void Key::AdvanceIndex()
{
	assert(m_type == index);
	m_index++;
}

bool Key::IsIndex() const
{
	return m_type == index;
}

bool Key::IsKey() const
{
	return m_type == key;
}

Key::operator bool() const
{
	return m_type != none;
}

void Key::SetIndex(std::size_t idx)
{
	m_type  = index;
	m_index = idx;
}

void Key::SetKey(const std::string& k)
{
	m_type = key;
	m_key  = k;
}

void Key::Clear()
{
	m_type = none;
}

bool Key::operator<(const Key& k) const
{
	if (m_type != k.m_type)
		return m_type < k.m_type;
	else if (m_type == key)
		return m_key < k.m_key;
	else
		return m_index < k.m_index;
}

std::ostream& operator<<(std::ostream& os, const Key& key)
{
	if (key.IsIndex())
		os << key.Index();
	else
		os << key.Str();
	return os;
}

} // end of namespace
