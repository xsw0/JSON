# JSON

A toy-level C++ json library

# to_string

Create json from istream.

```json
{
  "a": [
    123.456,
    123.456e3,
    "str abc"
  ],
  "b": {
    "null value": null,
    "true value": true,
    "false value": false,
    "str": "abcdefg\t\n\\\r"
  }
}
```

```c++
// create json from istream.
std::ifstream input{ "../input.json" };
JSON json;
try
{
json = JSON::parse(input);
}
catch (const JSON::ParseError& pe)
{
cout << pe.what() << endl;
}

// convert to string and output
cout << json.to_string() << endl;

// (optional) add indentation parameter
cout << json.to_string(-1) << endl;         // in one line
cout << json.to_string(0) << endl;
cout << json.to_string(2) << endl;
cout << json.to_string(4) << endl;          // 4 spaces indent

// create json by variable
JSON null_value{ nullptr };                 // null
JSON boolean{ true };                       // bool
JSON str{ "this is a string" };             // string
JSON num{ 123.456 };                        // number
JSON arr{ JSON::Array{}};                   // array
JSON obj{ JSON::Object{}};                  // object

// check hold type
cout << null_value.is<nullptr_t>();         //true
cout << null_value.is<bool>();              //false

// Convert to in-build type
null_value.as<nullptr_t>();                 // nullptr
num.as<double>(); //123.456
arr.as<JSON::Object>();

// Subscript access
cout << arr[0].to_string() << endl;
cout << obj["abc"].to_string() << endl;
```