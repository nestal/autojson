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

#include "Automaton.hh"
#include <gtest/gtest.h>

#include <iostream>

using namespace json;

TEST(Simple, AutomatonTest)
{
	std::vector<Event> actual;
	
	Automaton sub([&](Event v, const char *, std::size_t){
		actual.push_back(v);
	});

	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\","
		"\"hello2\": \"1234567890abcdefghijk\" "
	" }";
	sub.Parse(js, sizeof(js)-1);
	
	ASSERT_TRUE(sub.Result());
	
	std::vector<Event> expect {
		Event::object_start,
			Event::object_key,
				Event::string_start,
				Event::string_data,
				Event::string_end,
			
			Event::string_start,
			Event::string_data,
			Event::string_end,

			Event::object_key,
				Event::string_start,
				Event::string_data,
				Event::string_end,
			
			Event::string_start,
			Event::string_data,
			Event::string_end,

		Event::object_end
	};
	ASSERT_EQ(expect, actual);
}

TEST(SimpleError, AutomatonTest)
{
	Automaton sub([](Event v, const char *p, std::size_t s){
		if (v == Event::string_data)
			std::cout << std::string(p,s) << std::endl;
	});

	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\"";
	sub.Parse(js, sizeof(js)-1);
	
	ASSERT_FALSE(sub.Result());
}
