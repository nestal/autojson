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

#ifndef REACTOR_HH_INCLUDED
#define REACTOR_HH_INCLUDED

#include "JSON_checker.h"
#include "LexicalCast.hh"

#include <string>

namespace ajs {

struct ParseState;

/**	Brief description of Reactor
*/
class Reactor
{
public:
	virtual ParseState On(ParseState& s, JSON_event event, const char *data, std::size_t len) = 0;
	virtual Reactor* Clone() const = 0;
};

struct ParseState
{
	Reactor	*reactor;
	void	*dest;
};

template <typename Host, typename Type>
class MemberVariable
{
public :
	MemberVariable(Type Host::*member) : m_member(member)
	{
	}

	Type& Get(const ParseState& s) const
	{
		Host *dest = reinterpret_cast<Host*>(s.dest);
		return dest->*m_member;
	}

	void Set(const ParseState& s, const Type& value) const
	{
		Host *dest = reinterpret_cast<Host*>(s.dest);
		dest->*m_member = value;
	}

private :
	Type Host::*m_member;
};

template <typename Host, typename Ret, typename Arg>
class MemberFunction
{
public :
	MemberFunction(Ret (Host::*member)(Arg)) : m_member(member)
	{
	}

	Ret Call(ParseState& s, Arg arg)
	{
		Host *dest = reinterpret_cast<Host*>(s.dest);
		return (dest->*m_member)(arg);
	}

private :
	Ret (Host::*m_member)(Arg);
};

template <typename DestType, typename T>
class SaveToMember : public Reactor
{
public :
	SaveToMember(T DestType::*member) : m_member(member)
	{
	}

	ParseState On(ParseState& s, JSON_event event, const char *data, std::size_t len) override
	{
		m_member.Set(s, lexical_cast<T>(data, len));
		return s;
	}

	SaveToMember* Clone() const override
	{
		return new SaveToMember(*this);
	}

private :
	MemberVariable<DestType, T> m_member;
};

template <typename DestType, typename R, typename T>
class SaveByCallingMember : public Reactor
{
public :
	SaveByCallingMember(R (DestType::*member)(T)) : m_member(member)
	{
	}

	ParseState On(ParseState& s, JSON_event event, const char *data, std::size_t len) override
	{
		m_member.Call(s, lexical_cast<T>(data, len)) ;
		return s;
	}

	SaveByCallingMember* Clone() const override
	{
		return new SaveByCallingMember(*this);
	}

private :
	MemberFunction<DestType, R, T>	m_member;
};

} // end of namespace

#endif
