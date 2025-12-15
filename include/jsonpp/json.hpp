#pragma once

#include "json_value.hpp"
#include "json_parser.hpp"
#include "json_serializer.hpp"
#include "json_exception.hpp"

namespace jsonpp {

    [[nodiscard]] inline auto parse(std::string_view json_text) -> value {
        parser p{ json_text };
        return p.parse();
    }


    [[nodiscard]] inline auto to_string(const value& val, bool pretty = false) -> std::string {
        serializer s{ pretty };
        return s.serialize(val);
    }
}
