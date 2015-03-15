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

#include "Automaton.hh"

#include <vector>

namespace json {

namespace chars
{
	// character types. each character can be classified to any of the types below
	enum Type
	{
		space,  // space */
		white,  // other whitespace characters
		lcurb,  // {
		rcurb,  // }
		lsqrb,  // [
		rsqrb,  // ]
		colon,  // :
		comma,  // ,
		quote,  // "
		backs,  /* \ */
		slash,  // /
		plus,	// +
		minus,  // -
		period, // .
		zero,	// 0
		digit,  // 123456789
		a,	   // lower case characters
		b,
		c,
		d,
		e,
		f,
		l,
		n,
		r,
		s,
		t,
		u,
		abcdf,  // upper case ABCDF
		upperE, // upper case E
		etc,	 // everything else
		
		bad,	// invalid control characters
		
		// number of character types
		ctype_count
	};
	
	// deduce type from character
	Type Deduce(char ch)
	{
		static const Type ascii[128] = {
		/*
			This array maps the 128 ASCII characters into character classes.
			The remaining Unicode characters should be mapped to etc.
			Non-whitespace control characters are errors.
		*/
			bad,	bad,	bad,	bad,	bad,	bad,	bad,	bad,
			bad,	white,	white,	bad,	bad,	white,	bad,	bad,
			bad,	bad,	bad,	bad,	bad,	bad,	bad,	bad,
			bad,	bad,	bad,	bad,	bad,	bad,	bad,	bad,

			space,	etc,	quote,	etc,	etc,	etc,	etc,	etc,
			etc,	etc,	etc,	plus,	comma,	minus,	period,	slash,
			zero,	digit,	digit,	digit,	digit,	digit,	digit,	digit,
			digit,	digit,	colon,	etc,	etc,	etc,	etc,	etc,

			etc,	abcdf,	abcdf,	abcdf,	abcdf,	upperE,	abcdf,	etc,
			etc,	etc,	etc,	etc,	etc,	etc,	etc,	etc,
			etc,	etc,	etc,	etc,	etc,	etc,	etc,	etc,
			etc,	etc,	etc,	lsqrb,	backs,	rsqrb,	etc,	etc,

			etc,	a,		b,		c,		d,		e,		f,		etc,
			etc,	etc,	etc,	etc,	l,		etc,	n,		etc,
			etc,	etc,	r,		s,		t,		u,		etc,	etc,
			etc,	etc,	etc,	lcurb,	etc,	rcurb,	etc,	etc
		};
		
		// invalid character
		if (ch < 0)
			throw -1;
		
		return ch >= sizeof(ascii) ? etc : ascii[ch];
	}
	
} // end of namespace chars

namespace state
{
	/*
    The state codes.
	*/
	enum State {
		go,  /* start    */
		ok,  /* ok       */
		obj, /* object   */
		key, /* key      */
		col, /* colon    */
		val, /* value    */
		arr, /* array    */
		str, /* string   */
		esp, /* escape   */
		u1,  /* u1       */
		u2,  /* u2       */
		u3,  /* u3       */
		u4,  /* u4       */
		mi_, /* minus    */
		ze0, /* zero     */
		inT, /* integer  */
		frt, /* fraction */
		ex1, /* e        */
		ex2, /* ex       */
		ex3, /* exp      */
		tr1, /* tr       */
		tr2, /* tru      */
		tr3,  /* true     */
		fe1,  /* fa       */
		fe2,  /* fal      */
		fe3,  /* fals     */
		fe4,  /* false    */
		n01,  /* nu       */
		n02,  /* nul      */
		n03,  /* null     */
		
		state_count
	};
	
	enum Action
	{
		SOJ,	// start of object
		EOJ,	// end of object
		SAR,	// start of array
		EAR,	// end of array
	};
	
	// bad state
	const int ___ = -1;
	
