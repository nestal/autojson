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
#include <cstdint>

namespace json {

template <typename Dest>
Dest LexicalCast(const char *str, std::size_t len);

template <>
std::int32_t LexicalCast(const char *str, std::size_t len);

template <>
std::int64_t LexicalCast(const char *str, std::size_t len);

template <>
std::uint32_t LexicalCast(const char *str, std::size_t len);

template <>
std::uint64_t LexicalCast(const char *str, std::size_t len);

template <>
double LexicalCast(const char *str, std::size_t len);

template <>
bool LexicalCast(const char *str, std::size_t len);

template <>
std::string LexicalCast(const char *str, std::size_t len);

std::string Unescape(const char *str, std::size_t len);
std::string Unescape(const std::string& str);

} // end of namespace

#endif
