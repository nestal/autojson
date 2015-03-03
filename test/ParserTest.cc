/*
	autojson: A JSON parser base on the automaton provided by json.org
	Copyright (C) 2015  Wan Wai Ho

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

#include <gtest/gtest.h>

#include "JsonParser.hh"
#include "LevelVisitor.hh"

#include <iterator>
#include <fstream>

using namespace json;

TEST(ParserTest, Test1Level)
{
	struct Person
	{
		std::string name;
		double      age;
	};
	
	JsonBuilder<Person> h;
	h.Add("myv", &Person::age);
	
	Person staff;
	JsonParser sub(&h);
		
	const char json[] = "{\"myv\": 321, \"staff\": {\"name\": \"Mary\", \"age\": 70}}";
	sub.Parse(json, sizeof(json)-1, &staff);
	sub.Done();
	ASSERT_EQ(321.0,   staff.age);
}

TEST(ParserTest, Test2Level)
{
	struct Person
	{
		std::string name;
		double		age;
	};
	
	struct Book
	{
		std::string title;
		Person 		author;
		std::size_t pages;
		std::string appendix;
	};
	
	JsonBuilder<Book> h =
	{
		{"title", 	&Book::title},
		{"author",	&Book::author, {
			{"name",	&Person::name},
			{"age",		&Person::age}}
		},
		{"pages",	&Book::pages}
	};
		
	
	JsonParser sub(&h);
	const char json[] =
	"{"
		"\"title\": \"Pride and Prejudices\","
		"\"author\": {\"name\": \"John\", \"age\": 25.2},"
		"\"pages\": 100,"
		"\"appendix\": \"???\","
	"}";
	
	Book b;
	b.appendix = "none";
	sub.Parse(json, sizeof(json)-1, &b);
	sub.Done();
	ASSERT_EQ("Pride and Prejudices", b.title);
	ASSERT_EQ("John", b.author.name);
	ASSERT_EQ(25.2,   b.author.age);
	ASSERT_EQ(100,    b.pages);
	ASSERT_EQ("none", b.appendix);
}

namespace
{
	class RealPerson
	{
	public:
		RealPerson(const std::string& name = "", double age = 0.0) : m_name(name), m_age(age)
		{
		}
		
		std::string Name() const { return m_name; }
		double Age() const { return m_age; }
		
		static const JsonBuilder<RealPerson>& Json()
		{
			static const auto p = JsonBuilder<RealPerson>{
				{"name", &RealPerson::m_name},
				{"age",  &RealPerson::m_age}
			};
			return p;
		}
	private :
		std::string	m_name;
		double		m_age;
	};
}

TEST(ParserTest, TestComplexClass)
{
	JsonParser sub(&RealPerson::Json());
	
	RealPerson p;
	const char json[] =
	"{"
		"\"name\": \"Isis\","
		"\"age\": 5.1,"
	"}";
	sub.Parse(json, sizeof(json)-1, &p);
	
	ASSERT_EQ(p.Age(), 5.1);
}

TEST(ParserTest, GoogleDriveListTest)
{
	std::string json;
	std::ifstream test_file(TEST_DATA "paddrive.json");
	std::copy(
		(std::istreambuf_iterator<char>(test_file)),
		(std::istreambuf_iterator<char>()),
		std::back_inserter(json));

	struct Label
	{
		bool starred;
		bool hidden;
		bool trashed;
		bool restricted;
		bool viewed;
	};
	
	struct FileList
	{
		std::string kind;
		Label		labels;
	};
	
	JsonBuilder<FileList> h
	{
		{ "kind", 	&FileList::kind },
		{ "labels",	&FileList::labels, {
			{"starred",	&Label::starred},
			{"hidden",	&Label::hidden},
			{"trashed",	&Label::trashed},
			{"restricted",	&Label::restricted},
			{"viewed",	&Label::viewed}}}
	};
	JsonParser sub(&h);
	
	FileList list;
	list.labels.starred = true;
	sub.Parse(json.data(), json.size(), &list);
	
	ASSERT_EQ(list.kind, "drive#fileList");
//	ASSERT_FALSE(list.labels.starred);
}
