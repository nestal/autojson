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

#include <typeinfo>

namespace ajs {

Json::Json() : m_type(Type::hash)
{
	m_raw.hash = new Hash;
}

Json::Json(const Json& val) : m_type(val.m_type), m_raw(val.m_raw)
{
	switch (m_type)
	{
	case Type::string:	m_raw.string = new std::string(*val.m_raw.string);	break;
	case Type::array:	m_raw.array	 = new Array(*val.m_raw.array);			break;
	case Type::hash:	m_raw.hash	 = new Hash(*val.m_raw.hash);			break;
	default:	break;
	}
}

Json::Json(Json&& val) : m_type(val.m_type), m_raw(val.m_raw)
{
	switch (m_type)
	{
	case Type::string:	m_raw.string = new std::string(std::move(*val.m_raw.string));	break;
	case Type::array:	m_raw.array	 = new Array(std::move(*val.m_raw.array));			break;
	case Type::hash:	m_raw.hash	 = new Hash(std::move(*val.m_raw.hash));			break;
	default:	break;
	}
}

Json::Json(int val) : m_type(Type::integer)
{
	m_raw.integer = val;
}

Json::Json(long long val) : m_type(Type::integer)
{
	m_raw.integer = val;
}

Json::Json(double val) : m_type(Type::real)
{
	m_raw.real = val;
}

Json::Json(bool val) : m_type(Type::boolean)
{
	m_raw.boolean = val;
}

Json::Json(const std::string& val) : m_type(Type::string)
{
	m_raw.string = new std::string(val);
}

Json::Json(std::string&& val) : m_type(Type::string)
{
	m_raw.string = new std::string(std::move(val));
}

Json::Json(const char *val) : m_type(Type::string)
{
	m_raw.string = new std::string(val);
}

Json::Json(const Array& val) : m_type(Type::array)
{
	m_raw.array = new Array(val);
}

Json::Json(Array&& val) : m_type(Type::array)
{
	m_raw.array = new Array(std::move(val));
}

Json::Json(const Hash& val) : m_type(Type::hash)
{
	m_raw.hash = new Hash(val);
}

Json::Json(Hash&& val) : m_type(Type::hash)
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
	case Type::string:	delete m_raw.string;	break;
	case Type::array:	delete m_raw.array;		break;
	case Type::hash:	delete m_raw.hash;		break;
	default:									break;
	}
}

int Json::AsInt() const
{
	return static_cast<int>(AsLong());
}

long long Json::AsLong() const
{
	switch (m_type)
	{
	case Type::integer:	return m_raw.integer;
	case Type::real:	return static_cast<long long>(m_raw.real);
	case Type::string:	return std::atoi(m_raw.string->c_str());
	default:			throw -1;
	}
}

double Json::AsDouble() const
{
	switch (m_type)
	{
	case Type::integer:	return m_raw.integer;
	case Type::real:	return static_cast<long long>(m_raw.real);
	case Type::string:	return std::atoi(m_raw.string->c_str());
	default:			throw -1;
	}
}

const std::string& Json::AsString() const
{
	if (m_type == Type::string)
		return *m_raw.string;
	else
		throw -1;
}

std::string& Json::AsString()
{
	if (m_type == Type::string)
		return *m_raw.string;
	else
		throw -1;
}

Json::Array& Json::AsArray()
{
	if (m_type == Type::array)
		return *m_raw.array;
	else
		throw -1;
}

const Json::Array& Json::AsArray() const
{
	if (m_type == Type::array)
		return *m_raw.array;
	else
		throw -1;
}

Json::Hash& Json::AsHash()
{
	if (m_type == Type::hash)
		return *m_raw.hash;
	else
		throw -1;
}

const Json::Hash& Json::AsHash() const
{
	if (m_type == Type::hash)
		return *m_raw.hash;
	else
		throw -1;
}

void Json::Add(const Json& val)
{
	AsArray().push_back(val);
}

void Json::Add(const std::string& key, const Json& val)
{
	AsHash().insert(std::make_pair(key, val));
}

Json::Type Json::MyType() const
{
	return m_type;
}

bool Json::Is(Type type) const
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

} // end of namespace
