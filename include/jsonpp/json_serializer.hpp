#pragma once

#include <string>
#include <sstream>
#include <iomanip>
#include "json_value.hpp"

namespace jsonpp {

    class serializer {
    public:
        explicit serializer(bool pretty = false, size_t indent_size = 2) noexcept
            : pretty_{ pretty }, indent_size_{ indent_size }, current_indent_{ 0 } {
        }

        [[nodiscard]] auto serialize(const value& val) -> std::string {
            std::ostringstream oss;
            serialize_value(oss, val);
            return oss.str();
        }

    private:
        bool pretty_;
        size_t indent_size_;
        size_t current_indent_;

        auto write_indent(std::ostringstream& oss) const -> void {
            if (pretty_) {
                oss << std::string(current_indent_ * indent_size_, ' ');
            }
        }

        auto write_newline(std::ostringstream& oss) const -> void {
            if (pretty_) {
                oss << '\n';
            }
        }

        auto write_space(std::ostringstream& oss) const -> void {
            if (pretty_) {
                oss << ' ';
            }
        }

        auto serialize_value(std::ostringstream& oss, const value& val) -> void {
            switch (val.type()) {
            case value_type::null:
                oss << "null";
                break;

            case value_type::boolean:
                oss << (val.as_boolean() ? "true" : "false");
                break;

            case value_type::integer:
                oss << val.as_integer();
                break;

            case value_type::number:
                serialize_number(oss, val.as_number());
                break;

            case value_type::string:
                serialize_string(oss, val.as_string());
                break;

            case value_type::array:
                serialize_array(oss, val.as_array());
                break;

            case value_type::object:
                serialize_object(oss, val.as_object());
                break;
            }
        }

        auto serialize_number(std::ostringstream& oss, double num) const -> void {

            if (num == static_cast<int64_t>(num) &&
                num >= std::numeric_limits<int64_t>::min() &&
                num <= std::numeric_limits<int64_t>::max()) {
                oss << static_cast<int64_t>(num);
            }
            else
            {
                oss << std::setprecision(17) << num;
            }
        }

        auto serialize_string(std::ostringstream& oss, const std::string& str) const -> void {
            oss << '"';

            for (char ch : str) {
                switch (ch) {
                case '"':  oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;
                default:
                    if (static_cast<unsigned char>(ch) < 0x20) {
                        oss << "\\u"
                            << std::hex << std::setfill('0') << std::setw(4)
                            << static_cast<int>(static_cast<unsigned char>(ch))
                            << std::dec;
                    }
                    else {
                        oss << ch;
                    }
                    break;
                }
            }

            oss << '"';
        }

        auto serialize_array(std::ostringstream& oss, const array_type& arr) -> void {
            oss << '[';

            if (!arr.empty()) {
                write_newline(oss);
                ++current_indent_;

                bool first = true;
                for (const auto& element : arr) {
                    if (!first) {
                        oss << ',';
                        write_newline(oss);
                    }
                    first = false;

                    write_indent(oss);
                    serialize_value(oss, element);
                }

                --current_indent_;
                write_newline(oss);
                write_indent(oss);
            }

            oss << ']';
        }

        auto serialize_object(std::ostringstream& oss, const object_type& obj) -> void {
            oss << '{';

            if (!obj.empty()) {
                write_newline(oss);
                ++current_indent_;

                bool first = true;
                for (const auto& [key, val] : obj) {
                    if (!first) {
                        oss << ',';
                        write_newline(oss);
                    }
                    first = false;

                    write_indent(oss);
                    serialize_string(oss, key);
                    oss << ':';
                    write_space(oss);
                    serialize_value(oss, val);
                }

                --current_indent_;
                write_newline(oss);
                write_indent(oss);
            }

            oss << '}';
        }
    };

}
