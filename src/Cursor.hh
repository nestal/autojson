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

class JsonProcessor;

/**	A Cursor is an object that points to a specific location within a hash or array, as
	well as a pointer to the current C++ object being built.

	Actually, the Cursor class contains 3 things:
	- The key which the cursor is pointing to, inside a hash or array.
	- A pointer to the C++ object being built. It's called Target().
	- The JsonVisitor that is suppose to build the host.
	
	The target (i.e. object being built) is not owned by the Cursor.
*/
class Cursor
{
public :
	template <typename TargetType>
	Cursor(const ::json::Key& key, TargetType *target, const JsonProcessor *rec) :
		m_key(key),
		m_obj(target),
		m_rec(rec),
		m_type(typeid(TargetType))
	{
		assert(m_obj);
	}
	
	explicit Cursor(const ::json::Key& key);
	explicit Cursor(const JsonProcessor *rec);
	
	Cursor(const Cursor&) = default;

	void SetKey(const ::json::Key& key);
	const ::json::Key& Key() const;
	const JsonProcessor* Rec() const;

	Cursor Forward(const JsonProcessor *rec) const;
	
	template <typename TargetType>
	void SetTarget(TargetType *target)
	{
		m_obj  = target;
		m_type = typeid(TargetType);
	}
	
	/**	Returns a pointer to the objects being built.
	
		Note that the Cursor does not own this object. Also, the caller needs to
		specify the type of the object as the template parameter. If the type
		is different from the actual type, this function will throw TypeMismatch.
		
		\throw	TypeMismatch	If the target type is different from the type
								specified in the template parameter.
	*/
	template <typename TargetType>
	TargetType* Target() const
	{
		if (m_type == typeid(TargetType))
			return static_cast<TargetType*>(m_obj);
	
		throw TypeMismatch(typeid(TargetType), m_type);
	}
	
	explicit operator bool() const;
	
private :
	::json::Key			m_key;
	void				*m_obj;	//!< The object being built by JSON
	const JsonProcessor	*m_rec;	//!< The Reactor that builds the members of the object
	std::type_index		m_type;
};

} // end of namespace

#endif
