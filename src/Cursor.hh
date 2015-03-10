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

#ifndef LEVEL_HH_INCLUDED
#define LEVEL_HH_INCLUDED

#include "Key.hh"
#include "Exception.hh"

#include <cassert>
#include <typeindex>
#include <iostream>

namespace json {

class LevelVisitor;

/**	Contain information about the current
*/
class Cursor
{
public :
	template <typename HostType>
	Cursor(const ::json::Key& key, HostType *host, const LevelVisitor *rec) :
		m_key(key),
		m_obj(host),
		m_rec(rec),
		m_type(typeid(HostType))
	{
		assert(m_obj);
	}
	
	explicit Cursor(const ::json::Key& key);
	explicit Cursor(const LevelVisitor *rec);
	
	Cursor(const Cursor&) = default;

	void SetKey(const ::json::Key& key);
	const ::json::Key& Key() const;
	const LevelVisitor* Rec() const;

	template <typename HostType>
	void SetHost(HostType *host)
	{
		m_obj  = host;
		m_type = typeid(HostType);
	}
	
	template <typename HostType>
	HostType* Host() const
	{
		if (m_type == typeid(HostType))
			return static_cast<HostType*>(m_obj);
	
		throw TypeMismatch(typeid(HostType), m_type);
	}
	
private :
	::json::Key			m_key;
	void				*m_obj;	//!< The object being built by JSON
	const LevelVisitor	*m_rec;	//!< The Reactor that builds the members of the object
	std::type_index		m_type;
};

} // end of namespace

#endif
