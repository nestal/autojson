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

Getting Started with the C Interface
====================================

\code{.c}

\endcode

Getting Started with the C++ Interface
====================================

\code{.cpp}
struct Subject
{
	std::string value;
	int in;
	struct Sub
	{
		std::string v2;
	} sub;
};

Subject j {};
const char js[] =
"{"
	"\"haha\": \"fun\","
	"\"hehe\": 199"
"}";

JsonParser p(
	ObjectReactor<Subject>().
		Map("haha", &Subject::value).
		Map("hehe", &Subject::in),
	j);
p.Parse(js, sizeof(js)-1);
\endcode

In the above example, there are a few class involved:

- JsonParser: the parser class that controls the parsing.
- ObjectReactor: a Reactor class that maps the key in the object to
  a member variable.
- Destination: it's the `Subject` class here. It is the class that
  stores the output of the JSON parser.
  
