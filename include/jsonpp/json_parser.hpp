#pragma once

#include <string_view>
#include <charconv>
#include <cctype>
#include <optional>
#include "json_value.hpp"
#include "json_exception.hpp"

namespace jsonpp {

class parser {
public:
    explicit parser(std::string_view input) noexcept 
        : input_{input}, position_{0} {}

    [[nodiscard]] auto parse() -> value {
        skip_whitespace();
        auto result = parse_value();
        skip_whitespace();
        
        if (position_ < input_.size()) {
            throw parse_exception{"Unexpected characters after JSON value", position_};
        }
        
        return result;
    }

private:
    std::string_view input_;
    size_t position_;

    auto skip_whitespace() noexcept -> void {
        while (position_ < input_.size() && std::isspace(input_[position_])) {
            ++position_;
        }
    }

    [[nodiscard]] auto peek() const noexcept -> std::optional<char> {
        if (position_ >= input_.size()) {
            return std::nullopt;
        }
        return input_[position_];
    }

    [[nodiscard]] auto consume() -> char {
        if (position_ >= input_.size()) {
            throw parse_exception{"Unexpected end of input", position_};
        }
        return input_[position_++];
    }

    auto expect(char expected) -> void {
        const auto ch = consume();
        if (ch != expected) {
            throw parse_exception{
                std::format("Expected '{}', got '{}'", expected, ch), 
                position_ - 1
            };
        }
    }

    [[nodiscard]] auto parse_value() -> value {
        skip_whitespace();
        
        const auto ch = peek();
        if (!ch) {
            throw parse_exception{"Unexpected end of input", position_};
        }

        switch (*ch) {
            case 'n': return parse_null();
            case 't':
            case 'f': return parse_boolean();
            case '"': return parse_string();
            case '[': return parse_array();
            case '{': return parse_object();
            case '-':
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                return parse_number();
            default:
                throw parse_exception{
                    std::format("Unexpected character '{}'", *ch), 
                    position_
                };
        }
    }

    [[nodiscard]] auto parse_null() -> value {
        if (position_ + 4 > input_.size() || 
            input_.substr(position_, 4) != "null") {
            throw parse_exception{"Invalid null literal", position_};
        }
        position_ += 4;
        return value{null_type{}};
    }

    [[nodiscard]] auto parse_boolean() -> value {
        if (position_ + 4 <= input_.size() && 
            input_.substr(position_, 4) == "true") {
            position_ += 4;
            return value{true};
        }
        
        if (position_ + 5 <= input_.size() && 
            input_.substr(position_, 5) == "false") {
            position_ += 5;
            return value{false};
        }
        
        throw parse_exception{"Invalid boolean literal", position_};
    }

    [[nodiscard]] auto parse_number() -> value {
        const size_t start = position_;
        
        if (peek() == '-') {
            ++position_;
        }
        bool has_decimal = false;
        bool has_exponent = false;
        
        if (!peek() || !std::isdigit(*peek())) {
            throw parse_exception{"Invalid number", start};
        }

        if (*peek() == '0') {
            ++position_;
        } else {
            while (peek() && std::isdigit(*peek())) {
                ++position_;
            }
        }

        if (peek() == '.') {
            has_decimal = true;
            ++position_;
            
            if (!peek() || !std::isdigit(*peek())) {
                throw parse_exception{"Invalid number: expected digit after '.'", position_};
            }
            
            while (peek() && std::isdigit(*peek())) {
                ++position_;
            }
        }

        if (peek() == 'e' || peek() == 'E') {
            has_exponent = true;
            ++position_;
            
            if (peek() == '+' || peek() == '-') {
                ++position_;
            }
            
            if (!peek() || !std::isdigit(*peek())) {
                throw parse_exception{"Invalid number: expected digit in exponent", position_};
            }
            
            while (peek() && std::isdigit(*peek())) {
                ++position_;
            }
        }

        const auto number_str = input_.substr(start, position_ - start);
        
        if (has_decimal || has_exponent) {
            double result;
            auto [ptr, ec] = std::from_chars(
                number_str.data(), 
                number_str.data() + number_str.size(), 
                result
            );
            
            if (ec != std::errc{}) {
                throw parse_exception{"Failed to parse number", start};
            }
            
            return value{result};
        } else {
            int64_t result;
            auto [ptr, ec] = std::from_chars(
                number_str.data(), 
                number_str.data() + number_str.size(), 
                result
            );
            
            if (ec != std::errc{}) {
                throw parse_exception{"Failed to parse integer", start};
            }
            
            return value{result};
        }
    }

