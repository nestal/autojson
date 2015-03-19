/*
	songbits: A cloud-base music player
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

#ifndef EMITDATA_HH_INCLUDED
#define EMITDATA_HH_INCLUDED

#include <string>

namespace json {

template <typename T>
class Range;
	
/**	Brief description of EmitData
*/
class EmitData
{
public:
	EmitData() ;

	using Buf = Range<const char*>;
	
	void Save(const char *p);
	Buf Flush(const char *p);
	
	void Stash(const char *p);
	void Unstash(const char *p);

private:
	const char	*m_start;
	std::string	m_tmp;
};

} // end of namespace

#endif
