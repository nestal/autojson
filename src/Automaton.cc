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

#include "EmitData.hh"
#include "Exception.hh"
#include "Range.hh"

#include <vector>

#include <algorithm>
#include <cassert>
#include <cstdint>
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
		
		// prevent sign extension
		std::uint8_t uch = static_cast<std::uint8_t>(ch);
		
		return uch >= sizeof(ascii)/sizeof(ascii[0]) ? etc : ascii[uch];
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
		
		bad,
		state_count = bad,
	};

	bool IsNumber(Code c)
	{
		return c >= mi_ && c <= ex3;
	}
	
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
	
	std::ostream& operator<<(std::ostream& os, Code state)
	{
		return os << code_str[state] ;
	}
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
		sep,	// start of escape sequence
		eep,	// end of escape sequence
		
		son,	// start of number
		eon,	// end of number
		enj,	// end of number and object
		ena,	// end of number and array
		enx,	// end of number and next element in array or object
		
		nul,	// emit null
		tru,	// boolean true
		fls,	// boolean false
	};
}

class Automaton::Impl
{
public:
	Impl(Callback&& callback)
	:	m_state(state::go),
		m_line(0),
		m_column(0),
		m_stack(1, Mode::done),
		m_callback(std::move(callback))
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
		object,
		escape
	};

	template <typename Expt>
	void Throw()
	{
		throw Expt() << LineNumInfo(m_line) << ColumnNumInfo(m_column);
	}
	
	void Push(Mode mode)
	{
		m_stack.push_back(mode);
	}

	void Pop(Mode mode)
	{
		if (m_stack.back() != mode)
			Throw<ParseError>();
		
		m_stack.pop_back();
	}

	void UpdateLineNumber(char ch)
	{
		m_column++;
		if (ch == '\n')
		{
			m_line++;
			m_column = 0;
		}
	}
	
	void OnNone(const char *)
	{
	}
	
	void OnStartObject(const char *)
	{
		Push(Mode::key);
		m_callback(Event::start, DataType::object, nullptr, 0);
	}
	
	void OnEndObject(const char *)
	{
		Pop(Mode::object);
		m_callback(Event::end, DataType::object, nullptr, 0);
	}
	
	void OnEndEmptyObject(const char *)
	{
		Pop (Mode::key);
		m_callback(Event::end, DataType::object, nullptr, 0);
	}
	
	void OnStartNumber(const char *p)
	{
		m_token.Save(p);
		m_callback(Event::start, DataType::number, nullptr, 0);
	}
	
	void OnEndNumber(const char *p)
	{
		assert(m_token.IsSaved());
		EmitData::Buf buf = m_token.Get(p);
		m_callback(Event::data, DataType::number, buf.begin(), buf.size());
		m_callback(Event::end, DataType::number, nullptr, 0);
	
		// reset token pointer for next use
		m_token.Clear();
	}
	
	void OnEndNumberAndObject(const char *p)
	{
		OnEndNumber(p);
		OnEndObject(p);
	}
	
	void OnEndNumberAndArray(const char *p)
	{
		OnEndNumber(p);
		OnEndArray(p);
	}
	
	void OnEndNumberAndNextValue(const char *p)
	{
		OnEndNumber(p);
		OnNextValue(p);
	}
	
	void OnStartArray(const char *)
	{
		Push(Mode::array);
		m_callback(Event::start, DataType::array, nullptr, 0);
	}
	
	void OnEndArray(const char *)
	{
		Pop (Mode::array);
		m_callback(Event::end, DataType::array, nullptr, 0);
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
	
	DataType Current() const
	{
		assert(!m_stack.empty());
		return m_stack.back() == Mode::key ? DataType::key : DataType::string;
	}

	///	\pre (m_token,p) denotes the string captured
	void EmitString(const char *p)
	{
		// m_token points to the double quote character
		// so it needs to be bumped
		assert(m_token.IsSaved());
		EmitData::Buf buf = m_token.Get(p);
		if (buf.size() > 1)
			m_callback(Event::data, Current(), buf.begin()+1, buf.size()-1);
	
		// reset token pointer for next use
		m_token.Clear();
	}
	
	void OnStartString(const char *p)
	{
		m_callback(Event::start, Current(), nullptr, 0);
	
		// save pointer to the start of the string
		// it points to the double quote character
		// so it needs to be adjusted in EmitString()
		assert(!m_token.IsSaved());
		m_token.Save(p);
	}
	
	void OnEndString(const char *p)
	{
		EmitString(p);
		m_callback(Event::end, Current(), nullptr, 0);
	}
	
	void OnStartEscape(const char *p)
	{
		EmitString(p);

		// similarly, points to the \ character
		assert(*p == '\\');
		assert(!m_token.IsSaved());
		m_token.Save(p);
	}
	
	void OnEndEscape(const char *p)
	{
		assert(p);
		assert(m_token.IsSaved());

		static const char out[]	= "\"\\/\b\f\n\r\t";
		static const char in[]	= "\"\\/bfnrt";

		auto pos = std::find(std::begin(in), std::end(in), *p);
		if (pos != std::end(in))
			m_callback(Event::data, Current(), &out[pos - in], sizeof(out[pos - in]));
		else
			Throw<InvalidChar>();

		m_token.Clear();
		m_token.Save(p);
	}
	
	void OnNull(const char*)
	{
		m_callback(Event::data, DataType::null_value, nullptr, 0);
	}

	void OnTrue(const char*)
	{
		m_callback(Event::data, DataType::boolean_true, nullptr, 0);
	}
	
	void OnFalse(const char*)
	{
		m_callback(Event::data, DataType::boolean_false, nullptr, 0);
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
				
				// start of object and array
				case soj:	return obj;
				case sar:	return arr;
				
				// end of object
				case enj:
				case eoj:
				case noj:	return ok;
				
				// end of array
				case ena:
				case ear:	return ok;
				
				// next element
				case enx:
				case nxt:	return (mode == Mode::object) ? key : arr;
				
				case ktv:	return val;
				case sos:	return str;
				case eos:	return (mode == Mode::key)    ? col : ok;
				case sep:	return esp;
				case eep:	return str;
				case son:	return m_dest;
				case eon:	return ok;
				
				case tru:
				case fls:
				case nul:	return ok;
				
				default:	assert(false);
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
			case Mode::array:	os << "array"; break;
			case Mode::key:		os << "key"; break;
			case Mode::done:	os << "done"; break;
			case Mode::object:	os << "object"; break;
			case Mode::escape:	os << "escape"; break;
		}
		return os;
	}
	
	action::Code NextAction(state::Code current, chars::Type input);
	
