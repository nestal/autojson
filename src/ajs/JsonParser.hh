/*
	autojson: A JSON parser base on the automaton provided by json.org
	Copyright (C) 2014  Wan Wai Ho

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

#include "JVar.hh"
#include "JSON_checker.h"

#include <memory>
#include <vector>

namespace ajs {

/**	The main parser class.
	JsonParser is the class you use to parse JSON data. First, construct a JsonParser
	object by providing a Reactor and an object that receives the output.
*/
class JsonParser
{
public :
	JsonParser(JVar& target);
	void Parse(const char *json, std::size_t len);

private :
	static void Callback(void *user, JSON_event type, const char *data, size_t len);
	void Callback(JSON_event type, const char *data, size_t len);

	JVar* NewObj(JVar&& js);

private :
	std::string				m_key;
	JVar&					m_target;
	std::vector<JVar*>		m_stack;
	JSON_checker			m_parser;
};

} // end of namespace

#endif
