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

#ifndef TYPE_HH_INCLUDED
#define TYPE_HH_INCLUDED

#include <map>
#include <string>
#include <type_traits>
#include <vector>

namespace ajs {

class Json;

enum class Type { null, integer, real, boolean, string, array, hash };

// mapping from a C++ type to its corresponding JSON type
template <typename T, typename=void> struct TypeMap;

template <typename T>
struct TypeMap<T, typename std::enable_if<std::is_integral<T>::value>::type>
{
	static const Type type = Type::integer;
	typedef long long UnderlyingType ;
};

template <typename T>
struct TypeMap<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
{
	static const Type type = Type::real;
	typedef double UnderlyingType ;
};

template <>
struct TypeMap<std::string>
{
	static const Type type = Type::string;
	typedef std::string UnderlyingType ;
};

template <>
struct TypeMap<const char*>
{
	static const Type type = Type::string;
	typedef std::string UnderlyingType ;
};

template <std::size_t n>
struct TypeMap<char[n]>
{
	static const Type type = Type::string;
	typedef std::string UnderlyingType ;
};

template <typename T>
struct TypeMap<std::vector<T>>
{
	static const Type type = Type::array;
	typedef std::vector<Json> UnderlyingType ;
};

template <>
struct TypeMap<std::map<std::string, Json>>
{
	static const Type type = Type::hash;
	typedef std::map<std::string, Json> UnderlyingType ;
};

} // end of namespace

#endif
