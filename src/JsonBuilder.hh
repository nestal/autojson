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

#ifndef JSONBUILDER_HH_INCLUDED
#define JSONBUILDER_HH_INCLUDED

#include "TypeBuilder.hh"

#include <initializer_list>
#include <map>
#include <memory>

namespace json {

/**	Brief description of JsonBuilder
*/
template <typename Host>
class JsonBuilder : public TypeBuilder<Host>
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
		// Since the type of the member variable is T, we need a TypeBuilder<T>
		// to build it. Therefore, the Builder must inherit ComplexTypeBuilder<T>. It
		// cannot be SimpleTextBuilder.
		static_assert(
			std::is_base_of<TypeBuilder<T>, Builder>::value,
			"member type and visitor does not match");
		m_obj_act.emplace(key, std::make_shared<MemberBuilder<Host,T,Builder>>(rec, mem));
	}
	
	void Data(const Level& current, JSON_event type, const char *data, size_t len) const override
	{
		assert(current.Key());
		assert(current.Rec() == this);
		
		auto i = m_obj_act.find(current.Key());
		if (i != m_obj_act.end())
			i->second->Data(current, type, data, len);
	}
	
	Level Advance(const Level& current) const override
	{
		assert(current.Key());
		assert(current.Rec() == this);
		
		Level mock{ current.Key()};

		auto i = m_obj_act.find(current.Key());
		return i != m_obj_act.end() ?
			i->second->Advance(current) : mock ;
	}

	void Finish(const Level& current) const override
	{
		assert(current.Rec() == this);
	}

private:
	using MemBase	= TypeBuilder<Host>;
	using ObjMap	= std::map<Key, std::shared_ptr<const MemBase>> ;
	ObjMap	m_obj_act;
};

} // end of namespace

#endif
