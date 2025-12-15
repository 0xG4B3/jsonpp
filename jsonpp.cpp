#include <jsonpp/json.hpp>
#include <iostream>
#include <format>

auto main() -> int {
    try {
        const auto json_text = R"({
            "library": "JSONPP",
            "version": "1.0.0",
            "features": ["header-only", "type-safe", "modern", "fast"],
            "author": {
                "name": "João Gabriel"
            }
        })";

        auto json = jsonpp::parse(json_text);

        std::cout << "Lib: " << json["library"].as_string() << '\n';
        std::cout << "Version: " << json["version"].as_string() << '\n';

        std::cout << "\nFeatures:\n";
        for (const auto& feature : json["features"].as_array()) {
            std::cout << feature.as_string() << '\n';
        }

        std::cout << "\nAutor: " << json["author"]["name"].as_string() << '\n';

    } catch (const jsonpp::json_exception& e) {
        std::cerr << "Erro: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
