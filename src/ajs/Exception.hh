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

#ifndef EXCEPTION_HH_INCLUDED
#define EXCEPTION_HH_INCLUDED

#include "JVar.hh"
#include <typeinfo>

#include <stdexcept>
#include <memory>

namespace ajs {

class ErrInfoBase
{
public:
	virtual ~ErrInfoBase() {}
	virtual const std::type_info& TypeID() const = 0;
	virtual void Write(std::ostream& os) const = 0;
	virtual ErrInfoBase* Clone() const = 0;
};

template <class Tag, typename T>
class ErrInfo : public ErrInfoBase
{
public :
	ErrInfo(const T& t) : m_val(t) {}
	virtual const std::type_info& TypeID() const override
	{
		return typeid(T);
	}
	virtual ErrInfo* Clone() const override
	{
		return new ErrInfo(m_val) ;
	}
	void Write(std::ostream& os) const override
	{
		os << m_val;
	}

	T m_val;
};

class Exception : public std::exception
{
public :
	Exception();
	Exception(const Exception& rhs);

	const char* what() const override;

	template <class Tag, typename T>
	Exception& Add(const T& t)
	{
		m_data.emplace(&typeid(Tag), ErrInfoPtr(new ErrInfo<Tag, T>(t)));
		return *this;
	}

	template <class Tag>
	Exception& Add(const char *str)
	{
		return Add<Tag>(std::string(str));
	}

private :
	typedef std::unique_ptr<ErrInfoBase> ErrInfoPtr;
	typedef std::map<const std::type_info*, ErrInfoPtr> Map;
	
	Map			m_data;
	std::string	m_what;
};

/**	Brief description of Exception
*/
class InvalidConversion : public std::runtime_error
{
public :
	InvalidConversion(const JVar& val, const std::string& dest);

private :
};

} // end of namespace

#endif
