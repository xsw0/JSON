//
// Created by 0x0 on 6/10/21.
//

#ifndef JSON_JSON_H
#define JSON_JSON_H


#include <string>
#include <map>
#include <vector>
#include <variant>
#include <list>
#include <unordered_map>
#include <any>

class JSON
{
public:
    using Array = std::vector<JSON>;
    using Object = std::unordered_map<std::string, JSON>;

private:
    class Parser;

    using value_type = std::variant<
        std::nullptr_t,         // null
        bool,                   // boolean
        double,                 // Number
        std::string,            // String
        Array,      // Array
        Object                  // Object
    >;
    template<class... Fs>
    struct Overload : Fs ... { using Fs::operator()...; };
    template<class... Fs> Overload(Fs...) -> Overload<Fs...>;

    value_type value{};
public:
    explicit JSON() = default;
    explicit JSON(value_type value) : value{ std::move(value) } {}

    static JSON parse(std::istream& is);
    static JSON parse(const std::string& str);
    [[nodiscard]] std::string to_string(int indent = -1, int level = 0) const;

    JSON& operator[](size_t index)
    {
        return std::get<Array>(value)[index];
    }

    JSON& operator[](const std::string& key)
    {
        return std::get<Object>(value)[key];
    }

    template<typename T>
    [[nodiscard]] bool is() const
    {
        return std::holds_alternative<T>(value);
    }

    template<typename T>
    T& as()
    {
        return std::get<T>(value);
    }

    [[nodiscard]] size_t size() const
    {
        return std::visit(
            Overload{
                [&](auto) -> size_t {
                    throw std::bad_cast();
                },
                [&](const std::string& base) -> size_t {
                    return base.size();
                },
                [&](const Array& base) -> size_t {
                    return base.size();
                },
                [&](const Object& base) -> size_t {
                    return base.size();
                }
            }, value);
    }

    class ParseError : public std::exception
    {
    public :
        std::string message;
        ParseError(size_t line, size_t column, const std::string& message)
            : message(std::to_string(line) + ":" + std::to_string(column) + " error: " + message) {}
        [[nodiscard]] const char* what() const noexcept override
        {
            return message.data();
        }
    };
};


#endif //JSON_JSON_H
