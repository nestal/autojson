/*
	songbits: A cloud-base music player
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
	m_json(::new_JSON_checker(depth))
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

void JsonParser::Parse(const char *data, size_t len, void *obj)
{
	CB cb = CB(this, obj);
	if ( ::JSON_checker_char(m_json, data, len, &Callback, &cb) == JSON_error)
		throw -1;//SB_THROW(ParseError());
}

void JsonParser::Callback(void *pvcb, JSON_event type, const char *data, size_t len)
{
	CB *cb = reinterpret_cast<CB*>(pvcb);
	assert(cb != nullptr);
	assert(cb->first != nullptr);
	cb->first->Callback(type, data, len, cb->second);
}

void JsonParser::Callback(JSON_event type, const char *data, size_t len, void *obj)
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
				m_stack.push_back(Level{Key(), obj, m_root});
			else
				m_stack.push_back(m_stack.back().rec->Advance(m_key, m_stack.back().obj));
			m_key.Clear();
			
			if (type == JSON_array_start)
				m_key.SetIndex(0);
			break;
		
		case JSON_object_end:
		case JSON_array_end:
			m_key = m_stack.back().key;
			m_stack.pop_back();
			FinishKey();
			break;
			
		case JSON_string:
		case JSON_number:
		case JSON_null:
		case JSON_true:
		case JSON_false:
			assert(!m_stack.empty());
			m_stack.back().rec->Data(m_key, type, data, len, m_stack.back().obj);
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
