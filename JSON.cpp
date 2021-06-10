//
// Created by 0x0 on 6/10/21.
//

#include "JSON.h"

#include <sstream>

class JSON::Parser
{
public:
    std::istream& is;
    size_t line = 1;
    size_t column = 1;
    JSON json;

    void whitespace();

    bool match(char c);
    auto get();

    JSON value();

    std::string string();
    std::vector<JSON> array();
    Object object();
    JSON checkKeyword(std::string_view key);
    char digit();
    char digitExceptZero();
    std::string fraction();
    std::string digits();
    JSON number();

    [[nodiscard]] ParseError error(const std::string& message) const
    {
        return ParseError{ line, column, message };
    }

    explicit Parser(std::istream& is) : is{ is }, json{ value() }
    {
        if (is.good()) throw error("Redundant content");
    }
    std::string exponent();
};

auto JSON::Parser::get()
{
    auto c = is.get();
    switch (c)
    {
    case '\n':
        ++line;
        column = 1;
        break;
    case '\r':
        if (is.peek() != '\n')
        {
            ++line;
            column = 1;
        }
        else
        {
            ++column;
        }
        break;
    default:
        ++column;
        break;
    }
    return c;
}

bool JSON::Parser::match(char c)
{
    if (is.peek() == c)
    {
        get();
        return true;
    }
    else
    {
        return false;
    }
}

JSON JSON::Parser::value()
{
    if (!is.good()) throw error("need a valid json value.");
    JSON result;
    whitespace();
    auto c = is.peek();
    switch (c)
    {
    case 'n':
        result = checkKeyword("null");
        break;
    case 't':
        result = checkKeyword("true");
        break;
    case 'f':
        result = checkKeyword("false");
        break;
    case '"':
        result = JSON{ string() };
        break;
    case '{':
        result = JSON{ object() };
        break;
    case '[':
        result = JSON{ array() };
        break;
    default:
        result = number();
    }
    whitespace();
    return result;
}

void JSON::Parser::whitespace()
{
    while (true)
    {
        switch (is.peek())
        {
        case ' ':
        case '\n':
        case '\r':
        case '\t':
            get();
            break;
        default:
            return;
        }
    }
}

JSON JSON::Parser::checkKeyword(std::string_view key)
{
    const std::map<std::string_view, JSON> keywords{
        { "null",  JSON(nullptr) },
        { "true",  JSON(true) },
        { "false", JSON(false) },
    };

    auto it = keywords.find(key);
    if (it != keywords.cend())
    {
        for (auto c : key)
        {
            if (!match(c))
                throw error("checkKeyword input error, do you mean '" + std::string(key) + "'?");
        }
        return it->second;
    }
    else
    {
        throw error("source code error, not define '" + std::string(key) + "' checkKeyword");
    }
}

std::string JSON::Parser::string()
{
    if (!match('"')) throw error("need a '\"' before a string");
    std::string result;
    while (is.good())
    {
        auto c = get();
        switch (c)
        {
        case '\r':
        case '\n':
            throw error(
                "Newline characters cannot be included in the string, please use escape characters '\\'");
        case '\\':
            c = get();
            switch (c)
            {
            case '"':
                result.push_back('"');
                break;
            case '\\':
                result.push_back('\\');
                break;
            case '/':
                result.push_back('/');
                break;
            case 'b':
                result.push_back('\b');
                break;
            case 'f':
                result.push_back('\f');
                break;
            case 'n':
                result.push_back('\n');
                break;
            case 'r':
                result.push_back('\r');
                break;
            case 't':
                result.push_back('\t');
                break;
            case 'u':
                throw error("TODO: \\u000");
            default:
                throw error("Unrecognized escape character.");
            }
            break;
        case '"':
            return result;
        default:
            result.push_back(c);
            break;
        }
    }
    throw error("need a '\"' after a string");
}

std::vector<JSON> JSON::Parser::array()
{
    std::vector<JSON> result;

    if (!match('[')) throw error("need a '[' before a array");

    whitespace();

    if (match(']')) return result;

    while (is.good())
    {
        result.emplace_back(value());
        auto c = is.get();
        switch (c)
        {
        case ',':
            break;
        case ']':
            return result;
        default:
            throw error("need a ',' or ']' after element");
        }
    }
    throw error("need a ']' after a array");
}

JSON::Object JSON::Parser::object()
{
    Object object;

    if (!match('{')) throw error("need a '{' before a object");

    whitespace();

    if (match('}')) return object;

    while (is.good())
    {
        auto key = string();

        if (object.find(key) != object.cend())
            throw error("repeat key in object");

        whitespace();

        if (!match(':')) throw error("need a ':' after key");

        object.insert({ key, value() });

        auto c = is.get();
        switch (c)
        {
        case ',':
            whitespace();
            break;
        case '}':
            return object;
        default:
            throw error("need a ',' or '}' after element");
        }
    }
    throw error("need a '}' after a object");
}

