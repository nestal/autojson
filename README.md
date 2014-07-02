autojson
========

A JSON parser base on the automaton provided by [json.org](http://www.json.org/JSON_checker/).

The library has the following features:

- Stream based parsing: you don't need to provide the whole JSON file in order to
parse it. The parser can parse the file progressively.
- Callback interface: the parser will invoke a callback function when it parse
a string/hash/array/etc from the JSON.
- Minimal copying: the parser will try not to copy the input data. It will only
copy input data when it can't parse a complete object from it.
- No dynamic allocation: the parser will not allocate memory dynamically when
parsing. It only allocate memory when set up. The parser object is designed to be
reusable after it parsed a JSON file.

Detailed documentation is in [github page](http://nestal.github.io/autojson/index.html)
