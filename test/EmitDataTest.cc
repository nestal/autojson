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

#include "EmitData.hh"
#include "Range.hh"

#include <gtest/gtest.h>

using namespace json;

TEST(EmitDataTest, Flush_can_get_back_Saved_data)
{
	const char str[] = "hello world";
	
	EmitData sub;
	sub.Save(str);
	
	EmitData::Buf b = sub.Get(std::end(str)-1);
	ASSERT_TRUE(std::equal(b.begin(), b.end(), std::begin("hello world")));
}

TEST(EmitDataTest, Stash_and_unstash)
{
	const char str[] = "hello world";
	
	EmitData sub;
	sub.Save(str);
	sub.Stash(std::end(str)-1);
	
	sub.Save(str);
	EmitData::Buf b = sub.Get(std::end(str)-1);
	
	ASSERT_TRUE(std::equal(b.begin(), b.end(), std::begin("hello worldhello world")));
}

TEST(EmitDataTest, Stash_and_unstash_twice)
{
	const char str[] = "\\n123";
	
	EmitData sub;
	sub.Save(str);
	sub.Stash(std::end(str)-1);
	
	sub.Save(str);
	sub.Stash(std::end(str)-1);
	
	sub.Save(str);
	EmitData::Buf b = sub.Get(std::end(str)-2);
	
	ASSERT_TRUE(std::equal(b.begin(), b.end(), std::begin("\\n123\\n123\\n12")));
}

TEST(EmitDataTest, Clear_will_clear_stash_data)
{
	const char str[] = "*sample";
	
	EmitData sub;
	sub.Save(str);
	sub.Stash(std::end(str)-1);
	
	sub.Clear();
	ASSERT_FALSE(sub.IsSaved());
	
	const char str2[] = "$other";
	sub.Save(str2);		// re-save. will clear previously stashed data
	EmitData::Buf b = sub.Get(std::end(str2)-1);
	
	ASSERT_TRUE(std::equal(b.begin(), b.end(), std::begin("$other")));
}

TEST(EmitDataTest, Get_will_not_clear_data)
{
	const char str[] = "*sample";
	
	EmitData sub;
	sub.Save(str);
	EmitData::Buf b = sub.Get(std::end(str)-1);
	ASSERT_TRUE(std::equal(b.begin(), b.end(), std::begin("*sample")));
	sub.Stash(std::end(str)-1);

	const char str2[] = "$other";
	sub.Save(str2);		// re-save. will clear previously stashed data
	EmitData::Buf b2 = sub.Get(std::end(str2)-1);
	
	ASSERT_TRUE(std::equal(b2.begin(), b2.end(), std::begin("*sample$other")));
}
