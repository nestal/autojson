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
	enum class Type { integer, real, boolean, string, array, hash };

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
	explicit Json(std::string&& val);
	explicit Json(const char *val);
	explicit Json(const Array& val);
	explicit Json(Array&& val);
	explicit Json(const Hash& val);
	explicit Json(Hash&& val);

	int AsInt() const;
	long long AsLong() const ;
	double AsDouble() const;
	bool AsBool() const;
	const std::string& AsString() const;
	std::string& AsString();
	const Array& AsArray() const;
	Array& AsArray();
	const Hash& AsHash() const;
	Hash& AsHash();
	bool IsNull() const;

	template <typename F>
	void Apply(F func)
	{
	}

	template <typename T>
	void Assign(const T& val)
	{
		Json tmp(val);
		Swap(tmp);
	}
	
	template <typename T>
	void Assign(T&& val)
	{
		Json tmp(std::move(val));
		Swap(tmp);
	}

	void Swap(Json& rhs);
	
	// complex types only
	void Add(const Json& val);
	void Add(const std::string& key, const Json& val);
	void Clear();
	const Json& operator[](const std::string& key) const;

//	template <typename T> const T& As() const;
//	template <typename T> bool Is() const;

	Type MyType() const;
	bool Is(Type type) const;

private :
	union Raw
	{
		long long	integer;
		double		real;
		bool		boolean;
		std::string	*string;
		Array		*array;
		Hash		*hash;
	};
	
	Type	m_type;
	Raw 	m_raw;
};

} // end of namespace

#endif
