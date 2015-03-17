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
	std::vector<DataType> dt_actual;
	std::vector<Event> ev_actual;
	
	Automaton sub([&](Event v, DataType t, const char *, std::size_t){
		ev_actual.push_back(v);
		dt_actual.push_back(t);
	});

	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\","
		"\"hello2\": \"1234567890abcdefghijk\" "
	" }";
	sub.Parse(js, sizeof(js)-1);
	
	ASSERT_TRUE(sub.Result());
	
	std::vector<DataType> dt_expect {
		DataType::object,
			DataType::key,
			DataType::key,
			DataType::key,
			
			DataType::string,
			DataType::string,
			DataType::string,
		
			DataType::key,
			DataType::key,
			DataType::key,

			DataType::string,
			DataType::string,
			DataType::string,
		DataType::object
	};
	ASSERT_EQ(dt_expect, dt_actual);
	std::vector<Event> ev_expect{
		Event::start,
			Event::start,
			Event::data,
			Event::end,

			Event::start,
			Event::data,
			Event::end,

			Event::start,
			Event::data,
			Event::end,

			Event::start,
			Event::data,
			Event::end,

		Event::end
	};
	ASSERT_EQ(ev_expect, ev_actual);
}

TEST(SimpleError, AutomatonTest)
{
	Automaton sub([](Event v, DataType, const char *p, std::size_t s){
//		if (v == Event::string_data)
//			std::cout << std::string(p,s) << std::endl;
	});

	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\"";
	sub.Parse(js, sizeof(js)-1);
	
	ASSERT_FALSE(sub.Result());
}
