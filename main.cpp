#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "JSON.h"

using std::cin;
using std::cout;
using std::cerr;
using std::flush;
using std::endl;

using namespace std;

int main()
{
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
    cout << json.is<JSON::Object>() << endl;
    cout << json["b"].to_string(4) << endl;
    cout << json["a"][0].size() << endl;

    return 0;
}
