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

ParseError::ParseError() : Exception("parse error")
{
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

} // end of namespace