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

#ifndef LEXICALCAST_HH_INCLUDED
#define LEXICALCAST_HH_INCLUDED

#include <string>

namespace json {

template <typename Dest>
Dest lexical_cast(const char *str, std::size_t len);

template <>
int lexical_cast(const char *str, std::size_t len);

template <>
long long lexical_cast(const char *str, std::size_t len);

template <>
unsigned long long lexical_cast(const char *str, std::size_t len);

template <>
unsigned long lexical_cast(const char *str, std::size_t len);

template <>
double lexical_cast(const char *str, std::size_t len);

template <>
bool lexical_cast(const char *str, std::size_t len);

template <>
std::string lexical_cast(const char *str, std::size_t len);

} // end of namespace

#endif
