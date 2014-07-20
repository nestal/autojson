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

#include <cassert>
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
		explicit CheckEqual(const T& r) : val(r), equal(false) {}

		const T& val;
		bool equal;
		template <typename U>
		void operator()(U)		{ equal = false; }
		void operator()(T t)	{ equal = (t == val); }
	};
	
	template <typename T>
	struct GetVal
	{
		explicit GetVal(T *v = nullptr) : val(v) {}
		T *val;
		template <typename U>
		void operator()(U)		{ val = nullptr; }
		void operator()(T& t)	{ val = &t; }
	};
	
	struct Destroyer;
	struct Printer;
	struct SizeOf;

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
	template <typename T>
	explicit Json(const std::vector<T>& vec) : Json((Array()))
	{
		for (const auto& i : vec)
			m_raw.array->push_back(Json(i));
	}

	// shortcuts for As()
	int Int() const 				{return static_cast<int>(As<int>());}
	long long Long() const			{return As<long long>();}
	double Real() const				{return As<double>();}
	bool Bool() const				{return As<bool>();}
	const std::string& Str() const	{return As<std::string>();}
	std::string& Str()				{return As<std::string>();}
	const Array& AsArray() const	{return As<Array>();}
	Array& AsArray()				{return As<Array>();}
	const Hash& AsHash() const		{return As<Hash>();}
	Hash& AsHash()					{return As<Hash>();}
	bool IsNull() const				{return Is<void>();}
	std::size_t Size() const;

	template <typename Visitor>
	Visitor Apply(Visitor func) const
	{
		switch (m_type)
		{
		case ajs::Type::integer:	func(*static_cast<const long long*>(&m_raw.integer));	break;
		case ajs::Type::real:		func(*static_cast<const double*>(&m_raw.real));			break;
		case ajs::Type::boolean:	func(*static_cast<const bool*>(&m_raw.boolean));		break;
		case ajs::Type::string:		func(*static_cast<const std::string*>(m_raw.string));	break;
		case ajs::Type::array:		func(*static_cast<const Array*>(m_raw.array));			break;
		case ajs::Type::hash:		func(*static_cast<const Hash*>(m_raw.hash));			break;
		default:	break;
		}
		return func;
	}
	template <typename Visitor>
	Visitor Apply(Visitor func)
	{
		switch (m_type)
		{
		case ajs::Type::integer:	func(m_raw.integer);	break;
		case ajs::Type::real:		func(m_raw.real);		break;
		case ajs::Type::boolean:	func(m_raw.boolean);	break;
		case ajs::Type::string:		func(*m_raw.string);	break;
		case ajs::Type::array:		func(*m_raw.array);		break;
		case ajs::Type::hash:		func(*m_raw.hash);		break;
		default:	break;
		}
		return func;
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

	template <typename T> const typename TypeMap<T>::UnderlyingType& As() const
	{
		auto func = Apply(GetVal<const typename TypeMap<T>::UnderlyingType>{});
		if (func.val == nullptr)
			throw -1;

		return *func.val;
	}
	template <typename T> typename TypeMap<T>::UnderlyingType& As()
	{
		auto func = Apply(GetVal<typename TypeMap<T>::UnderlyingType>{});
		if (func.val == nullptr)
			throw -1;
		
		return *func.val;
	}
	template <typename T> bool Is() const
	{
		return m_type == TypeMap<T>::type;
	}
	template <typename T> bool Equal(const T& v) const
	{
		auto func = Apply(CheckEqual<typename TypeMap<T>::UnderlyingType>(v));
		return func.equal;
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
