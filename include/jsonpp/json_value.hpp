#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <concepts>
#include <ranges>
#include <optional>
#include "json_exception.hpp"

namespace jsonpp {

class value;

using null_type = std::monostate;
using boolean_type = bool;
using number_type = double;
using integer_type = int64_t;
using string_type = std::string;
using array_type = std::vector<value>;
using object_type = std::map<std::string, value, std::less<>>;

enum class value_type {
    null,
    boolean,
    number,
    integer,
    string,
    array,
    object
};

class value {
public:
    using variant_type = std::variant<
        null_type,
        boolean_type,
        integer_type,
        number_type,
        string_type,
        array_type,
        object_type
    >;

    value() noexcept = default;
    
    value(null_type) noexcept : data_{null_type{}} {}
    
    value(boolean_type b) noexcept : data_{b} {}
    
    value(integer_type i) noexcept : data_{i} {}
    
    value(int i) noexcept : data_{static_cast<integer_type>(i)} {}
    
    value(number_type n) noexcept : data_{n} {}
    
    value(const char* s) : data_{string_type{s}} {}
    
    value(string_type s) : data_{std::move(s)} {}
    
    value(std::string_view s) : data_{string_type{s}} {}
    
    value(array_type arr) : data_{std::move(arr)} {}
    
    value(object_type obj) : data_{std::move(obj)} {}
    
    value(std::initializer_list<value> init) : data_{array_type{init}} {}
    
    value(std::initializer_list<std::pair<const std::string, value>> init) 
        : data_{object_type{init}} {}

    [[nodiscard]] auto type() const noexcept -> value_type {
        return static_cast<value_type>(data_.index());
    }
    
    [[nodiscard]] auto is_null() const noexcept -> bool {
        return std::holds_alternative<null_type>(data_);
    }
    
    [[nodiscard]] auto is_boolean() const noexcept -> bool {
        return std::holds_alternative<boolean_type>(data_);
    }
    
    [[nodiscard]] auto is_number() const noexcept -> bool {
        return std::holds_alternative<number_type>(data_);
    }
    
    [[nodiscard]] auto is_integer() const noexcept -> bool {
        return std::holds_alternative<integer_type>(data_);
    }
    
    [[nodiscard]] auto is_string() const noexcept -> bool {
        return std::holds_alternative<string_type>(data_);
    }
    
    [[nodiscard]] auto is_array() const noexcept -> bool {
        return std::holds_alternative<array_type>(data_);
    }
    
    [[nodiscard]] auto is_object() const noexcept -> bool {
        return std::holds_alternative<object_type>(data_);
    }

    [[nodiscard]] auto as_boolean() const -> boolean_type {
        if (!is_boolean()) {
            throw type_exception{"Value is not a boolean"};
        }
        return std::get<boolean_type>(data_);
    }
    
    [[nodiscard]] auto as_integer() const -> integer_type {
        if (is_integer()) {
            return std::get<integer_type>(data_);
        }
        if (is_number()) {
            return static_cast<integer_type>(std::get<number_type>(data_));
        }
        throw type_exception{"Value is not a number"};
    }
    
    [[nodiscard]] auto as_number() const -> number_type {
        if (is_number()) {
            return std::get<number_type>(data_);
        }
        if (is_integer()) {
            return static_cast<number_type>(std::get<integer_type>(data_));
        }
        throw type_exception{"Value is not a number"};
    }
    
    [[nodiscard]] auto as_string() const -> const string_type& {
        if (!is_string()) {
            throw type_exception{"Value is not a string"};
        }
        return std::get<string_type>(data_);
    }
    
    [[nodiscard]] auto as_array() const -> const array_type& {
        if (!is_array()) {
            throw type_exception{"Value is not an array"};
        }
        return std::get<array_type>(data_);
    }
    
    [[nodiscard]] auto as_array() -> array_type& {
        if (!is_array()) {
            throw type_exception{"Value is not an array"};
        }
        return std::get<array_type>(data_);
    }
    
    [[nodiscard]] auto as_object() const -> const object_type& {
        if (!is_object()) {
            throw type_exception{"Value is not an object"};
        }
        return std::get<object_type>(data_);
    }
    
    [[nodiscard]] auto as_object() -> object_type& {
        if (!is_object()) {
            throw type_exception{"Value is not an object"};
        }
        return std::get<object_type>(data_);
    }

    [[nodiscard]] auto size() const -> size_t {
        if (is_array()) {
            return std::get<array_type>(data_).size();
        }
        if (is_object()) {
            return std::get<object_type>(data_).size();
        }
        throw type_exception{"Value is not an array or object"};
    }
    
    [[nodiscard]] auto empty() const -> bool {
        if (is_array()) {
            return std::get<array_type>(data_).empty();
        }
        if (is_object()) {
            return std::get<object_type>(data_).empty();
        }
        throw type_exception{"Value is not an array or object"};
    }
    
    auto push_back(value val) -> void {
        if (!is_array()) {
            throw type_exception{"Value is not an array"};
        }
        std::get<array_type>(data_).push_back(std::move(val));
    }
    
    [[nodiscard]] auto operator[](size_t index) const -> const value& {
        return std::get<array_type>(data_)[index];
    }
    
    [[nodiscard]] auto operator[](size_t index) -> value& {
        return std::get<array_type>(data_)[index];
    }

    [[nodiscard]] auto contains(std::string_view key) const -> bool {
        if (!is_object()) {
            return false;
        }
        const auto& obj = std::get<object_type>(data_);
        return obj.contains(key);
    }
    
    [[nodiscard]] auto at(std::string_view key) const -> const value& {
        if (!is_object()) {
            throw type_exception{"Value is not an object"};
        }
        const auto& obj = std::get<object_type>(data_);
        auto it = obj.find(key);
        if (it == obj.end()) {
            throw std::out_of_range{std::format("Key '{}' not found", key)};
        }
        return it->second;
    }
    
    [[nodiscard]] auto at(std::string_view key) -> value& {
        if (!is_object()) {
            throw type_exception{"Value is not an object"};
        }
        auto& obj = std::get<object_type>(data_);
        auto it = obj.find(key);
        if (it == obj.end()) {
            throw std::out_of_range{std::format("Key '{}' not found", key)};
        }
        return it->second;
    }
    
    [[nodiscard]] auto operator[](std::string_view key) -> value& {
        if (!is_object()) {
            throw type_exception{"Value is not an object"};
        }
        return std::get<object_type>(data_)[std::string{key}];
    }

    [[nodiscard]] auto operator==(const value& other) const noexcept -> bool = default;
    [[nodiscard]] auto operator<=>(const value& other) const noexcept = default;

    [[nodiscard]] auto get_variant() const noexcept -> const variant_type& {
        return data_;
    }

private:
    variant_type data_{null_type{}};
};

}
