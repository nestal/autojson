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

#include "JSON_checker.h"

#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <gtest/gtest.h>

struct JsonToken
{
	JSON_type	type;
	std::string	value;
};

std::ostream& operator<<(std::ostream& os, const JsonToken& t)
{
	static const char *state[] =
	{
		"JSON_array_start",
		"JSON_array_end",
		"JSON_object_start",
		"JSON_object_key",	// string as object key
		"JSON_string",		// string as object
		"JSON_number",		// number
		"JSON_null",
		"JSON_true",
		"JSON_false",
		"JSON_object_end"
	};
	
	return os << "{" << state[t.type] << ", \"" << t.value << "\"}";
}

bool operator==(const JsonToken& t1, const JsonToken& t2)
{
	return t1.type == t2.type && t1.value == t2.value;
}

void Callback(void *pvec, JSON_type type, const char *data, int len)
{
	std::vector<JsonToken>	*vec = reinterpret_cast<std::vector<JsonToken>*>(pvec);
	vec->push_back({type, data != nullptr ? std::string(data, len) : ""});
}

TEST(ParsedOutputInCallback, JsonTest)
{
	JSON_checker jc = new_JSON_checker(5);

	std::vector<JsonToken> actual;

	const char js[] = "{ \"hello\": 100, \"world\": 200.03, \"a\\tr\"   : [1,true,3,4,{\"k\":5},\"abc\\u00fF\", false] }";
	ASSERT_TRUE(JSON_checker_char(jc, js, sizeof(js)-1, &Callback, &actual));
	
	std::vector<JsonToken> expect =
	{
		{JSON_object_start, ""},
		{JSON_object_key, "hello"},
		{JSON_number, "100"},
		{JSON_object_key, "world"},
		{JSON_number, "200.03"},
		{JSON_object_key, "a\\tr"},
		{JSON_array_start, ""},
		{JSON_number, "1"},
		{JSON_true, ""},
		{JSON_number, "3"},
		{JSON_number, "4"},
		{JSON_object_start, ""},
		{JSON_object_key, "k"},
		{JSON_number, "5"},
		{JSON_object_end, ""},
		{JSON_string, "abc\\u00fF"},
		{JSON_false, ""},
		{JSON_array_end, ""},
		{JSON_object_end, ""},
	};
	
	ASSERT_EQ(expect, actual);
}

TEST(PartialJsonCanBeParsed, JsonTest)
{
	JSON_checker jc = new_JSON_checker(5);

	std::vector<JsonToken> actual;

	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\" }";
	ASSERT_TRUE(JSON_checker_char(jc, js, 20, &Callback, &actual));
	ASSERT_TRUE(JSON_checker_char(jc, js+20, sizeof(js)-20-1, &Callback, &actual));
	
	std::vector<JsonToken> expect =
	{
		{JSON_object_start, ""},
		{JSON_object_key, "hello"},
		{JSON_string, "1234567890abcdefghijk"},
		{JSON_object_end, ""},
	};
	ASSERT_EQ(expect, actual);
}
