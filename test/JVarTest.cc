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

#include "ajs/JVar.hh"

#include <functional>
#include <cassert>
#include <vector>
#include <string>
#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <map>

using namespace ajs;

TEST(AssignmentOpCanChangeType, JVarTest)
{
	JVar target(100);
	target = "this is a string";
	
	ASSERT_TRUE(target.Is<std::string>());
	ASSERT_EQ("this is a string", target.Str());
	ASSERT_EQ("this is a string", target);
	ASSERT_EQ("this is a string", target.As<std::string>());
}

TEST(AsReturnByReference, JVarTest)
{
	JVar target;
	target = 100;
	ASSERT_FALSE(target.Is<void>());
	ASSERT_TRUE(target.Is<int>());
	ASSERT_TRUE(target.Is<short>());
	
	long long& i = target.As<int>();
	i = 20;
	ASSERT_EQ(20, target);
	
	const JVar& ref = target;
	ASSERT_EQ(20, ref);
}

TEST(ArrayAddInteger, JVarTest)
{
	JVar target((JVar::Array())), in(1001);
	ASSERT_EQ(1001, in);
	target.Add(in);
	ASSERT_EQ(1, target.Size());
	ASSERT_EQ(1001, target[0]);
}

TEST(AddWillConvertNullToArrayOrHash, JVarTest)
{
	JVar target;
	ASSERT_TRUE(target.IsNull());
	target.Add("target");
	ASSERT_TRUE(target.Is<JVar::Array>());
	ASSERT_EQ(1, target.Size());
	ASSERT_EQ("target", target[0]);
}

TEST(OpSqBracketCanBeNested, JVarTest)
{
	JVar target;
	target.Add("key", std::move(JVar().
		Add("subkey", "value1").
		Add("subkey2", "value2"))
	);
	ASSERT_EQ("value2", target["key"]["subkey2"]);
}

TEST(PrintToStdOut, JVarTest)
{
	JVar target;
	target.Add("abc", "def");
	target.Add("efg", 100.234);
	target.Print(std::cout);
	
	const std::string s("aaa");
	target.Add("cde", s);

	JVar arr;
	arr.Add("123");
	arr.Add(true);
	arr.Print(std::cout);
}
