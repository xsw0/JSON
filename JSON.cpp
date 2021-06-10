//
// Created by 0x0 on 6/10/21.
//

#include "JSON.h"

#include <sstream>

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
                        std::string result;
                        result += "\"";
                        for (auto c : base)
                        {
                            switch (c)
                            {
                            case '\r':
                                result += "\\r";
                                break;
                            case '\n':
                                result += "\\n";
                                break;
                            case '\\':
                                result += "\\\\";
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
                    },
                    [&](int64_t base) -> std::string {
                        return std::to_string(base);
                    },
                    [&](double base) -> std::string {
                        return std::to_string(base);
                    },
                    [&](const std::vector<JSON>& base) -> std::string {
                        if (base.empty()) return "[]";

                        std::string result;

                        result += "[";
                        result += wrap;

                        auto it = base.cbegin();

                        while (true)
                        {
                            result += indentation_1;
                            result += it->to_string(indent, level + 1);
                            ++it;
                            if (it == base.cend()) break;
                            result += ",";
                            result += wrap;
                        }

                        result += wrap;
                        result += indentation;
                        result += "]";
                        return result;
                    },
                    [&](const Object& base) -> std::string {
                        if (base.empty()) return "{}";

                        std::string result;

                        result += "{";
                        result += wrap;

                        auto it = base.cbegin();

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
                            if (it == base.cend()) break;
                            result += ",";
                            result += wrap;
                        }

                        result += wrap;
                        result += indentation;
                        result += "}";
                        return result;
                    }
            }, value);
}

class JSON::Parser
{
public:
    std::istream& is;
    size_t line = 1;
    size_t column = 1;
    JSON json;

    void SkipWhitespace();
    bool match(char c);

    JSON value();

    std::string string();
    std::vector<JSON> array();
    Object object();
    JSON checkKeyword(std::string_view key);
    JSON number();
    std::string digits();

    auto get();
    [[nodiscard]] ParseError error(const std::string& message) const
    {
        return ParseError{ line, column, message };
    }

    explicit Parser(std::istream& is) : is{ is }, json{ value() }
    {
        if (is.good()) SkipWhitespace();
        if (is.good()) throw error("Redundant content");
    }
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
    if (!is.good()) throw error("parser error");
    SkipWhitespace();
    auto c = is.peek();
    switch (c)
    {
    case 'n':
        return checkKeyword("null");
    case 't':
        return checkKeyword("true");
    case 'f':
        return checkKeyword("false");
    case '"':
        return JSON{ string() };
    case '{':
        return JSON{ object() };
    case '[':
        return JSON{ array() };
    default:
        return number();
    }
}

void JSON::Parser::SkipWhitespace()
{
    while (true)
    {
        switch (is.peek())
        {
        case '\n':
        case '\t':
        case '\r':
        case ' ':
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
            case 'r':
                result.push_back('\r');
                break;
            case 't':
                result.push_back('\t');
                break;
            case 'n':
                result.push_back('\n');
                break;
            case '\\':
                result.push_back('\\');
                break;
            case '"':
                result.push_back('"');
                break;
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
    if (!match('[')) throw error("need a '[' before a array");
    std::vector<JSON> result;
    bool comma = true;
    while (is.good())
    {
        SkipWhitespace();
        if (match(']')) return result;
        if (!comma) throw error("need a ',' between element");
        result.emplace_back(value());

        SkipWhitespace();
        comma = match(',');
    }
    throw error("need a ']' after a array");
}

JSON::Object JSON::Parser::object()
{
    if (!match('{')) throw error("need a '{' before a object");
    Object object;

    bool comma = true;
    while (is.good())
    {
        SkipWhitespace();
        if (match('}')) return object;

        if (!comma) throw error("need a ',' between element");

        auto key = string();
        if (object.find(key) != object.cend())
            throw error("repeat key in object");

        SkipWhitespace();
        if (!match(':')) throw error("need a ':' after key");

        auto v = value();

        object.insert({ key, v });

        SkipWhitespace();
        comma = match(',');
    }
    throw error("need a '}' after a object");
}

std::string JSON::Parser::digits()
{
    std::string result;
    while (is.good())
    {
        char c = is.peek();
        if (c >= '0' && c <= '9')
        {
            result.push_back(get());
        }
        else
        {
            return result;
        }
    }
    return result;
}

JSON JSON::Parser::number()
{
    std::string s;
    auto c = is.peek();
    bool positive = true;
    if (c == '+')
    {
        s.push_back(get());
    }
    else if (c == '-')
    {
        s.push_back(get());
        positive = false;
    }
    s += digits();
    bool isDecimal = match('.');
    if (isDecimal) s += '.';
    s += digits();
    if (s.empty()) throw error("error value");
    c = is.peek();
    std::string pow;
    if (c == 'e' || c == 'E')
    {
        if (isDecimal)
        {
            s.push_back(get());
        }
        else
        {
            get();
        }
        c = is.peek();
        if (c == '+' || c == '-')
        {
            pow.push_back(get());
        }
        pow += digits();
        if (isDecimal) s += pow;
    }
    if (isDecimal)
    {
        double d = 0.0;
        std::stringstream ss{ s };
        ss >> d;
        return JSON{ d };
    }
    else
    {
        int64_t n = 0;
        int64_t p = 0;
        std::stringstream ss{ s };
        ss >> n;
        std::stringstream ss_pow{ pow };
        ss_pow >> p;
        if (n == 0) return JSON{ n };
        if (p > 0)
        {
            if (n > 0) n = -n;
            while (p != 0)
            {
                if (n < std::numeric_limits<int64_t>::min() / 10)
                    throw error("intger out of range");
                n *= 10;
                --p;
            }
            if (positive)
            {
                if (n == std::numeric_limits<int64_t>::min())
                    throw error("intger out of range");
                n = -n;
            }
        }
        else if (p < 0)
        {
            while (p != 0 && n != 0)
            {
                n /= 10;
                ++p;
            }
        }
        return JSON{ n };
    }
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
