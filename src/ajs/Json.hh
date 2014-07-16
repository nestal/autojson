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

#ifndef JSON_HH_INCLUDED
#define JSON_HH_INCLUDED

#include "Type.hh"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ajs {

/**	A variant class to define the structure of an expected JSON object
*/
class Json
{
private :
	// helpers
	template <typename T>
	struct CheckEqual
	{
		T val;
		template <typename U>
		bool operator()(U u) { return false; }
		bool operator()(T t) { return t == val; }
	};

public :
	typedef std::vector<Json> 			Array;
	typedef std::map<std::string, Json>	Hash;

public :
	// special member functions
	Json(const Json& rhs);
	Json(Json&& rhs);
	Json& operator=(const Json& rhs);
	Json& operator=(Json&& rhs);
	~Json();
	
	template <typename T>
	Json& operator=(const T& t)
	{
		Json temp(t);
		Swap(temp);
		return *this;
	}
	
	template <typename T>
	Json& operator=(T&& t)
	{
		Json temp(std::move(t));
		Swap(temp);
		return *this;
	}

	// construction from supported types
	Json();
	explicit Json(int val);
	explicit Json(long long val);
	explicit Json(double val);
	explicit Json(bool val);
	explicit Json(const std::string& val);
	explicit Json(std::string&& val);
	explicit Json(const char *val);
	explicit Json(const Array& val);
	explicit Json(Array&& val);
	explicit Json(const Hash& val);
	explicit Json(Hash&& val);

	int Int() const;
	long long Long() const ;
	double Real() const;
	bool Bool() const;
	const std::string& Str() const;
	std::string& Str();
	const Array& AsArray() const;
	Array& AsArray();
	const Hash& AsHash() const;
	Hash& AsHash();
	bool IsNull() const;

	template <typename F>
	auto Apply(F func) const -> decltype(func(1))
	{
		switch (m_type)
		{
		case ajs::Type::integer:	return func(m_raw.integer);
		case ajs::Type::real:		return func(m_raw.real);
		case ajs::Type::boolean:	return func(m_raw.boolean);
		case ajs::Type::string:		return func(*m_raw.string);
		case ajs::Type::array:		return func(*m_raw.array);
		case ajs::Type::hash:		return func(*m_raw.hash);
		default:	throw -1;
		}
	}
	template <typename F>
	auto Apply(F func) -> decltype(func(1))
	{
		switch (m_type)
		{
		case ajs::Type::integer:	return func(m_raw.integer);
		case ajs::Type::real:		return func(m_raw.real);
		case ajs::Type::boolean:	return func(m_raw.boolean);
		case ajs::Type::string:		return func(*m_raw.string);
		case ajs::Type::array:		return func(*m_raw.array);
		case ajs::Type::hash:		return func(*m_raw.hash);
		default:	throw -1;
		}
	}
	
	void Swap(Json& rhs);
	
	// complex types only
	template <typename T>
	Json& Add(const T& val)
	{
		return Add(Json(val));
	}
	template <typename T>
	Json& Add(T&& val)
	{
		return Add(Json(std::move(val)));
	}
	template <typename T>
	Json& Add(const std::string& key, const T& val)
	{
		return Add(key, Json(val));
	}
	template <typename T>
	Json& Add(const std::string& key, T&& val)
	{
		return Add(key, Json(std::move(val)));
	}
	Json& Add(const Json& val);
	Json& Add(Json&& val);
	Json& Add(const std::string& key, const Json& val);
	Json& Add(const std::string& key, Json&& val);
	
	void Clear();
	const Json& operator[](const std::string& key) const;
	const Json& operator[](std::size_t idx) const;

	template <typename T> bool Is() const
	{
		return m_type == TypeMap<T>::type;
	}
	template <typename T> bool Equal(const T& v) const
	{
		return Apply(CheckEqual<typename TypeMap<T>::UnderlyingType>{v});
	}

	ajs::Type Type() const;
	bool Is(ajs::Type type) const;

private :
	union Raw
	{
		long long	integer;
		double		real;
		bool		boolean;
		std::string	*string;
		Array		*array;
		Hash		*hash;
	};

	ajs::Type	m_type;
	Raw 		m_raw;
};

template <typename T> bool operator==(const T& v, const Json& js)
{
	return js.Equal(v);
}

template <typename T> bool operator==(const Json& js, const T& v)
{
	return js.Equal(v);
}

} // end of namespace

#endif
