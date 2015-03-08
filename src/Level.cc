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

#include "Level.hh"
#include "LevelVisitor.hh"

namespace json {

Level::Level(const LevelVisitor *rec) :
	m_obj(nullptr),
	m_rec(rec),
	m_type(typeid(void))
{
}
	
Level::Level(const ::json::Key& key) :
	m_key(key),
	m_obj(nullptr),
	m_rec(MockObjectHandler::Instance()),
	m_type(typeid(void))
{
}
	
const ::json::Key& Level::Key() const
{
	return m_key;
}

const LevelVisitor* Level::Rec() const
{
	return m_rec;
}

void Level::SetKey(const ::json::Key& key)
{
	m_key = key;
}

} // end of namespace
