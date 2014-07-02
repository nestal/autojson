/* JSON_checker.c */

/* 2007-08-24 */

/*
Copyright (c) 2005 JSON.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

The Software shall be used for Good, not Evil.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "JSON_checker.h"

#define true  1
#define false 0
#define __   -1     /* the universal error code */

/*
    Characters are mapped into these 31 character classes. This allows for
    a significant reduction in the size of the state transition table.
*/
enum classes {
    C_SPACE,  /* space */
    C_WHITE,  /* other whitespace */
    C_LCURB,  /* {  */
    C_RCURB,  /* } */
    C_LSQRB,  /* [ */
    C_RSQRB,  /* ] */
    C_COLON,  /* : */
    C_COMMA,  /* , */
    C_QUOTE,  /* " */
    C_BACKS,  /* \ */
    C_SLASH,  /* / */
    C_PLUS,   /* + */
    C_MINUS,  /* - */
    C_POINT,  /* . */
    C_ZERO ,  /* 0 */
    C_DIGIT,  /* 123456789 */
    C_LOW_A,  /* a */
    C_LOW_B,  /* b */
    C_LOW_C,  /* c */
    C_LOW_D,  /* d */
    C_LOW_E,  /* e */
    C_LOW_F,  /* f */
    C_LOW_L,  /* l */
    C_LOW_N,  /* n */
    C_LOW_R,  /* r */
    C_LOW_S,  /* s */
    C_LOW_T,  /* t */
    C_LOW_U,  /* u */
    C_ABCDF,  /* ABCDF */
    C_E,      /* E */
    C_ETC,    /* everything else */
    NR_CLASSES
};

static int ascii_class[128] = {
/*
    This array maps the 128 ASCII characters into character classes.
    The remaining Unicode characters should be mapped to C_ETC.
    Non-whitespace control characters are errors.
*/
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      C_WHITE, C_WHITE, __,      __,      C_WHITE, __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,
    __,      __,      __,      __,      __,      __,      __,      __,

    C_SPACE, C_ETC,   C_QUOTE, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_PLUS,  C_COMMA, C_MINUS, C_POINT, C_SLASH,
    C_ZERO,  C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT, C_DIGIT,
    C_DIGIT, C_DIGIT, C_COLON, C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,

    C_ETC,   C_ABCDF, C_ABCDF, C_ABCDF, C_ABCDF, C_E,     C_ABCDF, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LSQRB, C_BACKS, C_RSQRB, C_ETC,   C_ETC,

    C_ETC,   C_LOW_A, C_LOW_B, C_LOW_C, C_LOW_D, C_LOW_E, C_LOW_F, C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_ETC,   C_LOW_L, C_ETC,   C_LOW_N, C_ETC,
    C_ETC,   C_ETC,   C_LOW_R, C_LOW_S, C_LOW_T, C_LOW_U, C_ETC,   C_ETC,
    C_ETC,   C_ETC,   C_ETC,   C_LCURB, C_ETC,   C_RCURB, C_ETC,   C_ETC
};


/*
    The state codes.
*/
enum states {
    GO,  /* start    */
    OK,  /* ok       */
    OB,  /* object   */
    KE,  /* key      */
    CO,  /* colon    */
    VA,  /* value    */
    AR,  /* array    */
    ST,  /* string   */
    ES,  /* escape   */
    U1,  /* u1       */
    U2,  /* u2       */
    U3,  /* u3       */
    U4,  /* u4       */
    MI,  /* minus    */
    ZE,  /* zero     */
    IN,  /* integer  */
    FR,  /* fraction */
    E1,  /* e        */
    E2,  /* ex       */
    E3,  /* exp      */
    T1,  /* tr       */
    T2,  /* tru      */
    T3,  /* true     */
    F1,  /* fa       */
    F2,  /* fal      */
    F3,  /* fals     */
    F4,  /* false    */
    N1,  /* nu       */
    N2,  /* nul      */
    N3,  /* null     */
    NR_STATES
};


