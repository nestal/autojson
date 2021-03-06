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
#include <string>
#include <cxxabi.h>

namespace json {

const char* Exception::what() const noexcept
{
	std::ostringstream ss;
	for (const auto& p : m_data)
	{
		int status;
		ss << abi::__cxa_demangle(p.first.name(), 0, 0, &status) << ": ";
		p.second->Write(ss);
		ss << std::endl;
	}
	m_what = ss.str();
	return m_what.c_str();
}

} // end of namespace
