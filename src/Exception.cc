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

#include "Exception.hh"
#include <sstream>

namespace json {

Exception::Exception(const std::string& errmsg ) :
	runtime_error(errmsg)
{
}

ParseError::ParseError(std::size_t line, std::size_t index) :
	Exception("parse error"),
	m_line(line),
	m_index(index)
{
}

std::size_t ParseError::Line() const
{
	return m_line;
}

std::size_t ParseError::Index() const
{
	return m_index;
}

TypeMismatch::TypeMismatch(const std::type_index& expect, const std::type_index& actual) :
	Exception([&]{
		std::ostringstream oss;
		oss << "type mismatch: expect(" << expect.name() << ") "
			<< "actual: (" << actual.name() << ")";
		return oss.str();
	}())
{
}

InvalidChar::InvalidChar(char ch) :
	Exception("invalid character"),
	m_ch(ch)
{
}

char InvalidChar::Get() const
{
	return m_ch;
}

} // end of namespace
