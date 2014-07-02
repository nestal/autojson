/* JSON_checker.h */

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	JSON_object_end,
	
	JSON_type_none = INT_MAX
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


extern JSON_checker new_JSON_checker(int depth);
extern int  JSON_checker_char(JSON_checker jc, const char *chars, int len, JSON_callback cb, void *user);
extern int  JSON_checker_done(JSON_checker jc);

#ifdef __cplusplus
}
#endif