    [[nodiscard]] auto parse_string() -> value {
        expect('"');
        
        std::string result;
        result.reserve(32); 
        
        while (true) {
            if (position_ >= input_.size()) {
                throw parse_exception{"Unterminated string", position_};
            }
            
            const char ch = input_[position_++];
            
            if (ch == '"') {
                break;
            }
            
            if (ch == '\\') {
                if (position_ >= input_.size()) {
                    throw parse_exception{"Unterminated escape sequence", position_};
                }
                
                const char escaped = input_[position_++];
                
                switch (escaped) {
                    case '"':  result += '"'; break;
                    case '\\': result += '\\'; break;
                    case '/':  result += '/'; break;
                    case 'b':  result += '\b'; break;
                    case 'f':  result += '\f'; break;
                    case 'n':  result += '\n'; break;
                    case 'r':  result += '\r'; break;
                    case 't':  result += '\t'; break;
                    case 'u': {

                        if (position_ + 4 > input_.size()) {
                            throw parse_exception{"Invalid unicode escape", position_};
                        }
                        
                        uint32_t codepoint = 0;
                        for (int i = 0; i < 4; ++i) {
                            const char hex = input_[position_++];
                            codepoint <<= 4;
                            
                            if (hex >= '0' && hex <= '9') {
                                codepoint |= (hex - '0');
                            } else if (hex >= 'a' && hex <= 'f') {
                                codepoint |= (hex - 'a' + 10);
                            } else if (hex >= 'A' && hex <= 'F') {
                                codepoint |= (hex - 'A' + 10);
                            } else {
                                throw parse_exception{"Invalid hex digit in unicode escape", position_ - 1};
                            }
                        }
                        
                        if (codepoint <= 0x7F) {
                            result += static_cast<char>(codepoint);
                        } else if (codepoint <= 0x7FF) {
                            result += static_cast<char>(0xC0 | (codepoint >> 6));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        } else {
                            result += static_cast<char>(0xE0 | (codepoint >> 12));
                            result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                            result += static_cast<char>(0x80 | (codepoint & 0x3F));
                        }
                        break;
                    }
                    default:
                        throw parse_exception{
                            std::format("Invalid escape sequence '\\{}'", escaped), 
                            position_ - 1
                        };
                }
            } else if (static_cast<unsigned char>(ch) < 0x20) {
                throw parse_exception{"Unescaped control character in string", position_ - 1};
            } else {
                result += ch;
            }
        }
        
        return value{std::move(result)};
    }

    [[nodiscard]] auto parse_array() -> value {
        expect('[');
        skip_whitespace();
        
        array_type result;
        
        if (peek() == ']') {
            ++position_;
            return value{std::move(result)};
        }
        
        while (true) {
            result.push_back(parse_value());
            skip_whitespace();
            
            const auto ch = peek();
            if (!ch) {
                throw parse_exception{"Unterminated array", position_};
            }
            
            if (*ch == ']') {
                ++position_;
                break;
            }
            
            if (*ch == ',') {
                ++position_;
                skip_whitespace();
                
                if (peek() == ']') {
                    throw parse_exception{"Trailing comma in array", position_};
                }
            } else {
                throw parse_exception{
                    std::format("Expected ',' or ']', got '{}'", *ch), 
                    position_
                };
            }
        }
        
        return value{std::move(result)};
    }

    [[nodiscard]] auto parse_object() -> value {
        expect('{');
        skip_whitespace();
        
        object_type result;
        
        if (peek() == '}') {
            ++position_;
            return value{std::move(result)};
        }
        
        while (true) {
            skip_whitespace();
            
            if (peek() != '"') {
                throw parse_exception{"Expected string key in object", position_};
            }
            
            auto key = parse_string().as_string();
            
            skip_whitespace();
            expect(':');
            skip_whitespace();
            auto val = parse_value();
            
            result.emplace(std::move(key), std::move(val));
            
            skip_whitespace();
            
            const auto ch = peek();
            if (!ch) {
                throw parse_exception{"Unterminated object", position_};
            }
            
            if (*ch == '}') {
                ++position_;
                break;
            }
            
            if (*ch == ',') {
                ++position_;
                skip_whitespace();
                
                if (peek() == '}') {
                    throw parse_exception{"Trailing comma in object", position_};
                }
            } else {
                throw parse_exception{
                    std::format("Expected ',' or '}}', got '{}'", *ch), 
                    position_
                };
            }
        }
        
        return value{std::move(result)};
    }
};

}