static int state_transition_table[NR_STATES][NR_CLASSES] = {
/*
    The state transition table takes the current state and the current symbol,
    and returns either a new state or an action. An action is represented as a
    negative number. A JSON text is accepted if at the end of the text the
    state is OK and if the mode is MODE_DONE.

                 white                                      1-9                                   ABCDF  etc
             space |  {  }  [  ]  :  ,  "  \  /  +  -  .  0  |  a  b  c  d  e  f  l  n  r  s  t  u  |  E  |*/
/*start  GO*/ {GO,GO,-6,__,-5,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ok     OK*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*object OB*/ {OB,OB,__,-9,__,__,__,__,-1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*key    KE*/ {KE,KE,__,__,__,__,__,__,-1,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*colon  CO*/ {CO,CO,__,__,__,__,-2,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*value  VA*/ {VA,VA,-6,__,-5,__,__,__,-1,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*array  AR*/ {AR,AR,-6,__,-5,-7,__,__,-1,__,__,__,MI,__,ZE,IN,__,__,__,__,__,F1,__,N1,__,__,T1,__,__,__,__},
/*string ST*/ {ST,__,ST,ST,ST,ST,ST,ST,-4,ES,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST,ST},
/*escape ES*/ {__,__,__,__,__,__,__,__,ST,ST,ST,__,__,__,__,__,__,ST,__,__,__,ST,__,ST,ST,__,ST,U1,__,__,__},
/*u1     U1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U2,U2,U2,U2,U2,U2,U2,U2,__,__,__,__,__,__,U2,U2,__},
/*u2     U2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U3,U3,U3,U3,U3,U3,U3,U3,__,__,__,__,__,__,U3,U3,__},
/*u3     U3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,U4,U4,U4,U4,U4,U4,U4,U4,__,__,__,__,__,__,U4,U4,__},
/*u4     U4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ST,ST,ST,ST,ST,ST,ST,ST,__,__,__,__,__,__,ST,ST,__},
/*minus  MI*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,ZE,IN,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*zero   ZE*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*int    IN*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,FR,IN,IN,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*frac   FR*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,FR,FR,__,__,__,__,E1,__,__,__,__,__,__,__,__,E1,__},
/*e      E1*/ {__,__,__,__,__,__,__,__,__,__,__,E2,E2,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*ex     E2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*exp    E3*/ {OK,OK,__,-8,__,-7,__,-3,__,__,__,__,__,__,E3,E3,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*tr     T1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T2,__,__,__,__,__,__},
/*tru    T2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,T3,__,__,__},
/*true   T3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*fa     F1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F2,__,__,__,__,__,__,__,__,__,__,__,__,__,__},
/*fal    F2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F3,__,__,__,__,__,__,__,__},
/*fals   F3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,F4,__,__,__,__,__},
/*false  F4*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__,__,__},
/*nu     N1*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N2,__,__,__},
/*nul    N2*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,N3,__,__,__,__,__,__,__,__},
/*null   N3*/ {__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,__,OK,__,__,__,__,__,__,__,__},
};


/*
    These modes can be pushed on the stack.
*/
enum modes {
    MODE_ARRAY, 
    MODE_DONE,  
    MODE_KEY,   
    MODE_OBJECT,
};

static int
reject(JSON_checker jc)
{
/*
    Delete the JSON_checker object.
*/
    free((void*)jc->stack);
    free((void*)jc);
    return false;
}


static int
push(JSON_checker jc, int mode)
{
/*
    Push a mode onto the stack. Return false if there is overflow.
*/
    jc->top += 1;
    if (jc->top >= jc->depth) {
        return false;
    }
    jc->stack[jc->top] = mode;
    return true;
}


static int
pop(JSON_checker jc, int mode)
{
/*
    Pop the stack, assuring that the current mode matches the expectation.
    Return false if there is underflow or if the modes mismatch.
*/
    if (jc->top < 0 || jc->stack[jc->top] != mode) {
        return false;
    }
    jc->top -= 1;
    return true;
}

JSON_checker
new_JSON_checker(int depth)
{
    JSON_checker jc = (JSON_checker)malloc(sizeof(struct JSON_checker_struct));
    jc->state = GO;
    jc->depth = depth;
    jc->top = -1;
    jc->stack = (int*)calloc(depth, sizeof(int));
	jc->token_len = 0;
    push(jc, MODE_DONE);
    return jc;
}

typedef struct JSON_token_
{
	const char		*start;
	JSON_callback	cb;
	void			*user;
} JSON_token;

static void emit_token(JSON_checker jc, JSON_token *token, const char *pos, JSON_event type)
{
	int len = pos - token->start;
	if (jc->token_len > 0 && sizeof(jc->token) - jc->token_len > len )
	{
		assert(token->start != 0);
		memcpy(jc->token + jc->token_len, token->start, len);
		(token->cb)(token->user, type, jc->token, jc->token_len + len);
	}
	else
		(token->cb)(token->user, type, len == 0 ? 0 : token->start, len);

	// reset
	jc->token_len = 0;
	token->start = 0;
}

static void emit_permitive(JSON_checker jc, JSON_token *token, JSON_event type)
{
	jc->token_len	= 0;
	emit_token(jc, token, 0, type);
}

static int is_number(int state)
{
	return state >= MI && state <= E3;
}

static void save_char(JSON_checker jc, const char *ptr, JSON_token *token, int next_class, int next_state)
{
	// case                next_char         jc->state          next_state        action
	// start of string     "                 !ST                ST                not save
	// inside a string     a-z0-9 etc        ST                 ST                save char
	// end of string       "                 ST                 -4                not save, consume string (see case -4:)
	if (next_state  >= ST && next_state <= E3)
	{
		if (token->start == 0)
			token->start = ptr;
	}
}

static void detect_state(JSON_checker jc, const char *pos, int next_state, JSON_token *token)
{
	// state from number to non-number
	if ( (is_number(jc->state) && !is_number(next_state)))
		emit_token(jc, token, pos, JSON_number);
	
	else if (jc->state == N3 && next_state == OK)
		emit_permitive(jc, token, JSON_null);
	
	else if (jc->state == T3 && next_state == OK)
		emit_permitive(jc, token, JSON_true);
	
	else if (jc->state == F4 && next_state == OK)
		emit_permitive(jc, token, JSON_false);
}

static int save_token(JSON_checker jc, JSON_token *token, const char *pos)
{
	// save the rest of the token string in the parser state
	if (token->start != 0)
	{
		char *p = &jc->token[jc->token_len];
		int len = pos - token->start;
		if (sizeof(jc->token) > len)
		{
			memcpy(p, token->start, len);
			jc->token_len += len;
		}
		else
			reject(jc);
	}
	return true;
}

int
JSON_checker_char(JSON_checker jc, const char *chars, int len, JSON_callback cb, void *user)
{
	JSON_token token = {0, cb, user};
	int i;
	
	for (i = 0; i < len; ++i)
	{
	/*
		After calling new_JSON_checker, call this function for each character (or
		partial character) in your JSON text. It can accept UTF-8, UTF-16, or
		UTF-32. It returns true if things are looking ok so far. If it rejects the
		text, it deletes the JSON_checker object and returns false.
	*/
		int next_char = chars[i];
		int next_class, next_state;
	/*
		Determine the character's class.
	*/
		if (next_char < 0) {
			return reject(jc);
		}
		if (next_char >= 128) {
			next_class = C_ETC;
		} else {
			next_class = ascii_class[next_char];
			if (next_class <= __) {
				return reject(jc);
			}
		}
	/*
		Get the next state from the state transition table.
	*/
		next_state = state_transition_table[jc->state][next_class];

		// check if need to save the character for consuming and any state change
		save_char(jc, &chars[i], &token, next_class, next_state);
		detect_state(jc, &chars[i], next_state, &token);

		if (next_state >= 0) {
	/*
		Change the state.
	*/
			jc->state = next_state;
		} else {
	/*
		Or perform one of the actions.
	*/
			switch (next_state) {
	/* empty } */
			case -9:
				emit_permitive(jc, &token, JSON_object_end);
				if (!pop(jc, MODE_KEY)) {
					return reject(jc);
				}
				jc->state = OK;
				break;

	/* } */ case -8:
				emit_permitive(jc, &token, JSON_object_end);
				if (!pop(jc, MODE_OBJECT)) {
					return reject(jc);
				}
				jc->state = OK;
				break;

	/* ] */ case -7:
				emit_permitive(jc, &token, JSON_array_end);
				if (!pop(jc, MODE_ARRAY)) {
					return reject(jc);
				}
				jc->state = OK;
				break;

	/* { */ case -6:
				emit_permitive(jc, &token, JSON_object_start);
				if (!push(jc, MODE_KEY)) {
					return reject(jc);
				}
				jc->state = OB;
				break;

	/* [ */ case -5:
				emit_permitive(jc, &token, JSON_array_start);
				if (!push(jc, MODE_ARRAY)) {
					return reject(jc);
				}
				jc->state = AR;
				break;

	/* " */ case -4:
				switch (jc->stack[jc->top]) {
				case MODE_KEY:
					jc->state = CO;
					emit_token(jc, &token, &chars[i], JSON_object_key);
					break;
				case MODE_ARRAY:
				case MODE_OBJECT:
					jc->state = OK;
					emit_token(jc, &token, &chars[i], JSON_string);
					break;
				default:
					return reject(jc);
				}
				break;

	/* , */ case -3:
				switch (jc->stack[jc->top]) {
				case MODE_OBJECT:
	/*
		A comma causes a flip from object mode to key mode.
	*/
					if (!pop(jc, MODE_OBJECT) || !push(jc, MODE_KEY)) {
						return reject(jc);
					}
					jc->state = KE;
					break;
				case MODE_ARRAY:
					jc->state = VA;
					break;
				default:
					return reject(jc);
				}
				break;

	/* : */ case -2:
	/*
		A colon causes a flip from key mode to object mode.
	*/
				if (!pop(jc, MODE_KEY) || !push(jc, MODE_OBJECT)) {
					return reject(jc);
				}
				jc->state = VA;
				break;

			case -1:
				jc->state = ST;
				break;
	/*
		Bad action.
	*/
			default:
				return reject(jc);
			}
		}
	}
	
	save_token(jc, &token, &chars[i]);
	
    return true;
}


int
JSON_checker_done(JSON_checker jc)
{
/*
    The JSON_checker_done function should be called after all of the characters
    have been processed, but only if every call to JSON_checker_char returned
    true. This function deletes the JSON_checker and returns true if the JSON
    text was accepted.
*/
    int result = jc->state == OK && pop(jc, MODE_DONE);
    reject(jc);
    return result;
}
