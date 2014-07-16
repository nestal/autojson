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

#include "Json.hh"

#include <cassert>
#include <typeinfo>

namespace ajs {

Json::Json() : m_type(ajs::Type::hash)
{
	m_raw.hash = new Hash;
}


Json::Json(const Json& val) : m_type(val.m_type), m_raw(val.m_raw)
{
	switch (m_type)
	{
	case ajs::Type::string:	m_raw.string = new std::string(*val.m_raw.string);	break;
	case ajs::Type::array:	m_raw.array	 = new Array(*val.m_raw.array);			break;
	case ajs::Type::hash:	m_raw.hash	 = new Hash(*val.m_raw.hash);			break;
	default:	break;
	}
}

Json::Json(Json&& val) : m_type(val.m_type), m_raw(val.m_raw)
{
	switch (m_type)
	{
	case ajs::Type::string:	m_raw.string = new std::string(std::move(*val.m_raw.string));	break;
	case ajs::Type::array:	m_raw.array	 = new Array(std::move(*val.m_raw.array));			break;
	case ajs::Type::hash:	m_raw.hash	 = new Hash(std::move(*val.m_raw.hash));			break;
	default:	break;
	}
}

Json::Json(int val) : m_type(ajs::Type::integer)
{
	m_raw.integer = val;
}

Json::Json(long long val) : m_type(ajs::Type::integer)
{
	m_raw.integer = val;
}

Json::Json(double val) : m_type(ajs::Type::real)
{
	m_raw.real = val;
}

Json::Json(bool val) : m_type(ajs::Type::boolean)
{
	m_raw.boolean = val;
}

Json::Json(const std::string& val) : m_type(ajs::Type::string)
{
	m_raw.string = new std::string(val);
}

Json::Json(std::string&& val) : m_type(ajs::Type::string)
{
	m_raw.string = new std::string(std::move(val));
}

Json::Json(const char *val) : m_type(ajs::Type::string)
{
	m_raw.string = new std::string(val);
}

Json::Json(const Array& val) : m_type(ajs::Type::array)
{
	m_raw.array = new Array(val);
}

Json::Json(Array&& val) : m_type(ajs::Type::array)
{
	m_raw.array = new Array(std::move(val));
}

Json::Json(const Hash& val) : m_type(ajs::Type::hash)
{
	m_raw.hash = new Hash(val);
}

Json::Json(Hash&& val) : m_type(ajs::Type::hash)
{
	m_raw.hash = new Hash(std::move(val));
}

void Json::Swap(Json& rhs)
{
	std::swap(m_type, rhs.m_type);
	std::swap(m_raw,  rhs.m_raw);
}

Json::~Json()
{
	switch (m_type)
	{
	case ajs::Type::string:	delete m_raw.string;	break;
	case ajs::Type::array:	delete m_raw.array;		break;
	case ajs::Type::hash:	delete m_raw.hash;		break;
	default:									break;
	}
}

int Json::Int() const
{
	return static_cast<int>(Long());
}

long long Json::Long() const
{
	switch (m_type)
	{
	case ajs::Type::integer:	return m_raw.integer;
	case ajs::Type::real:	return static_cast<long long>(m_raw.real);
	case ajs::Type::string:	return std::atoi(m_raw.string->c_str());
	default:			throw -1;
	}
}

double Json::Real() const
{
	switch (m_type)
	{
	case ajs::Type::integer:	return m_raw.integer;
	case ajs::Type::real:	return static_cast<long long>(m_raw.real);
	case ajs::Type::string:	return std::atoi(m_raw.string->c_str());
	default:			throw -1;
	}
}

const std::string& Json::Str() const
{
	if (m_type != ajs::Type::string)
		throw -1;
	
	assert(m_raw.string != nullptr);
	return *m_raw.string;
}

std::string& Json::Str()
{
	if (m_type != ajs::Type::string)
		throw -1;
	
	assert(m_raw.string != nullptr);
	return *m_raw.string;
}

Json::Array& Json::AsArray()
{
	if (m_type != ajs::Type::array)
		throw -1;
	
	assert(m_raw.array != nullptr);
	return *m_raw.array;
}

const Json::Array& Json::AsArray() const
{
	if (m_type != ajs::Type::array)
		throw -1;
	
	assert(m_raw.array != nullptr);
	return *m_raw.array;
}

Json::Hash& Json::AsHash()
{
	if (m_type != ajs::Type::hash)
		throw -1;
	
	assert( m_raw.hash != nullptr);
	return *m_raw.hash;
}

const Json::Hash& Json::AsHash() const
{
	if (m_type != ajs::Type::hash)
		throw -1;
	
	assert( m_raw.hash != nullptr);
	return *m_raw.hash;
}

Json& Json::Add(const Json& val)
{
	AsArray().push_back(val);
	return *this;
}

Json& Json::Add(Json&& val)
{
	AsArray().push_back(std::move(val));
	return *this;
}

Json& Json::Add(const std::string& key, const Json& val)
{
	AsHash().insert(std::make_pair(key, val));
	return *this;
}

Json& Json::Add(const std::string& key, Json&& val)
{
	AsHash().insert(std::make_pair(key, std::move(val)));
	return *this;
}

ajs::Type Json::Type() const
{
	return m_type;
}

bool Json::Is(ajs::Type type) const
{
	return m_type == type;
}

const Json& Json::operator[](const std::string& key) const
{
	auto it = AsHash().find(key);
	if (it == AsHash().end())
		throw -1;
	
	return it->second;
}

const Json& Json::operator[](std::size_t idx) const
{
	return AsArray().at(idx);
}

} // end of namespace
