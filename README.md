## C++23 JSON Parser
A lightweight, type-safe JSON parsing library implemented in modern C++23, created as a personal study project to explore advanced language features and sound systems design principles.
The primary goal was hands-on learning: building a fully functional JSON parser from scratch while emphasizing type safety, performance, and idiomatic use of contemporary C++ all without relying on inheritance, virtual functions, or runtime polymorphism.
C++23

- Data Model: Uses std::variant to represent the seven JSON value types (null, boolean, integer, double, string, array, object). This provides zero-overhead abstraction and enables efficient pattern matching via std::visit.
- Parser: Hand-written recursive descent parser, fully compliant with RFC 8259.
Numeric Parsing: Leverages std::from_chars for high-performance conversion of strings to integers and floating-point numbers.
- Type Safety: Strict type enforcement—values are accessed through typed methods (as_string(), as_int(), as_double(), as_bool(), as_array(), as_object()) that throw a custom type_exception on mismatch.
- Modern C++23 Usage:
Three-way comparison operator (<=>) for automatic ordering and equality
std::format for type-safe string formatting
Trailing return types and [[nodiscard]] attributes

- Memory Efficiency: Full move semantics, perfect forwarding, and extensive use of std::string_view during parsing to avoid unnecessary allocations and copies.
Error Handling: Custom exception hierarchy enriched with std::source_location for precise error messages and location tracking.
API Design: Clean, minimalist facade-style interface with strict const-correctness and carefully chosen operator overloading.

Known Limitations
*This is an educational project, not intended for production use*:

No support for streaming large files
No advanced performance optimizations (e.g., SIMD, custom allocators)
Limited error recovery
No comprehensive test suite or benchmarks
Not validated against official JSON test suites

For production applications, consider established libraries such as nlohmann/json, RapidJSON, or simdjson.
