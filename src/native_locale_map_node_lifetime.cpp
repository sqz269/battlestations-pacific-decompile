#include "bsp/native_locale_map_node_lifetime.hpp"
#include "bsp/native_wide_string.hpp"

#include <cstddef>

namespace bsp {
namespace {

struct NodeNarrowUnwind {
    void* original_node;
    NativeStringRawPoolContext& context;
    bool armed = true;

    ~NodeNarrowUnwind() noexcept(false) {
        if (armed)
            destroy_native_string_header_0041dd20(original_node, context);
    }
};

} // namespace

void destroy_native_locale_map_node_payload_00737ce0(void* actual_node,
    NativeStringRawPoolContext& context) {
    NodeNarrowUnwind narrow_cleanup{actual_node, context};
    destroy_native_wide_string_header_00436430(
        static_cast<std::byte*>(actual_node) + 8, context);
    narrow_cleanup.armed = false;
    destroy_native_string_header_0041dd20(actual_node, context);
}

} // namespace bsp
