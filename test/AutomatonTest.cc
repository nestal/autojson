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
#include "Exception.hh"

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

class AutomatonTest : public ::testing::Test
{
protected:
	virtual void SetUp()
	{
		m_sub = new Automaton([&](Event v, DataType t, const char *s, std::size_t l){
			m_actual.emplace_back(t, v, std::string{s,l});
		});
	}
	
	virtual void TearDown()
	{
		delete m_sub;
	}
	
	Automaton	*m_sub;
	std::vector<Entry> m_actual;
};

TEST_F(AutomatonTest, Simple)
{
	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\","
		"\"hello2\": \"1234567890abcdefghijk\" "
	" }";
	m_sub->Parse(js, sizeof(js)-1);
	
	ASSERT_TRUE(m_sub->Result());
	
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

	ASSERT_EQ(expect, m_actual);
}

TEST_F(AutomatonTest, SimpleError)
{
	const char js[] = "{ \"hello\": \"1234567890abcdefghijk\"";
	m_sub->Parse(js, sizeof(js)-1);
	
	ASSERT_FALSE(m_sub->Result());
}

TEST_F(AutomatonTest, TestEscape)
{
	const char js[] = "{\"1234\": \"a\\n1234\" }";
	m_sub->Parse(js, sizeof(js)-1);
	ASSERT_TRUE(m_sub->Result());
	
	std::vector<Entry> expect {
		{DataType::object,	Event::start, ""},
		{DataType::key,	Event::start, ""},
		{DataType::key,	Event::data, "1234"},
		{DataType::key,	Event::end, ""},
		{DataType::string,	Event::start, ""},
		{DataType::string,	Event::data, "a"},
		{DataType::string,	Event::data, "\n"},
		{DataType::string,	Event::data, "1234"},
		{DataType::string,	Event::end, ""},
		{DataType::object,	Event::end, ""},
	};
	
	ASSERT_EQ(expect, m_actual);
}

TEST_F(AutomatonTest, TestResume)
{
	const char js1[] = "{\"1234\": \"part one\", \"56";
	const char js2[] = "78\": \"part two\"}";

	m_sub->Parse(js1, sizeof(js1)-1);
	m_sub->Parse(js2, sizeof(js2)-1);
	ASSERT_TRUE(m_sub->Result());
	
	std::vector<Entry> expect {
		{DataType::object,	Event::start, ""},
		
		{DataType::key,	Event::start, ""},
		{DataType::key,	Event::data, "1234"},
		{DataType::key,	Event::end, ""},
		{DataType::string,	Event::start, ""},
		{DataType::string,	Event::data, "part one"},
		{DataType::string,	Event::end, ""},
		
		{DataType::key,	Event::start, ""},
		{DataType::key,	Event::data, "5678"},
		{DataType::key,	Event::end, ""},
		{DataType::string,	Event::start, ""},
		{DataType::string,	Event::data, "part two"},
		{DataType::string,	Event::end, ""},
		
		{DataType::object,	Event::end, ""},
	};
}

TEST_F(AutomatonTest, TestParseError)
{
	const char js1[] = "{\n\n\"1234\": 9\"part one\", \"56";

	try
	{
		m_sub->Parse(js1, sizeof(js1)-1);
		ASSERT_TRUE(false);
	}
	catch (ParseError& e)
	{
		ASSERT_EQ(2, e.Line());
		ASSERT_EQ(10, e.Column());
	}
	catch (...)
	{
		ASSERT_TRUE(false);
	}
}
