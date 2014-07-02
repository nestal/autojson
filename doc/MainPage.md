\mainpage autojson Main Page

autojson is a JSON parser base on the automaton provided by json.org

Two interfaces are provided: C and C++. The C interface is a simple
and basic callback interface: client code call JSON_checker_char()
to input JSON data, the parser will call the callback function
provided as argument of JSON_checker_char() when it encounters any
events. Events are described by its type #JSON_event.

The C interface is provided by the JSON_checker.h header. The main
functions to be used are JSON_checker_char() and new_JSON_checker().

The C++ interface is still a work in progress.

autojson is hosted in [Github](https://github.com/nestal/autojson)
