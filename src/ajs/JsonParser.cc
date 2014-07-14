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

JsonParser::JsonParser(Json& target) :
	m_target(target),
	m_parser(new_JSON_checker(5))
{
}

void JsonParser::Parse(const char *json, std::size_t len)
{
	::JSON_checker_char(m_parser, json, static_cast<int>(len),
		&JsonParser::Callback, this);
}

void JsonParser::Callback(void *user, JSON_event type, const char *data, size_t len)
{
	JsonParser *pthis = reinterpret_cast<JsonParser*>(user);
	assert(pthis != nullptr);
	
	pthis->Callback(type, data, len);
}

void JsonParser::Callback(JSON_event type, const char *data, size_t len)
{
	switch (type)
	{
	case JSON_object_start:
		if (m_stack.empty() && m_target.Is(Json::Type::hash))
			m_stack.push_back(&m_target);
		break;
	
	case JSON_object_key:
		assert(!m_stack.empty());
		assert(m_stack.back()->Is(Json::Type::hash));
		m_current = m_stack.back()->AsHash().find(std::string(data,len));
		break;
	
	case JSON_string:
		assert(m_current != m_stack.back()->AsHash().end());
		assert(m_current->second.Is(Json::Type::string));
		m_current->second.Assign(std::string(data, len));
		break;
	
	default:
		break;
	}
}

} // end of namespace
