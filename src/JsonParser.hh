
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

#ifndef JSONPARSER_HH_INCLUDED
#define JSONPARSER_HH_INCLUDED

#include "JSON_checker.h"
#include "TypeBuilder.hh"

#include "Exception.hh"

#include <string>
#include <vector>

namespace json {

class Reactor;
	
/**	Brief description of JsonParser
*/
class JsonParser
{
public :
	explicit JsonParser(const JsonProcessor *root, std::size_t depth = 10);
	~JsonParser();
	
	JsonParser(const JsonParser&) = delete;
	JsonParser& operator=(const JsonParser&) = delete;

	template <typename Target>
	void SetTarget(Target *target)
	{
		m_root.SetTarget(target);
	}
	
	// parsing functions
	template <typename Target>
	void Parse(const char *data, std::size_t len, Target *target)
	{
		SetTarget(target);
		Parse(data, len);
	}
	
	void Parse(const char *data, size_t len);
	void Done();
	
private:
	static void Callback(void *pvthis, JSON_event type, const char *data, size_t len);
	void Callback(JSON_event type, const char *data, size_t len);

	void FinishKey();
	Cursor Next() const ;
	
private :
	// underlying parser
	JSON_checker 		m_json;
	
	// states
	Key					m_key;
	Cursor				m_root;
	std::vector<Cursor>	m_stack;
};

} // end of namespace

#endif
