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

#ifndef JSONPARSER_HH_INCLUDED
#define JSONPARSER_HH_INCLUDED

#include "Reactor.hh"
#include <memory>
#include <vector>

namespace ajs {

/**	The main parser class.
	JsonParser is the class you use to parse JSON data. First, construct a JsonParser
	object by providing a Reactor and an object that receives the output.
*/
class JsonParser
{
public :
	template <typename R, typename DestType>
	JsonParser(const R& reactor, DestType& t) :
		m_reactor(new R(reactor)),
		m_parser(new_JSON_checker(5))
	{
		ParseState p {m_reactor.get(), &t};
		m_stack.push_back(p);
	}

	
	void Parse(const char *json, std::size_t len);

private :
	static void Callback(void *user, JSON_event type, const char *data, size_t len);

private :
	std::unique_ptr<Reactor>	m_reactor;
	std::vector<ParseState>		m_stack;
	JSON_checker				m_parser;
};

} // end of namespace

#endif
