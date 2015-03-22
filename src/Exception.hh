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

#include <stdexcept>
#include <typeindex>
#include <map>
#include <memory>

namespace json {

class ErrInfoBase
{
public:
	virtual ~ErrInfoBase() = default;
	virtual void Write(std::ostream& os) const = 0;
};

template <typename Tag>
struct ErrInfoTag {};

template <class Tag, typename T>
class ErrInfo : public ErrInfoBase
{
public :
	typedef Tag TagType;
	typedef T   ValueType;
	
	ErrInfo(const T& t) : m_val(t) {}
	void Write(std::ostream& os) const override
	{
//		os << m_val;
	}

	const ValueType& Value() const
	{
		return m_val;
	}
	
private:
	T m_val;
};

class Exception : public std::exception
{
public :
	Exception() = default;
	Exception(const Exception& rhs) = default;

	const char* what() const noexcept override;

	template <typename Tag, typename T>
	void Add(const ErrInfo<Tag,T>& info) const
	{
		m_data.emplace(typeid(ErrInfoTag<Tag>), ErrInfoPtr(new ErrInfo<Tag,T>(info)));
	}

	template <typename TheErrInfo>
	const TheErrInfo* Get() const
	{
		auto i = m_data.find(typeid(ErrInfoTag<typename TheErrInfo::TagType>));
		return i != m_data.end() ? dynamic_cast<TheErrInfo*>(i->second.get()) : nullptr;
	}
	
private :
	typedef std::shared_ptr<ErrInfoBase> ErrInfoPtr;
	typedef std::map<std::type_index, ErrInfoPtr> Map;
	
	mutable Map			m_data;
	mutable std::string	m_what;
};

template <typename Tag, typename T, typename E>
const E& operator<<(const E& expt, const ErrInfo<Tag,T>& info)
{
	expt.Add(info);
	return expt;
}

// line number and column number
using LineNumInfo	= ErrInfo<struct Line, std::size_t>;
using ColumnNumInfo	= ErrInfo<struct Column, std::size_t>;

struct ParseError : public Exception {};

using ExpectedTypeInfo	= ErrInfo<struct ExpectedType, std::type_index>;
using ActualTypeInfo	= ErrInfo<struct ExpectedType, std::type_index>;


/**	Indicates an error caused by type mismatch

	The two attributes are expected and actual types. They are stored as std::type_index.
*/
struct TypeMismatch : public Exception {};

using InvalidCharInfo	= ErrInfo<struct InvalidChar_, char>;

struct InvalidChar : public ParseError {};

} // end of namespace

#endif
