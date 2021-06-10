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
    cout << json.to_string(-1) << endl;
    cout << json.to_string(0) << endl;
    cout << json.to_string(2) << endl;
    cout << json.to_string(4) << endl;
    return 0;
}
