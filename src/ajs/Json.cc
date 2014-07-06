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

struct Json::Base
{
	virtual ~Base() {}
	virtual Base* Clone() const = 0;
	virtual const std::type_info& Type() const = 0;
};

template <>
struct Json::Var<void> : public Json::Base
{
	Var* Clone() const override {return new Var;}
	const std::type_info& Type() const override {return typeid(void);}
};

template <typename T>
struct Json::Var : public Json::Base
{
	T var;
	Var(const T& t) : var(t) {}
	Var* Clone() const override
	{
		return new Var(*this);
	}
	const std::type_info& Type() const override
	{
		return typeid(T);
	}
};

Json::Json(const Json& rhs) : m_base(rhs.m_base->Clone())
{
}

Json::Json(Json&& rhs)
{
	Swap(rhs);
}

Json::~Json()
{
}

void Json::Swap(Json& rhs)
{
	std::swap(m_base, rhs.m_base);
}

Json& Json::operator=(const Json& rhs)
{
	return operator=(std::move(Json(rhs)));
}

Json& Json::operator=(Json&& rhs)
{
	Swap(rhs);
	return *this;
}

Json::Json() : m_base(new Var<void>)
{
}

Json::Json(int val) : Json(static_cast<long long>(val))
{
}

Json::Json(long long val) : m_base(new Var<long long>(val))
{
}

Json::Json(double val) : m_base(new Var<double>(val))
{
}

Json::Json(bool val) : m_base(new Var<bool>(val))
{
}

Json::Json(const std::string& val) : m_base(new Var<std::string>(val))
{
}

Json::Json(const char *val) : Json(std::string(val))
{
}

Json::Json(const Array& val) : m_base(new Var<Array>(val))
{
}

Json::Json(const Hash& val) : m_base(new Var<Hash>(val))
{
}

template <typename T>
const T& Json::As() const
{
	return dynamic_cast<const Var<T>*>(m_base.get())->var;
}

int			Json::AsInt()		const {return static_cast<int>(As<long long>());}
long long	Json::AsLong()		const {return static_cast<int>(As<long long>());}
double		Json::AsDouble()	const {return As<double>();}
bool		Json::AsBool()		const {return As<bool>();}

const std::string&	Json::AsString()	const {return As<std::string>();}
const Json::Array&	Json::AsArray()		const {return As<Array>();}
const Json::Hash&	Json::AsHash()		const {return As<Hash>();}
Json::Array&	Json::AsArray()	{return Cast<Array>()->var;}
Json::Hash&		Json::AsHash()	{return Cast<Hash>()->var;}

template <typename T>
Json::Var<T>* Json::Cast()
{
	return dynamic_cast<Var<T>*>(m_base.get());
}

template <typename T>
const Json::Var<T>* Json::Cast() const
{
	return dynamic_cast<const Var<T>*>(m_base.get());
}

template <typename T>
void Json::Assign(const T& val)
{
	Cast<T>()->var = val;
}

template void Json::Assign<long long>(const long long& val);
template void Json::Assign<double>(const double& val);
template void Json::Assign<bool>(const bool& val);
template void Json::Assign<std::string>(const std::string& val);
template void Json::Assign<Json::Array>(const Array& val);
template void Json::Assign<Json::Hash>(const Hash& val);

// complex types only
void Json::Append(const Json& val)
{
	Cast<Array>()->var.push_back(val);
}

void Json::Insert(const std::string& key, const Json& val)
{
	Cast<Hash>()->var.insert(std::make_pair(key, val));
}

void Json::Clear()
{
	if (m_base->Type() == typeid(Array))
		Cast<Array>()->var.clear();
	else if (m_base->Type() == typeid(Hash))
		Cast<Hash>()->var.clear();
}

template <typename T>
bool Json::Is() const
{
	return m_base->Type() == typeid(T);
}

template bool Json::Is<long long>() const;
template bool Json::Is<double>() const;
template bool Json::Is<bool>() const;
template bool Json::Is<std::string>() const;
template bool Json::Is<Json::Array>() const;
template bool Json::Is<Json::Hash>() const;

const Json& Json::operator[](const std::string& key) const
{
	static const Json null;

	auto it = AsHash().find(key);
	return it != AsHash().end() ? it->second : null;
}

} // end of namespace
