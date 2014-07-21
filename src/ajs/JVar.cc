/*
	autoJVar: A JVar parser base on the automaton provided by JVar.org
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

#include "JVar.hh"

#include <cassert>
#include <typeinfo>

namespace ajs {

JVar::JVar() : m_type(ajs::Type::null), m_raw({})
{
}

JVar::JVar(const JVar& val) : m_type(val.m_type), m_raw(val.m_raw)
{
	// deep copy
	switch (m_type)
	{
	case ajs::Type::string:	m_raw.string = new std::string(*val.m_raw.string);	break;
	case ajs::Type::array:	m_raw.array	 = new Array(*val.m_raw.array);			break;
	case ajs::Type::hash:	m_raw.hash	 = new Hash(*val.m_raw.hash);			break;
	default:	break;
	}
}

JVar::JVar(JVar&& val) : m_type(val.m_type), m_raw(val.m_raw)
{
	val.m_type	= ajs::Type::null;
	val.m_raw	= {};
}

JVar::JVar(int val) : m_type(ajs::Type::integer)
{
	m_raw.integer = val;
}

JVar::JVar(long long val) : m_type(ajs::Type::integer)
{
	m_raw.integer = val;
}

JVar::JVar(double val) : m_type(ajs::Type::real)
{
	m_raw.real = val;
}

JVar::JVar(bool val) : m_type(ajs::Type::boolean)
{
	m_raw.boolean = val;
}

JVar::JVar(const std::string& val) : m_type(ajs::Type::string)
{
	m_raw.string = new std::string(val);
}

JVar::JVar(std::string&& val) : m_type(ajs::Type::string)
{
	m_raw.string = new std::string(std::move(val));
}

JVar::JVar(const char *val) : m_type(ajs::Type::string)
{
	m_raw.string = new std::string(val);
}

JVar::JVar(const Array& val) : m_type(ajs::Type::array)
{
	m_raw.array = new Array(val);
}

JVar::JVar(Array&& val) : m_type(ajs::Type::array)
{
	m_raw.array = new Array(std::move(val));
}

JVar::JVar(const Hash& val) : m_type(ajs::Type::hash)
{
	m_raw.hash = new Hash(val);
}

JVar::JVar(Hash&& val) : m_type(ajs::Type::hash)
{
	m_raw.hash = new Hash(std::move(val));
}

void JVar::Swap(JVar& rhs)
{
	std::swap(m_type, rhs.m_type);
	std::swap(m_raw,  rhs.m_raw);
}

JVar::~JVar()
{
	switch (m_type)
	{
	case ajs::Type::string:	delete m_raw.string;	break;
	case ajs::Type::array:	delete m_raw.array;		break;
	case ajs::Type::hash:	delete m_raw.hash;		break;
	default:									break;
	}
}

JVar& JVar::AddByKey(const std::string& key)
{
	if (m_type == ajs::Type::null)
		*this = Hash();

	return AsHash().emplace(key, JVar()).first->second;
}

JVar& JVar::Add(const JVar& val)
{
	return Add(std::move(JVar(val)));
}

JVar& JVar::Add(JVar&& val)
{
	if (m_type == ajs::Type::null)
		*this = Array();

	AsArray().push_back(std::move(val));
	return *this;
}

JVar& JVar::Add(const std::string& key, const JVar& val)
{
	return Add(key, std::move(JVar(val)));
}

JVar& JVar::Add(const std::string& key, JVar&& val)
{
	if (m_type == ajs::Type::null)
		*this = Hash();
	
	AsHash().insert(std::make_pair(key, std::move(val)));
	return *this;
}

ajs::Type JVar::Type() const
{
	return m_type;
}

bool JVar::Is(ajs::Type type) const
{
	return m_type == type;
}

const JVar& JVar::operator[](const std::string& key) const
{
	auto it = AsHash().find(key);
	if (it == AsHash().end())
		throw -1;
	
	return it->second;
}

const JVar& JVar::operator[](std::size_t idx) const
{
	return AsArray().at(idx);
}

struct JVar::ClearComposite
{
	template <typename U> void operator()(U)	{}
	void operator()(JVar::Hash& hash)			{hash.clear();}
	void operator()(JVar::Array& array)			{array.clear();}
};

void JVar::Clear()
{
	Apply(ClearComposite());
}

struct JVar::SizeOf
{
	std::size_t size ;
	template <typename U> void operator()(U)	{size = 0;}
	void operator()(const JVar::Hash& hash)		{size = hash.size();}
	void operator()(const JVar::Array& array)	{size = array.size();}
};

std::size_t JVar::Size() const
{
	return Apply(SizeOf{}).size;
}

struct JVar::Printer
{
	std::ostream& os;
	explicit Printer(std::ostream& outs) : os(outs) {}

	template <typename T>
	void operator()(T i)
	{
		os << i;
	}

	void operator()(const std::string& s)
	{
		// will escape later
		os << '\"' << s << '\"' ;
	}

	void operator()(bool b)
	{
		os << (b ? "true" : "false");
	}

	void operator()(const Hash& hash)
	{
		bool first = true;
		os << "{";
		for (const auto& e : hash)
		{
			os << (!first ? ", " : "") << e.first << ": " << e.second ;
			first = false;
		}
		os << '}';
	}
	
	void operator()(const Array& array)
	{
		bool first = true;
		os << '[';
		for (const auto& e : array)
		{
			os << (!first ? ", " : "") << e ;
			first = false;
		}
		os << ']';
	}
};

std::ostream& JVar::Print(std::ostream& os) const
{
	Apply(Printer(os));
	return os;
}

std::ostream& operator<<(std::ostream& os, const JVar& jv)
{
	return jv.Print(os);
}

} // end of namespace
