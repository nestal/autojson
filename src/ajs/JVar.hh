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

#ifndef JVAR_HH_INCLUDED
#define JVAR_HH_INCLUDED

#include "Type.hh"
#include "Exception.hh"

#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ajs {

/**	A variant class to define the structure of an expected JSON object
*/
class JVar
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
		void operator()(U)			{ equal = false; }
		void operator()(const T& t)	{ equal = (t == val); }
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
	
	struct ClearComposite;
	struct Printer;
	struct SizeOf;

public :
	typedef std::vector<JVar> 			Array;
	typedef std::map<std::string, JVar>	Hash;

public :
	// special member functions
	JVar(const JVar& rhs);
	JVar(JVar&& rhs);
	JVar& operator=(const JVar& rhs);
	JVar& operator=(JVar&& rhs);
	~JVar();
	
	template <typename T>
	JVar& operator=(const T& t)
	{
		JVar temp(t);
		Swap(temp);
		return *this;
	}
	
	template <typename T>
	JVar& operator=(T&& t)
	{
		JVar temp(std::move(t));
		Swap(temp);
		return *this;
	}

	// construction from supported types
	JVar();
	explicit JVar(int val);
	explicit JVar(long long val);
	explicit JVar(double val);
	explicit JVar(bool val);
	explicit JVar(const std::string& val);
	explicit JVar(std::string&& val);
	explicit JVar(const char *val);
	explicit JVar(const Array& val);
	explicit JVar(Array&& val);
	explicit JVar(const Hash& val);
	explicit JVar(Hash&& val);
	template <typename T>
	explicit JVar(const std::vector<T>& vec)
	{
		for (const auto& i : vec)
			m_raw.array->emplace_back(i);
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
	
	void Swap(JVar& rhs);
	
	// complex types only
	template <typename T>
	JVar& Add(const T& val)
	{
		return Add(JVar(val));
	}
	template <typename T>
	JVar& Add(T&& val)
	{
		return Add(JVar(std::move(val)));
	}
	template <typename T>
	JVar& Add(const std::string& key, const T& val)
	{
		return Add(key, JVar(val));
	}
	template <typename T>
	JVar& Add(const std::string& key, T&& val)
	{
		return Add(key, JVar(std::move(val)));
	}
	JVar& Add(const JVar& val);
	JVar& Add(JVar&& val);
	JVar& Add(const std::string& key, const JVar& val);
	JVar& Add(const std::string& key, JVar&& val);
	
	void Clear();
	const JVar& operator[](const std::string& key) const;
	const JVar& operator[](std::size_t idx) const;

	template <typename T> const typename TypeMap<T>::UnderlyingType& As() const
	{
		auto func = Apply(GetVal<const typename TypeMap<T>::UnderlyingType>{});
		if (func.val == nullptr)
			throw -1;

		return *func.val;
	}
	template <typename T> typename TypeMap<T>::UnderlyingType& As()
	{
		typedef typename TypeMap<T>::UnderlyingType TargetType;
		auto func = Apply(GetVal<TargetType>{});
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

	std::ostream& Print(std::ostream& os) const;

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

template <typename T> bool operator==(const T& v, const JVar& js)
{
	return js.Equal(v);
}

template <typename T> bool operator==(const JVar& js, const T& v)
{
	return js.Equal(v);
}

std::ostream& operator<<(std::ostream& os, const JVar& jv);

} // end of namespace

#endif
