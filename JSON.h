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

class JSON
{
public:
    class Object
    {
    public:
        using key_type = std::string;
        using mapped_type = JSON;
        using value_type = std::pair<const key_type, mapped_type>;

        using iterator = std::list<value_type>::iterator;
        using const_iterator = std::list<value_type>::const_iterator;
        using reverse_iterator = std::list<value_type>::reverse_iterator;
        using const_reverse_iterator = std::list<value_type>::const_reverse_iterator;

        [[nodiscard]] const_iterator find(const key_type& key) const
        {
            auto it = _map.find(key);
            return it == _map.cend() ? cend() : it->second;
        }

        [[nodiscard]] iterator find(const key_type& key)
        {
            auto it = _map.find(key);
            return it == _map.cend() ? end() : it->second;
        }

        void insert(const value_type& key_value)
        {
            assert(_map.find(key_value.first) == _map.cend());
            _value.push_back(key_value);
            _map.insert({ key_value.first, std::prev(_value.end()) });
        }

        iterator erase(const key_type& key)
        {
            auto it = _map.find(key);
            assert(it != _map.cend());
            auto result = _value.erase(it->second);
            _map.erase(it);
            return result;
        }

        [[nodiscard]] bool empty() const { return _value.empty(); }

        [[nodiscard]] iterator begin() { return _value.begin(); }
        [[nodiscard]] iterator end() { return _value.end(); }
        [[nodiscard]] const_iterator cbegin() const { return _value.cbegin(); }
        [[nodiscard]] const_iterator cend() const { return _value.cend(); }
        [[nodiscard]] reverse_iterator rbegin() { return _value.rbegin(); }
        [[nodiscard]] reverse_iterator rend() { return _value.rend(); }
        [[nodiscard]] const_reverse_iterator crbegin() const { return _value.crbegin(); }
        [[nodiscard]] const_reverse_iterator crend() const { return _value.crend(); }

    private:
        std::list<value_type> _value;
        std::map<key_type, iterator> _map;
    };

private:
    class Parser;

    using value_type = std::variant<
        std::nullptr_t,         // null
        bool,                   // boolean
//        int64_t,                // Number
        double,                 // Number
        std::string,            // String
        std::vector<JSON>,      // Array
        Object                  // Object sort by read
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
