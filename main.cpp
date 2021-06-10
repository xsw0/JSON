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
    try
    {
        cout << JSON::parse(input).to_string(-1) << endl;
    }
    catch (const JSON::ParseError& pe)
    {
        cout << pe.what() << endl;
    }
    return 0;
}
