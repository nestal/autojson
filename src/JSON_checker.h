/**
	\file	JSON_checker.h
	\brief	Main header of the C interface of autojson
	
	This file is adapted from http://www.json.org/JSON_checker/ with modification.
*/

#ifdef __cplusplus
extern "C" {
#endif

/**	something
*/
typedef enum JSON_type_
{
	JSON_array_start,
	JSON_array_end,
	JSON_object_start,
	JSON_object_key,	// string as object key
	JSON_string,		// string as object
	JSON_number,
	JSON_null,
	JSON_true,
	JSON_false,
	JSON_object_end
} JSON_type;

typedef void (*JSON_callback)(void *user, JSON_type type, const char *data, int len);

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
    object. It takes the \a depth parameter that restricts the level of maximum
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
*/
extern int  JSON_checker_char(JSON_checker jc, const char *chars, int len, JSON_callback cb, void *user);
extern int  JSON_checker_done(JSON_checker jc);

#ifdef __cplusplus
}
#endif
