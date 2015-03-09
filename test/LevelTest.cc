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

#include "Level.hh"

#include "Exception.hh"
#include "TypeBuilder.hh"

#include <gtest/gtest.h>

using namespace json;

TEST(Get_pointer_with_the_wrong_type_will_throw, LevelTest)
{
	SimpleTypeBuilder<int> iv;
	
	int idx = 0;
	Level sub(Key(1), &idx, &iv);
	
	ASSERT_THROW(sub.Host<double>(), TypeMismatch);
}

TEST(Get_pointer_with_the_right_type_will_return_it, LevelTest)
{
	SimpleTypeBuilder<std::string> iv;
	
	std::string test = "abc";
	Level sub(Key(1), &test, &iv);
	
	ASSERT_EQ("abc", *sub.Host<std::string>());
}
