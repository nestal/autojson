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

#include <string>

namespace ajs {

class ParseState;

/**	Brief description of Reactor
*/
class Reactor
{
public:
	virtual ParseState On(ParseState& s, JSON_event event, const char *data, std::size_t len) = 0;
};

struct ParseState
{
	Reactor	*handler;
	void	*ptr;
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
		DestType *dest = reinterpret_cast<DestType*>(s.ptr);
		(dest->*m_member) = std::string(data, len);
		return s;
	}

private :
	T DestType::*m_member;
};

} // end of namespace

#endif
