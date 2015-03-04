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
#include <vector>

namespace json {

// host is a vector of T
template <typename T>
class VectorBuilder : public ComplexTypeBuilder<std::vector<T>>
{
public:
	VectorBuilder(const VectorBuilder&) = default;
#ifndef _MSC_VER
	VectorBuilder(VectorBuilder&&) = default;
#endif
	~VectorBuilder() override = default;
	
	template <typename V=JsonBuilder<T>>
	VectorBuilder(const V& v = V()) : m_visitor(std::make_shared<V>(v))
	{
		assert(m_visitor.get());
	}

	void OnData(const Key& key, JSON_event type, const char *data, size_t len, std::vector<T> *host) const override
	{
		assert(key);
		assert(host);

		host->emplace_back();
		
		assert(m_visitor.get());
		m_visitor->Data(key, type, data, len, &host->back());
	}
	
	Level OnAdvance(const Key& key, std::vector<T> *host) const override
	{
		assert(key);
		assert(host);
		
		host->emplace_back();

		assert(m_visitor.get());
		return Level{key, &host->back(), m_visitor.get()};
	}

private:
	std::shared_ptr<const LevelVisitor>	m_visitor;
};

} // end of namespace

#endif
