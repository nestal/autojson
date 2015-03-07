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

#include "LexicalCast.hh"

#include <cassert>
#include <functional>
#include <initializer_list>
#include <map>
#include <memory>
#include <type_traits>

namespace json {

//struct KeyNotFound : virtual expt::Exception {};
//typedef boost::error_info<struct KeyTag, std::string> Key_ ;

class LevelVisitor;
struct Level
{
	Key					key;
	void				*obj;	//!< The object being built by JSON
	const LevelVisitor	*rec;	//!< The Reactor that builds the members of the object
};


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
		assert(current.rec == this);
		return Level{current.key, nullptr, Instance()};
	}
	
	void Finish(const Level& current) const override
	{
		assert(current.rec == this);
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
		assert(current.rec == this);
		*static_cast<T*>(current.obj) = lexical_cast<T>(data, len);
	}
	
	Level Advance(const Level& current) const override
	{
		assert(current.rec == this);
		return Level{current.key, nullptr, MockObjectHandler::Instance()};
	}
	
	void Finish(const Level& current) const override
	{
		assert(current.rec == this);
	}
};

template <typename Host>
class ComplexTypeBuilder : public LevelVisitor
{
public:
/*	virtual void OnData(const Key& key, JSON_event, const char *data, size_t len, Host *host) const = 0 ;
	virtual Level OnAdvance(const Key& key, Host *host) const = 0;
	virtual void Finish(const Level& level, Host *host ) const = 0;

	void Data(const Level& current, JSON_event ev, const char *data, size_t len) const override
	{
		assert(current.rec == this);
		OnData(current.key, ev, data, len, static_cast<Host*>(current.obj));
	}
	
	Level Advance(const Level& current) const override
	{
		assert(current.rec == this);
		return this->OnAdvance(current.key, static_cast<Host*>(current.obj));
	}
	
	void Finish(const Level& current) const override
	{
		this->Finish(current, static_cast<Host*>(current.obj));
	}*/

	static Host* Self(const Level& current)
	{
		assert(current.obj);
		return static_cast<Host*>(current.obj);
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
		return Level{current.key, &(this->Self(current)->*m_mem), &m_rec};
	}
	
	void Data(const Level& current, JSON_event type, const char *data, size_t len) const override
	{
		m_rec.Data(
			Level{current.key, &(this->Self(current)->*m_mem), &m_rec},
			type, data, len);
	}
	
	void Finish(const Level& current) const override
	{
		assert(current.rec == this);
	}
	
private:
	Builder		m_rec;
	T Host::*	m_mem;
};

template <typename Host>
class JsonBuilder : public ComplexTypeBuilder<Host>
{
public :
	JsonBuilder() = default;
	JsonBuilder(const JsonBuilder&) = default;
#ifndef _MSC_VER
	JsonBuilder(JsonBuilder&&) = default;
#endif
	~JsonBuilder() override = default;
	
	template <typename T>
	JsonBuilder(const Key& key, T Host::*mem)
	{
		Add(key, mem);
	}
	
	template <typename T, class V=JsonBuilder<T> >
	JsonBuilder(const Key& key, T Host::*mem, const V& rec)
	{
		Add(key, mem, rec);
	}
	
	JsonBuilder(std::initializer_list<JsonBuilder> l)
	{
		for (const auto& i : l)
		{
			m_obj_act.insert(i.m_obj_act.begin(), i.m_obj_act.end());
		}
	}

	template <typename T>
	void Add(const Key& key, T Host::*mem)
	{
		using Builder = SimpleTypeBuilder<T>;
		m_obj_act.emplace(key, std::make_shared<MemberBuilder<Host,T,Builder>>(Builder(), mem));
	}
	
	template <typename T, class Builder=JsonBuilder<T>>
	void Add(const Key& key, T Host::*mem, const Builder& rec)
	{
		// Since the type of the member variable is T, we need a ComplexTypeBuilder<T>
		// to build it. Therefore, the Builder must inherit ComplexTypeBuilder<T>. It
		// cannot be SimpleTextBuilder.
		static_assert(
			std::is_base_of<ComplexTypeBuilder<T>, Builder>::value,
			"member type and visitor does not match");
		m_obj_act.emplace(key, std::make_shared<MemberBuilder<Host,T,Builder>>(rec, mem));
	}
	
	void Data(const Level& current, JSON_event type, const char *data, size_t len) const override
	{
		assert(current.key);
		assert(current.rec == this);
		
		auto i = m_obj_act.find(current.key);
		if (i != m_obj_act.end())
			i->second->Data(current, type, data, len);
	}
	
	Level Advance(const Level& current) const override
	{
		assert(current.key);
		assert(current.rec == this);
		
		Level mock{ current.key, nullptr, MockObjectHandler::Instance() };

		auto i = m_obj_act.find(current.key);
		return i != m_obj_act.end() ?
			i->second->Advance(current) : mock ;
	}

	void Finish(const Level& current) const override
	{
		assert(current.rec == this);
	}

private:
	using MemBase	= ComplexTypeBuilder<Host>;
	using ObjMap	= std::map<Key, std::shared_ptr<const MemBase>> ;
	ObjMap	m_obj_act;
};

} // end of namespace

#endif