char JSON::Parser::digitExceptZero()
{
    auto c = is.peek();
    if (c >= '1' && c <= '9') return get();
    return static_cast<char>(false);
}

char JSON::Parser::digit()
{
    auto c = is.peek();
    if (c >= '0' && c <= '9') return get();
    return static_cast<char>(false);
}

std::string JSON::Parser::digits()
{
    std::string result;
    while (is.good())
    {
        auto c = digit();
        if (!c) break;
        result.push_back(c);
    }
    return result;
}

std::string JSON::Parser::fraction()
{
    if (!match('.')) return "";
    return digits();
}

std::string JSON::Parser::exponent()
{
    if (!match('e') && !match('E')) return "";
    if (match('-'))
    {
        return "-" + digits();
    }
    else
    {
        match('+');
        return digits();
    }
}

JSON JSON::Parser::number()
{
    std::string str;
    if (match('-')) str += "-";

    if (match('0'))
    {
        str += "0";
    }
    else
    {
        auto c = digitExceptZero();
        if (c) str += c;
        str += digits();
    }
    auto f = fraction();
    if (!f.empty()) str += "." + f;

    auto e = exponent();
    if (!e.empty()) str += "E" + e;

    return JSON{ std::stod(str) };
}

static std::string stringToString(const std::string& string)
{
    std::string result;
    result += "\"";
    for (auto c : string)
    {
        switch (c)
        {
        case '"':
            result += "\\\"";
            break;
        case '\\':
            result += "\\\\";
            break;
        case '/':
            result += "\\/";
            break;
        case '\b':
            result += "\\b";
            break;
        case '\f':
            result += "\\f";
            break;
        case '\n':
            result += "\\n";
            break;
        case '\r':
            result += "\\r";
            break;
        case '\t':
            result += "\\t";
            break;
        default:
            result += c;
            break;
        }
    }
    result += "\"";
    return result;
}

static std::string arrayToString(const std::vector<JSON>& array, int indent, int level)
{
    const std::string wrap{ indent < 0 ? "" : "\n" };
    const std::string indentation(indent > 0 ? indent * level : 0, ' ');
    const std::string indentation_1(indent > 0 ? indent * (level + 1) : 0, ' ');

    if (array.empty()) return "[]";

    std::string result;

    result += "[";
    result += wrap;

    auto it = array.cbegin();

    while (true)
    {
        result += indentation_1;
        result += it->to_string(indent, level + 1);
        ++it;
        if (it == array.cend()) break;
        result += ",";
        result += wrap;
    }

    result += wrap;
    result += indentation;
    result += "]";
    return result;
}

static std::string objectToString(const JSON::Object& object, int indent, int level)
{
    const std::string wrap{ indent < 0 ? "" : "\n" };
    const std::string indentation(indent > 0 ? indent * level : 0, ' ');
    const std::string indentation_1(indent > 0 ? indent * (level + 1) : 0, ' ');

    if (object.empty()) return "{}";

    std::string result;

    result += "{";
    result += wrap;

    auto it = object.cbegin();

    while (true)
    {
        result += indentation_1;
        result += "\"";
        result += it->first;
        result += "\"";
        result += ":";
        if (indent > 0) result += " ";
        result += it->second.to_string(indent, level + 1);
        ++it;
        if (it == object.cend()) break;
        result += ",";
        result += wrap;
    }

    result += wrap;
    result += indentation;
    result += "}";
    return result;
}

std::string JSON::to_string(int indent, int level) const
{
    const std::string wrap{ indent < 0 ? "" : "\n" };
    const std::string indentation(indent > 0 ? indent * level : 0, ' ');
    const std::string indentation_1(indent > 0 ? indent * (level + 1) : 0, ' ');
    return std::visit(
        Overload{
            [&](std::nullptr_t base) -> std::string {
                return "null";
            },
            [&](bool base) -> std::string {
                return base ? "true" : "false";
            },
            [&](const std::string& base) -> std::string {
                return stringToString(base);
            },
//            [&](int64_t base) -> std::string {
//                return std::to_string(base);
//            },
            [&](double base) -> std::string {
                return std::to_string(base);
            },
            [&](const std::vector<JSON>& base) -> std::string {
                return arrayToString(base, indent, level);
            },
            [&](const Object& base) -> std::string {
                return objectToString(base, indent, level);
            }
        }, value);
}

JSON JSON::parse(std::istream& is)
{
    return Parser{ is }.json;
}

JSON JSON::parse(const std::string& str)
{
    std::istringstream iss{ str };
    return parse(iss);
}
