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

#ifndef AUTOMATON_HH_INCLUDED
#define AUTOMATON_HH_INCLUDED

#include <string>
#include <memory>
#include <functional>
#include <iosfwd>

namespace json {

enum class DataType
{
	key,
	string,
	number,
	boolean_true,
	boolean_false,
	null_value,
	array,
	object
};

enum class Event
{
	start,
	end,
	data,
} ;

std::ostream& operator<<(std::ostream& os, Event ev);
std::ostream& operator<<(std::ostream& os, DataType ev);

/**	The state machine of the JSON parser.

	The Automaton is the JSON state machine. It takes a stream of characters as input
	and emit events when it encourter certain constructs, such as objects and arrays.
*/
class Automaton
{
public :
	using Callback = std::function<void (Event, DataType, const char *, std::size_t)>;
	
	Automaton(Callback&& callback, std::size_t depth=0);
	~Automaton();
	
	void Parse(const char *str, std::size_t len);
	bool Result() const;
	
private :
	class Impl;
	std::unique_ptr<Impl>	m_impl;
};

} // end of namespace

#endif
