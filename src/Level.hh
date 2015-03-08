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
#include <cassert>

namespace json {

class LevelVisitor;

/**	Brief description of Level
*/
class Level
{
public :
	template <typename Host>
	Level(const ::json::Key& key, Host *host, const LevelVisitor *rec) :
		m_key(key),
		m_obj(host),
		m_rec(rec)
	{
//		assert(m_obj);
	}
	
	explicit Level(const ::json::Key& key);

	const ::json::Key& Key() const;
	void* Host() const;
	const LevelVisitor* Rec() const;
	
private :
	::json::Key			m_key;
	void				*m_obj;	//!< The object being built by JSON
	const LevelVisitor	*m_rec;	//!< The Reactor that builds the members of the object
};

} // end of namespace

#endif
