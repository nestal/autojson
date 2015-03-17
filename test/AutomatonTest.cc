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

struct Entry
{
	DataType	type;
	Event		ev;
	std::string	data;
	
	template <typename T>
	Entry(DataType t, Event e, T&& str) : type(t), ev(e), data(str)
	{
	}
};

bool operator==(const Entry& e1, const Entry& e2)
{
	return e1.type == e2.type && e1.ev == e2.ev && e1.data == e2.data;
}

std::ostream& operator<<(std::ostream& os, const Entry& e)
{
	return os << e.type << ' ' << e.ev << " \"" << e.data << '\"';
}

TEST(Simple, AutomatonTest)
{
	std::vector<Entry> actual;
	
	Automaton sub([&](Event v, DataType t, const char *s, std::size_t l){
		actual.emplace_back(t, v, std::string{s,l});
	});

	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\","
		"\"hello2\": \"1234567890abcdefghijk\" "
	" }";
	sub.Parse(js, sizeof(js)-1);
	
	ASSERT_TRUE(sub.Result());
	
	std::vector<Entry> expect {
		{DataType::object,	Event::start, ""},
			{DataType::key,	Event::start, ""},
			{DataType::key,	Event::data, "hello"},
			{DataType::key,	Event::end, ""},
			
			{DataType::string,	Event::start, ""},
			{DataType::string,	Event::data, "1234567890abcdefghijk"},
			{DataType::string,	Event::end, ""},
		
			{DataType::key,	Event::start, ""},
			{DataType::key,	Event::data, "hello2"},
			{DataType::key,	Event::end, ""},
			
			{DataType::string,	Event::start, ""},
			{DataType::string,	Event::data, "1234567890abcdefghijk"},
			{DataType::string,	Event::end, ""},
		{DataType::object,	Event::end, ""},
	};

	ASSERT_EQ(expect, actual);
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
