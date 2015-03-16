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
#include "Exception.hh"

#include <vector>

#include <cassert>
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
			throw InvalidChar(ch);
		
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
} ;

namespace action
{
	enum Code
	{
		none,	// no action required
		
		soj,	// start of object
		eoj,	// end of object
		noj,	// end of empty object
		sar,	// start of array
		ear,	// end of array
		ktv,	// key to value
		nxt,	// next element in array or object
		sos,	// start of string
		eos,	// end of string
	};
}

class Automaton::Impl
{
public:
	Impl(Callback&& callback)
	:	m_state(state::go),
		m_stack(1, Mode::done),
		m_callback(std::move(callback)),
		m_token(nullptr)
	{
	}	

	void Parse(const char *str, std::size_t len);
	bool Result() const;
	
private:
	enum class Mode
	{
		array,
		done,
		key,
		object
	};
	
	void Push(Mode mode)
	{
		m_stack.push_back(mode);
	}

	void Pop(Mode mode)
	{
		if (m_stack.back() != mode)
			throw -1;
		
		m_stack.pop_back();
	}

	void OnNone(const char *)
	{
	}
	
	void OnStartObject(const char *)
	{
		Push(Mode::key);
		m_callback(Event::object_start, nullptr, 0);
	}
	
	void OnEndObject(const char *)
	{
		Pop(Mode::object);
		m_callback(Event::object_end, nullptr, 0);
	}
	
	void OnEndEmptyObject(const char *)
	{
		Pop (Mode::key);
		m_callback(Event::object_end, nullptr, 0);
	}
	
	void OnStartArray(const char *)
	{
		Push(Mode::array);
		m_callback(Event::array_start, nullptr, 0);
	}
	
	void OnEndArray(const char *)
	{
		Pop (Mode::array);
		m_callback(Event::array_end, nullptr, 0);
	}
	
	void OnKeyToValue(const char *)
	{
		Pop (Mode::key);
		Push(Mode::object);
	}
	
	void OnNextValue(const char *)
	{
		if (m_stack.back() == Mode::object)
		{
			Pop(Mode::object);
			Push(Mode::key);
		}
	}
	void OnStartString(const char *p)
	{
		if (m_stack.back() == Mode::key)
			m_callback(Event::object_key, nullptr, 0);
	
		m_callback(Event::string_start, nullptr, 0);
		
		// save pointer to the start of the string
		// it points to the double quote character
		// so it needs to be adjusted in OnEndString()
		assert(m_token == nullptr);
		m_token = p;
	}
	
	void OnEndString(const char *p)
	{
		// m_token points to the double quote character
		// so it needs to be bumped
		m_token++;
		
		m_callback(Event::string_data, m_token, p-m_token);
		m_callback(Event::string_end, nullptr, 0);
		
		// reset token pointer for next use
		m_token = nullptr;
	}
	
	class Edge
	{
	public:
		Edge(state::Code dest) : m_action(action::none), m_dest(dest) {}
		Edge(action::Code ac = action::none) : m_action(ac), m_dest(state::bad) {}

		action::Code Action() const { return m_action; }
		
		// sometimes, the next state will depend on the current mode
		state::Code  Dest(Mode mode) const
		{
			using namespace action;
			using namespace state;
			switch (m_action)
			{
				case none:	return m_dest;
				case soj:	return key;
				case eoj:	return ok;
				case noj:	return ok;
				case sar:	return arr;
				case ear:	return ok;
				case ktv:	return val;
				case nxt:	return mode == Mode::object ? key : arr;
				case sos:	return str;
				case eos:	return mode == Mode::key    ? col : ok;
				default:	throw ParseError(0,0);
			}
		}

		static Edge Next(state::Code current, chars::Type input);
		
	private:
		action::Code	m_action;
		state::Code		m_dest;
	};
	
	friend std::ostream& operator<<(std::ostream& os, Mode m)
	{
		switch (m)
		{
			case Automaton::Impl::Mode::array:	os << "array"; break;
			case Automaton::Impl::Mode::key:	os << "key"; break;
			case Automaton::Impl::Mode::done:	os << "done"; break;
			case Automaton::Impl::Mode::object:	os << "object"; break;
		}
		return os;
	}
	
	void EndObjObject();
	
private :
	state::Code			m_state;
	const char			*m_token;
	
	std::vector<Mode>	m_stack;
	const Callback		m_callback;
};


Automaton::Impl::Edge Automaton::Impl::Edge::Next(state::Code current, chars::Type input)
{
	using namespace action;
	using namespace state;
	
	// bad state
	static const Edge _____;
	
	static const Edge transition[state_count][chars::ctype_count] = {
//			   space  white   {     }     [     ]     :     ,     "     \     /     +     -     .     0    1-9    a     b     c     d     e     f     l     n     r     s     t     u   ABCDF   E    etc
/*start  go */ {{go} ,{go} ,{soj},_____,{sar},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*ok     ok */ {{ok} ,{ok} ,_____,{eoj},_____,{ear},_____,{nxt},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*object obj*/ {{obj},{obj},_____,{noj},_____,_____,_____,_____,{sos},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*key    key*/ {{key},{key},_____,_____,_____,_____,_____,_____,{sos},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*colon  col*/ {{col},{col},_____,_____,_____,_____,{ktv},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
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

void Automaton::Impl::Parse(const char *str, std::size_t len)
{
	for (std::size_t i = 0 ; i < len ; i++)
	{
		Edge next = Edge::Next(m_state, chars::DeduceType(str[i]));
		
		static void (Impl::*actions[])(const char*) =
		{
			&Impl::OnNone,
			&Impl::OnStartObject,
			&Impl::OnEndObject,
			&Impl::OnEndEmptyObject,
			&Impl::OnStartArray,
			&Impl::OnEndArray,
			&Impl::OnKeyToValue,
			&Impl::OnNextValue,
			&Impl::OnStartString,
			&Impl::OnEndString,
		};

		assert(next.Action() < sizeof(actions)/sizeof(actions[0]));
		(this->*actions[next.Action()])(&str[i]);
		
		m_state = next.Dest(m_stack.back());
		if (m_state == state::bad)
			throw -1;
	}
}

bool Automaton::Impl::Result() const
{
	return m_stack.size() == 1 && m_stack.back() == Mode::done;
}

Automaton::Automaton(Callback&& callback, std::size_t depth) :
	m_impl(new Impl(std::move(callback)))
{
}

Automaton::~Automaton() = default ;

void Automaton::Parse(const char *str, std::size_t len)
{
	m_impl->Parse(str, len);
}

bool Automaton::Result() const
{
	return m_impl->Result();
}

std::ostream& operator<<(std::ostream& os, Event ev)
{
	switch (ev)
	{
		case Event::array_start:	os << "array_start"; break;
		case Event::array_end:		os << "array_end"; break;
		case Event::object_start:	os << "object_start"; break;
		case Event::object_key:		os << "object_key"; break;
		case Event::object_end:		os << "object_end"; break;
		case Event::string_start:	os << "string_start"; break;
		case Event::string_data:	os << "string_data"; break;
		case Event::string_end:		os << "string_end"; break;
		case Event::number:			os << "number"; break;
		case Event::null_:			os << "null"; break;
		case Event::true_:			os << "true"; break;
		case Event::false_:			os << "false"; break;
	}
	return os;
}


} // end of namespace json
