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

#ifndef EXCEPTION_HH_INCLUDED
#define EXCEPTION_HH_INCLUDED

#include <stdexcept>
#include <typeindex>

namespace json {

class Exception : public std::runtime_error
{
public :
	Exception(const std::string& errmsg) ;
};

class ParseError : public Exception
{
public:
	ParseError(std::size_t line, std::size_t column) ;

	std::size_t Line() const;
	std::size_t Column() const;
	
private:
	std::size_t	m_line;
	std::size_t m_column;
};

/**	Indicates an error caused by type mismatch

	The two attributes are expected and actual types. They are stored as std::type_index.
*/
struct TypeMismatch : public Exception
{
	TypeMismatch(const std::type_index& expect, const std::type_index& actual);
};

class InvalidChar : public Exception
{
public:
	InvalidChar(char ch);
	
	char Get() const;
	
private:
	char m_ch;
};

} // end of namespace

#endif
