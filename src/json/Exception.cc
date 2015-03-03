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

namespace ajs {

Exception::Exception()
{
}

Exception::Exception(const Exception& rhs)
{
	for (const auto& i:rhs.m_data)
		m_data.emplace(i.first, ErrInfoPtr(i.second->Clone()));
}

const char* Exception::what() const
{
	if (m_what.empty())
	{
		std::ostringstream ss;
		
		for (const auto& i:m_data)
			ss << i.first->name() << i.second.get() ;
	}

	return m_what.c_str();
}

InvalidConversion::InvalidConversion(const JVar& val, const std::string& dest) :
	runtime_error([&]{
		std::ostringstream ss;
		ss << "Cannot convert " << val << " into " << dest ;
		return ss.str();
}())
{
}

} // end of namespace
