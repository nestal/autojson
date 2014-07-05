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

#ifndef JSON_HH_INCLUDED
#define JSON_HH_INCLUDED

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace ajs {

/**	A variant class to define the structure of an expected JSON object
*/
class Json
{
public :
	typedef std::vector<Json> 			Array;
	typedef std::map<std::string, Json>	Hash;

public :
	// special member functions
	Json(const Json& rhs);
	Json(Json&& rhs);
	Json& operator=(const Json& rhs);
	Json& operator=(Json&& rhs);
	~Json();

	// construction from supported types
	Json();
	explicit Json(int val);
	explicit Json(long long val);
	explicit Json(double val);
	explicit Json(bool val);
	explicit Json(const std::string& val);
	explicit Json(const Array& val);
	explicit Json(const Hash& val);

	int AsInt() const;
	long long AsLong() const ;
	double AsDouble() const;
	bool AsBool() const;
	const std::string& AsString() const;
	const Array& AsArray() const;
	const Hash& AsHash() const;
	bool IsNull() const;

	template <typename T>
	void Assign(const T& val);
	void Assign(int val);

	void Swap(Json& rhs);
	
	// complex types only
	void Append(const Json& val);
	void Insert(const std::string& key, const Json& val);
	void Clear();

	template <typename T> const T& As() const;
	template <typename T> bool Is() const;
	
	// schema related
	std::size_t MaxSize() const;
	bool Optional() const;

private :
	struct Base;
	std::unique_ptr<Base>	m_base;
	
	template <typename T> struct Var;
	
	template <typename T> Var<T>* Cast();
	template <typename T> const Var<T>* Cast() const;
};

} // end of namespace

#endif
