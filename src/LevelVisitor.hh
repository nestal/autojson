/*
	songbits: A cloud-base music player
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
#include <memory>
#include <map>

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
	virtual void Data(const Key& key, JSON_event type, const char *data, size_t len, void *obj) const = 0;
	virtual Level Advance(const Key& key, void *obj) const = 0;
};

class MockObjectHandler : public LevelVisitor
{
public:
	void Data(const Key&, JSON_event, const char*, size_t, void*) const override {}
	Level Advance(const Key& key, void *) const override
	{
		return Level{key, nullptr, Instance()};
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
	SimpleTypeBuilder(SimpleTypeBuilder&&) = default;
	~SimpleTypeBuilder() override = default;
	
	void Data(const Key& key, JSON_event, const char *data, size_t len, void *obj) const override
	{
		*static_cast<T*>(obj) = lexical_cast<T>(data, len);
	}
	
	Level Advance(const Key& key, void *obj) const override
	{
		return Level{key, nullptr, MockObjectHandler::Instance()};
	}
};

template <typename Host>
class JsonBuilder : public LevelVisitor
{
public :
	JsonBuilder() = default;
	JsonBuilder(const JsonBuilder&) = default;
	JsonBuilder(JsonBuilder&&) = default;
	~JsonBuilder() override = default;
	
	template <typename T>
	JsonBuilder(const Key& key, T Host::*mem)
	{
		Add(key, mem, SimpleTypeBuilder<T>());
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
		using V = SimpleTypeBuilder<T>;
		m_obj_act.emplace(key, std::make_shared<Action<T,V>>(V(), mem));
	}
	
	template <typename T, class V>
	void Add(const Key& key, T Host::*mem, const V& rec)
	{
		m_obj_act.emplace(key, std::make_shared<Action<T,V>>(rec, mem));
	}
	
	void Data(const Key& key, JSON_event type, const char *data, size_t len, void *obj) const override
	{
		assert(key);
		assert(obj);
		
		auto i = m_obj_act.find(key);
		if (i != m_obj_act.end())
			i->second->OnData(key, type, data, len, static_cast<Host*>(obj));
	}
	
	Level Advance(const Key& key, void *obj) const override
	{
		assert(key);
		assert(obj);
		
		auto i = m_obj_act.find(key);
		return i != m_obj_act.end() ?
			i->second->OnAdvance(key, static_cast<Host*>(obj)) :
			Level{key, nullptr, MockObjectHandler::Instance()};
	}
	
private:
	class BaseAction
	{
	public:
		virtual ~BaseAction() = default;
		virtual Level OnAdvance(const Key& key, Host *h) const = 0;
		virtual void OnData(const Key&, JSON_event, const char *data, size_t len, Host *h) const = 0;
	};
	
	template <typename T, class V>
	class Action : public BaseAction
	{
	public:
		Action(const V& rec, T Host::*m ) :
			m_rec(rec),
			m_mem(m)
		{
		}
		
		Action(const Action&) = default;
		
		Level OnAdvance(const Key& key, Host *h) const override
		{
			return Level{key, &(h->*m_mem), &m_rec};
		}
		
		void OnData(const Key& key, JSON_event type, const char *data, size_t len, Host *h) const override
		{
			m_rec.Data(key, type, data, len, &(h->*m_mem));
		}
		
	private:
		V			m_rec;
		T Host::*	m_mem;
	};
	
	using ObjMap	= std::map<Key, std::shared_ptr<const BaseAction>> ;
	ObjMap	m_obj_act;
};

} // end of namespace

#endif
