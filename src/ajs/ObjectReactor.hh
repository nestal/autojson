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

#ifndef OBJECTREACTOR_HH_INCLUDED
#define OBJECTREACTOR_HH_INCLUDED

#include "Reactor.hh"

#include <map>
#include <memory>
#include <vector>

namespace ajs {

/**	Brief description of ObjectReactor
*/
template <typename DestType>
class ObjectReactor : public Reactor
{
public:
	ObjectReactor() : m_next(nullptr)
	{
	}
	
	ObjectReactor(const ObjectReactor& rhs) : m_next(rhs.m_next)
	{
		for (const auto& p : rhs.m_actions)
			m_actions.insert(std::make_pair(p.first, ReactorPtr(p.second->Clone())));
	}
	
	ObjectReactor(ObjectReactor&& rhs) :
		m_actions(std::move(rhs.m_actions)),
		m_next(rhs.m_next)
	{
	}

	template <typename T>
	ObjectReactor& Map(const std::string& key, T DestType::*member)
	{
		m_actions.insert(
			std::make_pair(key, ReactorPtr(new SaveToMember<DestType,T>(member))));
		return *this;
	}

	ObjectReactor* Clone() const override
	{
		return new ObjectReactor(*this);
	}

	ParseState On(ParseState& s, JSON_event event, const char *data, std::size_t len) override
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
			default:
				break;
			}
			// only use once
			m_next = nullptr;
		}
		return s;
	}

private :
	typedef std::unique_ptr<Reactor> ReactorPtr;
	
	std::map<std::string, ReactorPtr> m_actions;
	Reactor *m_next;
};

} // end of namespace

#endif
