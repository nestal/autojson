/**
	\file	JSON_checker.h
	\brief	Main header of the C interface of autojson
	
	This file is adapted from http://www.json.org/JSON_checker/ with modification.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**	The type of JSON parse events.
	It is used to indicate what the parser has encountered. It is a parameter
	to the JSON_callback() function.
*/
typedef enum JSON_event
{
	JSON_array_start,	//!< Start of a JSON array
	JSON_array_end,		//!< End of a JSON array
	JSON_object_start,	//!< Start of a JSON object
	JSON_object_key,	//!< A key inside a JSON object.
						//!< The value of the key is given by \c data with
						//!< \c len bytes.
	JSON_object_end,	//!< End of a JSON object
	JSON_string,		//!< String as a value. The value is given by \c data
						//!< and \c len.
	JSON_number,		//!< Number as a value. It's provided as a string.
						//!< You need to call atoi() to obtain the real number.
	JSON_null,			//!< Literal value "null"
	JSON_true,			//!< Literal value "true"
	JSON_false			//!< Literal value "false"
} JSON_event;

/**	A callback function to be invoked when the parser encounter a event.
	See #JSON_event for a detailed list of events available. You need to
	provide this function when you call JSON_checker_char().
	
	\param	user	A user-provided pointer by JSON_checker_char(). The
					parser will not interpret its value. It will be pass
					as-is.
	\param	type	Type of event encountered.
	\param	data	Pointer to parsed data. It's only meaningful for
					certain events, such as ::JSON_string. Note that this
					buffer is not null-terminated. You need to refer to
					\c len to know how many bytes are there.
	\param	len		The number of bytes in the buffer pointed by \c data.
					Note that the unit is bytes, not characters, as unicode
					characters many need more than 1 byte to represent in
					UTF-8/UTF-16.
*/
typedef void (*JSON_callback)(void *user, JSON_event type, const char *data, int len);

typedef struct JSON_checker_struct {
    int state;
    int depth;
    int top;
    int* stack;

	char token[1024];
	int token_len;

} * JSON_checker;

/**	Create a new parser.
    new_JSON_checker() starts the checking process by constructing a JSON_checker
    object. It takes the \c depth parameter that restricts the level of maximum
    nesting.

    To continue the process, call JSON_checker_char() for each block of characters
    in the JSON text, and then call JSON_checker_done() to obtain the final result.
    These functions are fully reentrant.

    The JSON_checker object will be deleted by JSON_checker_done().
    JSON_checker_char() will auto delete the JSON_checker object if it sees an error.
*/
extern JSON_checker new_JSON_checker(int depth);

/**	Parse JSON data.
	Call this function to parse some JSON data. The data does not need to be a complete
	JSON file. It can be a fragment of the file. The parser will parse it progressively
	and invoke the callback function it is can successfully parse something.
	
	\param	jc		A parser created by new_JSON_checker()
	\param	chars	A block of JSON data to be parsed. It does not need to be
					a complete JSON document. JSON_checker_char() can parse
					incrementally.
	\param	len		Number of bytes in the buffer pointed by \c chars.
	\param	cb		A pointer to a callback function to be called when the parser
					encounter an event. See #JSON_event for more details about the events.
	\param	user	A pointer to user data. The parser will not try to interpret this
					pointer. It will be passed as-is to the callback function \c cb.
*/
extern int  JSON_checker_char(JSON_checker jc, const char *chars, int len, JSON_callback cb, void *user);
extern int  JSON_checker_done(JSON_checker jc);

#ifdef __cplusplus
}
#endif
