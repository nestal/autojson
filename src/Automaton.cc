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

#include <iostream>

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
	Type DeduceType(char ch)
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
	enum Code {
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
		
		state_count,
		bad = state_count,
	};

	const char *code_str[] = {
		"go",
		"ok",
		"obj",
		"key",
		"col",
		"val",
		"arr",
		"str",
		"esp",
		"u1",
		"u2",
		"u3",
		"u4",
		"mi_",
		"ze0",
		"inT",
		"frt",
		"ex1",
		"ex2",
		"ex3",
		"tr1",
		"tr2",
		"tr3",
		"fe1",
		"fe2",
		"fe3",
		"fe4",
		"n01",
		"n02",
		"n03",
		
		"bad",
	};
	
	enum Action
	{
		none,	// no action required
		
		soj,	// start of object
		eoj,	// end of object
		noj,	// end of empty object
		sar,	// start of array
		ear,	// end of array
		stk,	// switch to key
		nxt,	// next element in array or object
		sos,	// start of string
		eos,	// end of string
	};
	
	class Edge
	{
	public:
		Edge(Code dest) : m_action(none), m_dest(dest) {}
		Edge(Action ac = none) : m_action(ac), m_dest(bad) {}
	
		enum Action Action() const { return m_action; }
		Code   Dest() const { return m_dest; }
	
	private:
		enum Action	m_action;
		enum Code	m_dest;
	};
	
	// bad state
	const Edge _____;
	
	Edge Next(Code current, chars::Type input)
	{
		static const Edge transition[state_count][chars::ctype_count] = {
//			   space  white   {     }     [     ]     :     ,     "     \     /     +     -     .     0    1-9    a     b     c     d     e     f     l     n     r     s     t     u   ABCDF   E    etc
/*start  go */ {{go} ,{go} ,{soj},_____,{sar},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*ok     ok */ {{ok} ,{ok} ,_____,{eoj},_____,{ear},_____,{nxt},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*object obj*/ {{obj},{obj},_____,{noj},_____,_____,_____,_____,{sos},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*key    key*/ {{key},{key},_____,_____,_____,_____,_____,_____,{sos},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*colon  col*/ {{col},{col},_____,_____,_____,_____,{stk},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*value  val*/ {{val},{val},{soj},_____,{sar},_____,_____,_____,{sos},_____,_____,_____,{mi_},_____,{ze0},{inT},_____,_____,_____,_____,_____,{fe1},_____,{n01},_____,_____,{tr1},_____,_____,_____,_____},
/*array  arr*/ {{arr},{arr},{soj},_____,{sar},{ear},_____,_____,{sos},_____,_____,_____,{mi_},_____,{ze0},{inT},_____,_____,_____,_____,_____,{fe1},_____,{n01},_____,_____,{tr1},_____,_____,_____,_____},
/*string str*/ {{str},_____,{str},{str},{str},{str},{str},{str},{eos},{esp},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str}},
/*escape esp*/ {_____,_____,_____,_____,_____,_____,_____,_____,{str},{str},{str},_____,_____,_____,_____,_____,_____,{str},_____,_____,_____,{str},_____,{str},{str},_____,{str},{u1} ,_____,_____,_____},
/*u1     U1*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,_____,_____,_____,_____,_____,_____,{u2} ,{u2} ,_____},
/*u2     U2*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,_____,_____,_____,_____,_____,_____,{u3} ,{u3} ,_____},
/*u3     U3*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,_____,_____,_____,_____,_____,_____,{u4} ,{u4} ,_____},
/*u4     U4*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{str},{str},{str},{str},{str},{str},{str},{str},_____,_____,_____,_____,_____,_____,{str},{str},_____},
/*minus  mi_*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ze0},{inT},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*zero   ze0*/ {{ok} ,{ok} ,_____,{eoj},_____,{ear},_____,{nxt},_____,_____,_____,_____,_____,{frt},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*int    inT*/ {{ok} ,{ok} ,_____,{eoj},_____,{ear},_____,{nxt},_____,_____,_____,_____,_____,{frt},{inT},{inT},_____,_____,_____,_____,{ex1},_____,_____,_____,_____,_____,_____,_____,_____,{ex1},_____},
/*frac   frt*/ {{ok} ,{ok} ,_____,{eoj},_____,{ear},_____,{nxt},_____,_____,_____,_____,_____,_____,{frt},{frt},_____,_____,_____,_____,{ex1},_____,_____,_____,_____,_____,_____,_____,_____,{ex1},_____},
/*e      ex1*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ex2},{ex2},_____,{ex3},{ex3},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*ex     ex2*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ex3},{ex3},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*exp    ex3*/ {{ok} ,{ok} ,_____,{eoj},_____,{ear},_____,{nxt},_____,_____,_____,_____,_____,_____,{ex3},{ex3},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*tr     tr1*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{tr2},_____,_____,_____,_____,_____,_____},
/*tru    tr2*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{tr3},_____,_____,_____},
/*true   tr3*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ok} ,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*fa     fe1*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{fe2},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*fal    fe2*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{fe3},_____,_____,_____,_____,_____,_____,_____,_____},
/*fals   fe3*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{fe4},_____,_____,_____,_____,_____},
/*false  fe4*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ok} ,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*nu     N1*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{n02},_____,_____,_____},
/*nul    N2*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{n03},_____,_____,_____,_____,_____,_____,_____,_____},
/*null   N3*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ok} ,_____,_____,_____,_____,_____,_____,_____,_____},
		};
		return transition[current][input] ;
	}
}

enum class Automaton::Mode
{
	array,
	done,
	key,
	object
};

std::ostream& operator<<(std::ostream& os, Automaton::Mode m)
{
	switch (m)
	{
		case Automaton::Mode::array: os << "array"; break;
		case Automaton::Mode::key: os << "key"; break;
		case Automaton::Mode::done: os << "done"; break;
		case Automaton::Mode::object: os << "object"; break;
	}
	return os;
}

Automaton::Automaton(std::size_t depth) :
	m_state(state::go),
	m_stack(1, Mode::done)
{
}

void Automaton::Push(Mode mode)
{
	std::cout << "pushing " << mode << std::endl;
	m_stack.push_back(mode);
}

void Automaton::Pop(Mode mode)
{
	std::cout << "popping " << mode << " " << m_stack.back() << std::endl;
	if (m_stack.back() != mode)
		throw -1;
	
	m_stack.pop_back();
}

void Automaton::Char(char c)
{
	state::Edge  next = state::Next(static_cast<state::Code>(m_state), chars::DeduceType(c));

	state::Code dest = next.Dest();
	
	std::cout << c << ": next action = " << (int)next.Action() << std::endl;
	std::cout << c << ": m_state = " << state::code_str[(int)m_state] << std::endl;
	
	switch (next.Action())
	{
		case state::soj:	Push(Mode::key);	dest = state::key;	break;
		case state::eoj:	Pop (Mode::object);	dest = state::ok;	break;
		case state::noj:	Pop (Mode::key);	dest = state::ok;	break;
		case state::sar:	Push(Mode::array);	dest = state::arr;	break;
		case state::ear:	Pop (Mode::array);	dest = state::ok;	break;
		case state::stk:	Pop (Mode::key);
							Push(Mode::object);	dest = state::val;	break;
		case state::nxt:
			if (m_stack.back() == Mode::object)
			{
				Pop(Mode::object);
				Push(Mode::key);
				dest = state::key;
			}
			else if (m_stack.back() == Mode::array)
				dest = state::arr;
			break;
		
		case state::sos:
			dest = state::str;
			break;
			
		case state::eos:
			switch (m_stack.back())
			{
				case Mode::object:
				case Mode::array:
					dest = state::ok;
					break;
					
				case Mode::key:
					dest = state::col;
					break;
				
				default:
					break;
			}
			break;
		
		// default should be none
		default:	break;
	}
	
	std::cout << c << ": next state = " << state::code_str[(int)dest] << std::endl;
	m_state = dest;
}

bool Automaton::Result() const
{
	return m_stack.size() == 1 && m_stack.back() == Mode::done;
}

} // end of namespace json
