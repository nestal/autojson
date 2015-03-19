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

#ifndef RANGE_HH_INCLUDED
#define RANGE_HH_INCLUDED

#include <iterator>
#include <cstddef>

namespace json {

/**	Brief description of Range
*/
template <typename T>
class Range
{
public :
	Range() = default;
	Range(T begin, T end) : m_begin(begin), m_end(end) {}
	
	T begin() const { return m_begin; }
	T end() const { return m_end; }

	std::size_t size() const { return std::distance(m_begin, m_end); }
	
private :
	T	m_begin;
	T	m_end;
};

} // end of namespace

#endif
