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

#ifndef KEY_HH_INCLUDED
#define KEY_HH_INCLUDED

#include <string>
#include <iosfwd>

namespace json {

/**	Brief description of Key
*/
class Key
{
public:
	Key() ;
	
	template <std::size_t n>
	Key(const char (&str)[n]) : m_type(key), m_index(0), m_key(str) {}
	
	Key(const std::string& k);
	Key(std::size_t idx);
	
	std::size_t Index() const;
	std::string Str() const;

	bool IsIndex() const;
	bool IsKey() const;
	explicit operator bool() const;

	void SetIndex(std::size_t idx);
	void AdvanceIndex();
	void SetKey(const std::string& k);
	void Clear();

	// for std::map
	bool operator<(const Key& k) const;

private:
	enum Type {none, key, index} m_type;
	std::string m_key;
	std::size_t m_index;
};

std::ostream& operator<<(std::ostream& os, const Key& key);

} // end of namespace

#endif