private :
	state::Code			m_state;
	EmitData			m_token;

	std::size_t			m_line;
	std::size_t			m_column;
	
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
/*string str*/ {{str},_____,{str},{str},{str},{str},{str},{str},{eos},{sep},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str},{str}},
/*escape esp*/ {_____,_____,_____,_____,_____,_____,_____,_____,{eep},{eep},{eep},_____,_____,_____,_____,_____,_____,{eep},_____,_____,_____,{eep},_____,{eep},{eep},_____,{eep},{u1} ,_____,_____,_____},
/*u1     U1*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,{u2} ,_____,_____,_____,_____,_____,_____,{u2} ,{u2} ,_____},
/*u2     U2*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,{u3} ,_____,_____,_____,_____,_____,_____,{u3} ,{u3} ,_____},
/*u3     U3*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,{u4} ,_____,_____,_____,_____,_____,_____,{u4} ,{u4} ,_____},
/*u4     U4*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{eep},{eep},{eep},{eep},{eep},{eep},{eep},{eep},_____,_____,_____,_____,_____,_____,{eep},{eep},_____},
/*minus  mi_*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ze0},{inT},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*zero   ze0*/ {{eon},{eon},_____,{enj},_____,{ena},_____,{enx},_____,_____,_____,_____,_____,{frt},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*int    inT*/ {{eon},{eon},_____,{enj},_____,{ena},_____,{enx},_____,_____,_____,_____,_____,{frt},{inT},{inT},_____,_____,_____,_____,{ex1},_____,_____,_____,_____,_____,_____,_____,_____,{ex1},_____},
/*frac   frt*/ {{eon},{eon},_____,{enj},_____,{ena},_____,{enx},_____,_____,_____,_____,_____,_____,{frt},{frt},_____,_____,_____,_____,{ex1},_____,_____,_____,_____,_____,_____,_____,_____,{ex1},_____},
/*e      ex1*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ex2},{ex2},_____,{ex3},{ex3},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*ex     ex2*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{ex3},{ex3},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*exp    ex3*/ {{eon},{eon},_____,{enj},_____,{ena},_____,{enx},_____,_____,_____,_____,_____,_____,{ex3},{ex3},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*tr     tr1*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{tr2},_____,_____,_____,_____,_____,_____},
/*tru    tr2*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{tr3},_____,_____,_____},
/*true   tr3*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{tru},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*fa     fe1*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{fe2},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*fal    fe2*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{fe3},_____,_____,_____,_____,_____,_____,_____,_____},
/*fals   fe3*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{fe4},_____,_____,_____,_____,_____},
/*false  fe4*/ {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{fls},_____,_____,_____,_____,_____,_____,_____,_____,_____,_____},
/*nu     N1*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{n02},_____,_____,_____},
/*nul    N2*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{n03},_____,_____,_____,_____,_____,_____,_____,_____},
/*null   N3*/  {_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,_____,{nul},_____,_____,_____,_____,_____,_____,_____,_____},
	};

	Edge result = transition[current][input];
	
	// detect actionss start and end of numbers
	if (!state::IsNumber(current) && state::IsNumber(result.m_dest))
	{
		assert(result.m_action == action::none);
		result.m_action = son;
	}
