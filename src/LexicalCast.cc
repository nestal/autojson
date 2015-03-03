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

#include "LexicalCast.hh"

#include <cstdlib>

namespace json {

template <>
int lexical_cast(const char *str, std::size_t len)
{
	return str != nullptr ? std::atoi(std::string(str, len).c_str()) : 0;
}

template <>
long long lexical_cast(const char *str, std::size_t len)
{
	return str != nullptr ? std::atoll(std::string(str, len).c_str()) : 0LL;
}

template <>
unsigned long lexical_cast(const char *str, std::size_t len)
{
	return str != nullptr ? std::strtoll(std::string(str, len).c_str(), nullptr, 10) : 0ULL;
}

template <>
unsigned long long lexical_cast(const char *str, std::size_t len)
{
	return str != nullptr ? std::strtoull(std::string(str, len).c_str(), nullptr, 10) : 0ULL;
}

template <>
double lexical_cast(const char *str, std::size_t len)
{
	return str != nullptr ? std::atof(std::string(str, len).c_str()) : 0.0;
}

template <>
std::string lexical_cast(const char *str, std::size_t len)
{
	return str != nullptr ? std::string(str, len) : "";
}

template <>
bool lexical_cast(const char *str, std::size_t len)
{
	return str != nullptr && std::string(str, len) == "true" ;
}

	
} // end of namespace
