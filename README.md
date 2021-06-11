# JSON

A toy-level C++ json library

# to_string

Create json from istream.

``` cpp
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
```

Convert to string and output

``` cpp
cout << json.to_string() << endl;
```

(optional) add indentation parameter

``` cpp
cout << json.to_string(-1) << endl; // in one line
cout << json.to_string(4) << endl; // 4 spaces indent
```

create json by variable

``` cpp
JSON null_value{nullptr}                        // null
JSON str{"this is a string"}                    // string
JSON num{123.456}                               // number
JSON arr{ vector<JSON>{}};                      // array
JSON arr{ unordered_map<string, JSON>{}};       // object
```

check hold type

```cpp
null_value.is<nullptr_t>() //true
null_value.is<bool>() //false
```

Convert to in-build type

```cpp
null_value.as<nullptr_t>() // nullptr
num.as<double>() //123.456
```

