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

namespace json {

class Cursor;

/**	The abstract visitor
*/
class JsonProcessor
{
public:
	virtual ~JsonProcessor() = default;
	virtual void Data(const Cursor& current, JSON_event type, const char *data, size_t len) const = 0;
	virtual Cursor Advance(const Cursor& current) const = 0;
	virtual void Finish(const Cursor& current) const = 0;
};

template <typename Host>
class TypeBuilder : public JsonProcessor
{
public:
	virtual ~TypeBuilder() = default;
};


} // end of namespace

#endif
