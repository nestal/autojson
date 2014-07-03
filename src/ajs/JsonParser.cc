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

#include "JsonParser.hh"

#include <cassert>

namespace ajs {

void JsonParser::Parse(const char *json, std::size_t len)
{
	::JSON_checker_char(m_parser, json, static_cast<int>(len),
		&JsonParser::Callback, this);
}

void JsonParser::Callback(void *user, JSON_event type, const char *data, size_t len)
{
	JsonParser *parser = reinterpret_cast<JsonParser*>(user);
	assert(parser != nullptr);
	assert(!parser->m_stack.empty());

	ParseState current	= parser->m_stack.back();
	ParseState next		= current.reactor->On(current, type, data, len);
	
	if (next.reactor != current.reactor)
		parser->m_stack.push_back(next) ;
	
	else if (next.reactor == nullptr)
		parser->m_stack.pop_back();
}

} // end of namespace