	State Next(State current, chars::Type input)
	{
		static const int state_transition_table[state_count][chars::ctype_count] = {
/*
	The state transition table takes the current state and the current symbol,
	and returns either a new state or an action. An action is represented as a
	negative number. A JSON text is accepted if at the end of the text the
	state is OK and if the mode is MODE_DONE.

				  white                                                     1-9                                                ABCDF   etc
			  space |    {   }   [   ]   :   ,   "   \   /   +   -   .   0   |   a   b   c   d   e   f   l   n   r   s   t   u   |   E  | */
/*start  go */ {go ,go ,-6 ,___,-5 ,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*ok     ok */ {ok ,ok ,___,-8 ,___,-7 ,___,-3 ,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*object obj*/ {obj,obj,___,-9 ,___,___,___,___,-1 ,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*key    key*/ {key,key,___,___,___,___,___,___,-1 ,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*colon  col*/ {col,col,___,___,___,___,-2 ,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*value  val*/ {val,val,-6 ,___,-5 ,___,___,___,-1 ,___,___,___,mi_,___,ze0,inT,___,___,___,___,___,fe1,___,n01,___,___,tr1,___,___,___,___},
/*array  arr*/ {arr,arr,-6 ,___,-5 ,-7 ,___,___,-1 ,___,___,___,mi_,___,ze0,inT,___,___,___,___,___,fe1,___,n01,___,___,tr1,___,___,___,___},
/*string str*/ {str,___,str,str,str,str,str,str,-4 ,esp,str,str,str,str,str,str,str,str,str,str,str,str,str,str,str,str,str,str,str,str,str},
/*escape esp*/ {___,___,___,___,___,___,___,___,str,str,str,___,___,___,___,___,___,str,___,___,___,str,___,str,str,___,str,u1, ___,___,___},
/*u1     U1*/  {___,___,___,___,___,___,___,___,___,___,___,___,___,___,u2, u2, u2, u2, u2, u2, u2, u2, ___,___,___,___,___,___,u2, u2, ___},
/*u2     U2*/  {___,___,___,___,___,___,___,___,___,___,___,___,___,___,u3, u3, u3, u3, u3, u3, u3, u3, ___,___,___,___,___,___,u3, u3, ___},
/*u3     U3*/  {___,___,___,___,___,___,___,___,___,___,___,___,___,___,u4, u4, u4, u4, u4, u4, u4, u4, ___,___,___,___,___,___,u4, u4, ___},
/*u4     U4*/  {___,___,___,___,___,___,___,___,___,___,___,___,___,___,str,str,str,str,str,str,str,str,___,___,___,___,___,___,str,str,___},
/*minus  mi_*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,ze0,inT,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*zero   ze0*/ {ok ,ok ,___,-8 ,___,-7 ,___,-3 ,___,___,___,___,___,frt,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*int    inT*/ {ok ,ok ,___,-8 ,___,-7 ,___,-3 ,___,___,___,___,___,frt,inT,inT,___,___,___,___,ex1,___,___,___,___,___,___,___,___,ex1,___},
/*frac   frt*/ {ok ,ok ,___,-8 ,___,-7 ,___,-3 ,___,___,___,___,___,___,frt,frt,___,___,___,___,ex1,___,___,___,___,___,___,___,___,ex1,___},
/*e      ex1*/ {___,___,___,___,___,___,___,___,___,___,___,ex2,ex2,___,ex3,ex3,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*ex     ex2*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,ex3,ex3,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*exp    ex3*/ {ok ,ok ,___,-8 ,___,-7 ,___,-3 ,___,___,___,___,___,___,ex3,ex3,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*tr     tr1*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,tr2,___,___,___,___,___,___},
/*tru    tr2*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,tr3,___,___,___},
/*true   tr3*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,ok ,___,___,___,___,___,___,___,___,___,___},
/*fa     fe1*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,fe2,___,___,___,___,___,___,___,___,___,___,___,___,___,___},
/*fal    fe2*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,fe3,___,___,___,___,___,___,___,___},
/*fals   fe3*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,fe4,___,___,___,___,___},
/*false  fe4*/ {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,ok ,___,___,___,___,___,___,___,___,___,___},
/*nu     N1*/  {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,n02,___,___,___},
/*nul    N2*/  {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,n03,___,___,___,___,___,___,___,___},
/*null   N3*/  {___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,___,ok ,___,___,___,___,___,___,___,___},
		};
		return static_cast<State>(state_transition_table[current][input]) ;
	}
}

enum class Automaton::Mode
{
	array,
	done,
	key,
	object
};

Automaton::Automaton(std::size_t depth) :
	m_state(state::go),
	m_stack(1, Mode::done)
{
}

void Automaton::Push(Mode mode)
{
	m_stack.push_back(mode);
}

void Automaton::Pop(Mode mode)
{
	if (m_stack.back() != mode)
		throw -1;
	
	m_stack.pop_back();
}

void Automaton::Char(char c)
{
	chars::Type  next_char  = chars::Deduce(c);
	state::State next_state = state::Next(static_cast<state::State>(m_state), next_char);
	
	if (static_cast<int>(next_state) > 0)
		m_state = next_state;
	else
	{
		switch (static_cast<int>(next_state))
		{
			case -1:
				break;
		}
	}
}

} // end of namespace json
