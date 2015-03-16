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

enum class Event
{
	array_start,	//!< Start of a JSON array
	array_end,		//!< End of a JSON array
	object_start,	//!< Start of a JSON object
	object_key,		//!< A key inside a JSON object.
					//!< The value of the key is given by \c data with
					//!< \c len bytes.
	object_end,		//!< End of a JSON object
	string_start,	//!< Start of a string. Does not have any data
	string_data,	//!< String as a value. The value is given by \c data
	string_end,		//!< End of a string. Does not have any data
					//!< and \c len.
	number,			//!< Number as a value. It's provided as a string.
					//!< You need to call atoi() to obtain the real number.
	null_,			//!< Literal value "null"
	true_,			//!< Literal value "true"
	false_			//!< Literal value "false"
} ;

std::ostream& operator<<(std::ostream& os, Event ev);

/**	Brief description of Automaton
*/
class Automaton
{
public :
	using Callback = std::function<void (Event, const char *, std::size_t)>;
	
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