/*	else if (state::IsNumber(current) && !state::IsNumber(result.m_dest))
		result.m_action = eon;
*/	
	return result ;
}

void Automaton::Impl::Parse(const char *str, std::size_t len)
{
	assert(str != nullptr);
	assert(len > 0);
	assert(!m_token.IsSaved());
	
	if (m_token.IsStashed())
		m_token.Save(str);
	
	for (std::size_t i = 0 ; i < len ; i++)
	{
		UpdateLineNumber(str[i]);
		
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
			&Impl::OnStartEscape,
			&Impl::OnEndEscape,
			&Impl::OnStartNumber,
			&Impl::OnEndNumber,
			&Impl::OnEndNumberAndObject,
			&Impl::OnEndNumberAndArray,
			&Impl::OnEndNumberAndNextValue,
			&Impl::OnNull,
			&Impl::OnTrue,
			&Impl::OnFalse
		};

		assert(next.Action() < sizeof(actions)/sizeof(actions[0]));
		(this->*actions[next.Action()])(&str[i]);
		
		state::Code nstate = next.Dest(m_stack.back());
		if (nstate == state::bad)
		{
			std::cout << m_state << "->" << nstate << " \"" << str[i] << "\" (" << static_cast<int>(str[i]) << ")" << std::endl;
			Throw<ParseError>();
		}
		m_state = nstate;
	}
	
	// if we saved a token, stash it for later use because we will have a new
	// buffer the next time Parse() is called.
	if (m_token.IsSaved())
		m_token.Stash(str+len);
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
		case Event::start:	os << "start"; break;
		case Event::end:	os << "end"; break;
		case Event::data:	os << "data"; break;
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, DataType type)
{
	switch (type)
	{
		case DataType::array:	os << "array"; break;
		case DataType::object:	os << "object"; break;
		case DataType::key:		os << "key"; break;
		case DataType::string:	os << "string"; break;
		case DataType::boolean_true:	os << "true"; break;
		case DataType::boolean_false:	os << "false"; break;
		case DataType::number:		os << "number"; break;
		case DataType::null_value:	os << "null"; break;
	}
	return os;
}

} // end of namespace json
