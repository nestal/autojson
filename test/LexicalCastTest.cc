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

#include "LexicalCast.hh"

#include "Exception.hh"
#include "TypeBuilder.hh"

#include <gtest/gtest.h>

using namespace json;

TEST(LexicalCastTest, Cast_integer)
{
	ASSERT_EQ(100,  LexicalCast<std::int32_t>("100", 3) );
	ASSERT_EQ(100L, LexicalCast<std::int32_t>("100", 3));
	ASSERT_EQ(123UL, LexicalCast<std::uint64_t>("123", 3));
}

TEST(LexicalCastTest, Cast_double)
{
	ASSERT_EQ(100.0,   LexicalCast<double>("100", 3) );
	ASSERT_EQ(101.0,   LexicalCast<double>("101", 3) );
	ASSERT_EQ(123.123, LexicalCast<double>("123.123", 7) );
}

TEST(LexicalCastTest, Unescape_normal)
{
	ASSERT_EQ("ABC\n",	Unescape("ABC\\n"));
}
