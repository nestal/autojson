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

#include "JsonParser.hh"

#include <cassert>
#include <iostream>

namespace json {

JsonParser::JsonParser(const LevelVisitor *root, std::size_t depth) :
	m_root(root),
	m_json(::new_JSON_checker(static_cast<int>(depth)))
{
}

JsonParser::~JsonParser()
{
}

void JsonParser::Done()
{
	::JSON_checker_done(m_json);
	m_json = ::new_JSON_checker(5);
}

void JsonParser::Parse(const char *data, size_t len)
{
	if (::JSON_checker_char(m_json, data, len, &JsonParser::Callback, this) == JSON_error)
		throw ParseError() ;
}

void JsonParser::Callback(void *pvthis, JSON_event type, const char *data, size_t len)
{
	JsonParser *pthis = reinterpret_cast<JsonParser*>(pvthis);
	assert(pthis);
	
	pthis->Callback(type, data, len);
}

Level JsonParser::Next() const
{
	Level next = m_stack.back() ; 
	next.SetKey(m_key);
	return next ;
}

void JsonParser::Callback(JSON_event type, const char *data, size_t len)
{
	switch (type)
	{
		case JSON_object_key:
			assert(!m_key);
			m_key.SetKey(std::string(data, len));
			break;
		
		case JSON_object_start:
		case JSON_array_start:
			// first call
			if (m_stack.empty())
				m_stack.push_back(m_root);
			else
			{
				m_stack.push_back(m_stack.back().Rec()->Advance(Next()));
			}
			
			m_key.Clear();
			
			if (type == JSON_array_start)
				m_key.SetIndex(0);
			break;
		
		case JSON_object_end:
		case JSON_array_end:
			assert(!m_stack.empty());
			m_key = m_stack.back().Key();
			m_stack.back().Rec()->Finish(m_stack.back());
			m_stack.pop_back();
			FinishKey();
			break;
			
		case JSON_string:
		case JSON_number:
		case JSON_null:
		case JSON_true:
		case JSON_false:
			assert(!m_stack.empty());
			assert(m_stack.back().Rec());
			m_stack.back().Rec()->Data(Next(), type, data, len);
			FinishKey();
			break;

		default:
			break;
	}
}

void JsonParser::FinishKey()
{
	// for arrays, advance to next key
	if (m_key.IsIndex())
		m_key.AdvanceIndex();
	
	// for objects, clear the key to let it set next time
	else
		m_key.Clear();
}

} // end of namespace
