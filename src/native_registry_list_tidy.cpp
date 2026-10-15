#include "bsp/native_registry_list_tidy.hpp"

#include "bsp/singleton_lifetime.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {
namespace {

template<class Value>
Value read_at(const void* base, std::size_t offset) noexcept {
    Value value;
    std::memcpy(&value, static_cast<const char*>(base) + offset, sizeof(value));
    return value;
}

template<class Value>
void write_at(void* base, std::size_t offset, Value value) noexcept {
    std::memcpy(static_cast<char*>(base) + offset, &value, sizeof(value));
}

} // namespace

void __fastcall tidy_native_registry_list_0041d710(void* actual_list_header) {
    static_assert(sizeof(void*) == 4);

    void* sentinel = read_at<void*>(actual_list_header, 4);
    void* node = read_at<void*>(sentinel, 0); // 0041D713..0041D716
    write_at(sentinel, 0, sentinel); // 0041D718
    sentinel = read_at<void*>(actual_list_header, 4); // 0041D71A
    write_at(sentinel, 4, sentinel); // 0041D71D
    const bool has_nodes = node != read_at<void*>(actual_list_header, 4);
    write_at(actual_list_header, 8, std::uint32_t{0}); // 0041D720..0041D723

    if (has_nodes) {
        do {
            void* next = read_at<void*>(node, 0); // capture before free
            singleton_lifetime_free(node); // 0041D733; actual CRT free service
            node = next;
        } while (node != read_at<void*>(actual_list_header, 4)); // 0041D73B
    }

    // The current sentinel, not a pointer retained across the node frees.
    sentinel = read_at<void*>(actual_list_header, 4); // 0041D743
    singleton_lifetime_free(sentinel); // 0041D747
    write_at(actual_list_header, 4, static_cast<void*>(nullptr)); // 0041D74F
}

void __fastcall tidy_native_registry_list_0041e290(void* actual_list_header) {
    tidy_native_registry_list_0041d710(actual_list_header);
}

} // namespace bsp
