#pragma once

#include <stdexcept>
#include <string>
#include <format>
#include <source_location>

namespace jsonpp {

    class json_exception : public std::runtime_error
    {
        public:
        explicit json_exception(
            const std::string& message,
            std::source_location location = std::source_location::current()
        ) : std::runtime_error{ std::format("JSON Error: {} ({}:{})",
                                           message,
                                           location.file_name(),
                                           location.line()) } {}
    };

    class parse_exception : public json_exception
    {
        public:
        explicit parse_exception(
            const std::string& message,
            size_t position = 0,
            std::source_location location = std::source_location::current()
        ) : json_exception{ std::format("{} at position {}", message, position), location },
            position_{ position } {
        }

        [[nodiscard]] auto position() const noexcept -> size_t {
            return position_;
        }

    private:
        size_t position_;
    };

    class type_exception : public json_exception {
    public:
        using json_exception::json_exception;
    };

}
