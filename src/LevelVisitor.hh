/*
	autojson: A JSON parser base on the automaton provided by json.org
	Copyright (C) 2015  Wan Wai Ho

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

#ifndef REACTOR_HH_INCLUDED
#define REACTOR_HH_INCLUDED

#include "JSON_checker.h"
#include "Key.hh"
#include "Level.hh"

#include "LexicalCast.hh"

#include <cassert>
#include <type_traits>

namespace json {

class LevelVisitor
{
public:
	virtual ~LevelVisitor() = default;
	virtual void Data(const Level& current, JSON_event type, const char *data, size_t len) const = 0;
	virtual Level Advance(const Level& current) const = 0;
	virtual void Finish(const Level& current) const = 0;
};

class MockObjectHandler : public LevelVisitor
{
public:
	void Data(const Level&, JSON_event, const char*, size_t) const override {}
	Level Advance(const Level& current) const override
	{
		assert(current.Rec() == this);
		return Level{current.Key()};
	}
	
	void Finish(const Level& current) const override
	{
		assert(current.Rec() == this);
	}
	
	static MockObjectHandler* Instance()
	{
		static MockObjectHandler instance;
		return &instance;
	}
};

template <typename T>
class SimpleTypeBuilder : public LevelVisitor
{
public :
	SimpleTypeBuilder() = default;
	SimpleTypeBuilder(const SimpleTypeBuilder&) = default;
	~SimpleTypeBuilder() override = default;

#ifndef _MSC_VER
	SimpleTypeBuilder(SimpleTypeBuilder&&) = default;
#endif

	void Data(const Level& current, JSON_event, const char *data, size_t len) const override
	{
		assert(current.Rec() == this);
		*static_cast<T*>(current.Host()) = lexical_cast<T>(data, len);
	}
	
	Level Advance(const Level& current) const override
	{
		assert(current.Rec() == this);
		return Level{current.Key()};
	}
	
	void Finish(const Level& current) const override
	{
		assert(current.Rec() == this);
	}
};

template <typename Host>
class ComplexTypeBuilder : public LevelVisitor
{
public:
	static Host* Self(const Level& current)
	{
		assert(current.Host());
		return static_cast<Host*>(current.Host());
	}
};

/*!	Builds a member of a class with the given builder.

	The MemberBuilder builds a member of a class.
	\param	Host	The class that host the member to be built.
	\param	T		The type of the member to be built.
	\param	Builder	The builder that build the member. It can be a ComplexTypeBuilder
					or SimpleTypeBuilder.
	
*/
template <typename Host, typename T, typename Builder>
class MemberBuilder : public ComplexTypeBuilder<Host>
{
public:
	MemberBuilder(const Builder& rec, T Host::*m ) :
		m_rec(rec),
		m_mem(m)
	{
	}
	
	MemberBuilder(const MemberBuilder&) = default;
	
	Level Advance(const Level& current) const override
	{
		return Level{current.Key(), &(this->Self(current)->*m_mem), &m_rec};
	}
	
	void Data(const Level& current, JSON_event type, const char *data, size_t len) const override
	{
		m_rec.Data(
			Level{current.Key(), &(this->Self(current)->*m_mem), &m_rec},
			type, data, len);
	}
	
	void Finish(const Level& current) const override
	{
		assert(current.Rec() == this);
	}
	
private:
	Builder		m_rec;
	T Host::*	m_mem;
};

template <typename Host, typename T, typename R=void>
class MemberCaller : public ComplexTypeBuilder<Host>
{
public:
	using Func = T (Host::*)(T);
};

} // end of namespace

#endif
