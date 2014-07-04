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

namespace ajs {

template <typename DestType, typename T, typename R>
class SubObjectReactor : public Reactor
{
public :
	SubObjectReactor(MemberVariable<DestType,T> member, const R& reactor) : m_member(member), m_reactor(reactor)
	{
	}

	ParseState On(ParseState& s, JSON_event event, const char *data, std::size_t len) override
	{
		return ParseState {&m_reactor, &m_member.Get(s)};
	}

	SubObjectReactor* Clone() const override
	{
		return new SubObjectReactor(*this);
	}

private :
	MemberVariable<DestType,T>	m_member;
	R m_reactor;
};

/**	Brief description of ObjectReactor
*/
class ObjectReactor : public Reactor
{
public:
	ObjectReactor();
	ObjectReactor(const ObjectReactor& rhs);
	ObjectReactor(ObjectReactor&& rhs) NOEXCEPT;

	template <typename T, typename DestType>
	ObjectReactor& Map(const std::string& key, T DestType::*member)
	{
		m_actions.insert(
			std::make_pair(key, ReactorPtr(new SaveToMember<DestType,T>(member))));
		return *this;
	}

	template <typename T, typename R, typename DestType>
	ObjectReactor& Map(const std::string& key, R (DestType::*member)(T))
	{
		m_actions.insert(
			std::make_pair(key, ReactorPtr(new SaveByCallingMember<DestType,R,T>(member))));
		return *this;
	}

	template <typename T, typename DestType>
	ObjectReactor& Map(const std::string& key, T DestType::*member, const ObjectReactor& r)
	{
		m_actions.insert(
			std::make_pair(key, ReactorPtr(new SubObjectReactor<DestType, T, ObjectReactor>(member, r))));
		return *this;
	}

	template <typename DestType, typename Callable>
	ObjectReactor& Map(const std::string& key, Callable func)
	{
		m_actions.insert(
			std::make_pair(key, ReactorPtr(MakeCallbackReactor<DestType>(func))));;
		return *this;
	}

	ObjectReactor* Clone() const override;
	ParseState On(ParseState& s, JSON_event event, const char *data, std::size_t len) override;

private :
	typedef std::unique_ptr<Reactor> ReactorPtr;
	
	std::map<std::string, ReactorPtr> m_actions;
	Reactor *m_next;
};

} // end of namespace

#endif
