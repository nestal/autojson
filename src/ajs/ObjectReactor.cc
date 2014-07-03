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

namespace ajs {

void ReactorCallback(void *user, JSON_event type, const char *data, int len)
{
	std::vector<ParseState> *state =
		reinterpret_cast<std::vector<ParseState>*>(user);

	ParseState p = state->back().handler->On(state->back(), type, data, len);
	if (p.handler != state->back().handler)
		state->push_back(p) ;
	else if (p.handler == nullptr)
		state->pop_back();
}

} // end of namespace
