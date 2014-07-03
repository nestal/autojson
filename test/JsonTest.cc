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

#include <functional>
#include <cassert>
#include <vector>
#include <string>
#include <gtest/gtest.h>

#include <iostream>
#include <memory>
#include <map>

struct JsonToken
{
	JSON_event	type;
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
		"JSON_object_end",
		"JSON_string",		// string as object
		"JSON_number",		// number
		"JSON_null",
		"JSON_true",
		"JSON_false"
	};
	
	return os << "{" << state[t.type] << ", \"" << t.value << "\"}";
}

bool operator==(const JsonToken& t1, const JsonToken& t2)
{
	return t1.type == t2.type && t1.value == t2.value;
}

void Callback(void *pvec, JSON_event type, const char *data, int len)
{
	std::vector<JsonToken>	*vec = reinterpret_cast<std::vector<JsonToken>*>(pvec);
	
	// MSVC needs two lines
	JsonToken t {type, data != nullptr ? std::string(data, len) : ""};
	vec->push_back(t);
	assert(vec->back().value.size() == len);
}

TEST(ParsedOutputInCallback, JsonTest)
{
	JSON_checker jc = new_JSON_checker(5);

	std::vector<JsonToken> actual;

	const char js[] = "{ \"hello\": 100, \"world\": 200.03, \"a\\tr\"   : [1,true,3,4,{\"k\":5},\"abc\\u00fF\", false] }";
	ASSERT_EQ(JSON_ok, JSON_checker_char(jc, js, sizeof(js)-1, &Callback, &actual));
	
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
	ASSERT_EQ(JSON_ok, JSON_checker_char(jc, js, 20, &Callback, &actual));
	ASSERT_EQ(JSON_ok, JSON_checker_char(jc, js+20, sizeof(js)-20-1, &Callback, &actual));
	
	std::vector<JsonToken> expect =
	{
		{JSON_object_start, ""},
		{JSON_object_key, "hello"},
		{JSON_string, "1234567890abcdefghijk"},
		{JSON_object_end, ""},
	};
	ASSERT_EQ(expect, actual);
}

class Handler;
struct ParseState
{
	Handler	*handler;
	void	*ptr;
};

class Handler
{
public :
	virtual ParseState On(ParseState& s, JSON_event event, const char *data, int len) = 0;
};

template <typename DestType, typename T>
class SaveToMember : public Handler
{
public :
	SaveToMember(T DestType::*member) : m_member(member)
	{
	}

	ParseState On(ParseState& s, JSON_event event, const char *data, int len) override
	{
		DestType *dest = reinterpret_cast<DestType*>(s.ptr);
		(dest->*m_member) = std::string(data, len);
		return s;
	}

private :
	T DestType::*m_member;
};

template <typename DestType>
class ObjectReactor : public Handler
{
public:
	ObjectReactor() : m_next(nullptr)
	{
	}

	void Parse(DestType& t, JSON_checker jc, const char *json, std::size_t len)
	{
		ParseState root = {this, &t};
		std::vector<ParseState> vec(1, root);

		::JSON_checker_char(jc, json, static_cast<int>(len), &ReactorCallback, &vec);
	}

	template <typename T>
	ObjectReactor& Map(const std::string& key, T DestType::*member)
	{
		m_actions.insert(
			std::make_pair(key, HandlerPtr(new SaveToMember<DestType,T>(member))));
		return *this;
	}

	ParseState On(ParseState& s, JSON_event event, const char *data, int len) override
	{
		if (event == JSON_object_key)
		{
			auto i = m_actions.find(std::string(data,len)) ;
			m_next = (i != m_actions.end() ? i->second.get() : nullptr);
		}
		else if (m_next != nullptr)
		{
			switch (event)
			{
			case JSON_string:
			case JSON_null:
			case JSON_true:
			case JSON_false:
			case JSON_number:
				m_next->On(s, event, data, len);
				break;

			case JSON_object_start:
				break;
			}
			// only use once
			m_next = nullptr;
		}
		return s;
	}

private :
	typedef std::unique_ptr<Handler> HandlerPtr;
	
	std::map<std::string, HandlerPtr> m_actions;
	Handler *m_next;
};

void ReactorCallback(void *user, JSON_event type, const char *data, int len)
{
	std::vector<ParseState> *state =
		reinterpret_cast<std::vector<ParseState>*>(user);

	ParseState p = state->back().handler->On(state->back(), type, data, len);
	if (p.handler != state->back().handler)
		state->push_back(p) ;
	else if (p.handler == nullptr)
		state->pop_back();
}

TEST(TryOutCpp, JsonTest)
{
	struct Subject
	{
		std::string value;
		struct Sub
		{
			std::string v2;
		} sub;
	};

	JSON_checker jc = new_JSON_checker(5);
	Subject j {};
	const char js[] = "{ \"haha\": \"fun\" }";

	ObjectReactor<Subject> r;
	r.Map("haha", &Subject::value);
	r.Parse(j, jc, js, sizeof(js)-1);
	
	ASSERT_EQ("fun", j.value) ;
}
