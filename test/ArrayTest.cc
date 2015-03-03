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
#include "VectorBuilder.hh"

#include <iterator>
#include <fstream>

using namespace json;

class ItemArrayTest : public testing::Test
{
protected:
	struct Item
	{
		std::string kind;
		std::string title;
		int page;
	};
	
	static const JsonBuilder<Item> item_level;
	
	virtual void SetUp()
	{
		std::ifstream test_file(TEST_DATA "array.json");
		std::copy(
			(std::istreambuf_iterator<char>(test_file)),
			(std::istreambuf_iterator<char>()),
			std::back_inserter(m_json));
	}
	
	std::string m_json;
};

const JsonBuilder<ItemArrayTest::Item> ItemArrayTest::item_level =
{
	{"kind", &Item::kind},
	{"title", &Item::title},
	{"page", &Item::page}
};

TEST_F(ItemArrayTest, SimpleArrayTest)
{
	struct Items
	{
		Item item0;
		Item item1;
	};
		
	struct FileList
	{
		std::string kind;
		Items items;
	};

	JsonBuilder<FileList> list_level =
	{
		{"kind", &FileList::kind},
		{"items", &FileList::items, {
			{0, &Items::item0, item_level},
			{1, &Items::item1, item_level},
		}}
	};
	
	JsonParser sub(&list_level);
	
	FileList list;
	sub.Parse(m_json.data(), m_json.size(), &list);
	
	ASSERT_EQ("Title of item[0]", 	list.items.item0.title);
	ASSERT_EQ("Folder's Title",		list.items.item1.title);
}

TEST_F(ItemArrayTest, VectorArrayTest)
{
	struct FileList
	{
		std::string kind;
		std::vector<Item> items;
	};

	JsonBuilder<FileList> list_level =
	{
		{"kind", &FileList::kind},
		{"items", &FileList::items, VectorBuilder<Item>(item_level)}
	};
	
	JsonParser sub(&list_level);
	
	FileList list;
	sub.Parse(m_json.data(), m_json.size(), &list);
	
	ASSERT_EQ("hello world!", list.kind);
	
	ASSERT_EQ(2, 	list.items.size());
	ASSERT_EQ(100,  list.items[0].page);
	ASSERT_EQ("Title of item[0]", 	list.items[0].title);
	ASSERT_EQ("Folder's Title",		list.items[1].title);
}
