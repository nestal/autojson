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

#include "ObjectReactor.hh"

#include <vector>

namespace ajs {

ObjectReactor::ObjectReactor() : m_next(nullptr)
{
}
	
ObjectReactor::ObjectReactor(const ObjectReactor& rhs) : m_next(rhs.m_next)
{
	for (const auto& p : rhs.m_actions)
		m_actions.insert(std::make_pair(p.first, ReactorPtr(p.second->Clone())));
}
	
ObjectReactor::ObjectReactor(ObjectReactor&& rhs) NOEXCEPT:
	m_actions(std::move(rhs.m_actions)),
	m_next(rhs.m_next)
{
}

ObjectReactor* ObjectReactor::Clone() const
{
	return new ObjectReactor(*this);
}

ParseState ObjectReactor::On(ParseState& s, JSON_event event, const char *data, std::size_t len)
{
	if (event == JSON_object_key)
	{
		auto i = m_actions.find(std::string(data,len)) ;
		m_next = (i != m_actions.end() ? i->second.get() : nullptr);
	}
	else if (m_next != nullptr)
	{
		switch (event)
		{
		case JSON_string:
		case JSON_null:
		case JSON_true:
		case JSON_false:
		case JSON_number:
			m_next->On(s, event, data, len);
			break;

		case JSON_object_start:
			// return new reactor and new dest in s
			return m_next->On(s, event, data, len);
			break;
		default:
			break;
		}
		// only use once
		m_next = nullptr;
	}
	return s;
}

} // end of namespace
