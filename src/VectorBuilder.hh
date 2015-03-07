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

#ifndef VECTORBUILDER_HH_INCLUDED
#define VECTORBUILDER_HH_INCLUDED

#include "LevelVisitor.hh"

#include <cassert>

namespace json {

// host is a vector of T
template <
	typename T,
	template <typename, typename> class Container=std::vector,
	typename A=std::allocator<T>
>
class VectorBuilder : public ComplexTypeBuilder<Container<T,A>>
{
public:
	using HostType = Container<T,A>;
	
public:
	VectorBuilder(const VectorBuilder&) = default;
#ifndef _MSC_VER
	VectorBuilder(VectorBuilder&&) = default;
#endif
	~VectorBuilder() override = default;
	
	template <typename Builder=JsonBuilder<T>>
	VectorBuilder(const Builder& v = Builder()) : m_visitor(std::make_shared<Builder>(v))
	{
		assert(m_visitor);
		static_assert(
			std::is_base_of<ComplexTypeBuilder<T>, Builder>::value,
			"member type and visitor does not match");

	}

	void Data(const Level& current, JSON_event type, const char *data, size_t len) const override
	{
		assert(current.key);
		assert(current.rec == this);
		assert(m_visitor);

		auto host = this->Self(current);
		host->emplace_back();
		
		m_visitor->Data(Level{current.key, &host->back(), m_visitor.get()}, type, data, len);
	}
	
	Level Advance(const Level& current) const override
	{
		assert(current.key);
		assert(current.rec == this);
		assert(m_visitor);
		
		auto host = this->Self(current);
		host->emplace_back();

		return Level{current.key, &host->back(), m_visitor.get()};
	}

	void Finish(const Level& current) const override
	{
		assert(current.rec == this);
	}

private:
	std::shared_ptr<const LevelVisitor>	m_visitor;
};

} // end of namespace

#endif
