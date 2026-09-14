#include "bsp/native_structured_node_destruction.hpp"
#include "bsp/native_ref_counted.hpp"
#include "bsp/native_string.hpp"

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native structured node destruction requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

volatile std::uint32_t& word(void* storage, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(
        static_cast<unsigned char*>(storage) + offset);
}

void* pointer(void* storage, std::size_t offset) noexcept {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(word(storage, offset)));
}
} // namespace

void destroy_native_structured_node_00be9df0(void* actual_node,
    NativeStringRawPoolContext& strings) {
    word(actual_node, 0) = 0x00d68bb4u;
    // Native state0 is armed before the reader test. CC70A0 calls BD30F0
    // on unwind; normal execution disarms state0 before that same base call.
    __try {
        if (pointer(actual_node, 8) != nullptr) {
            void* const parent = pointer(actual_node, 0x0c);
            if (parent != nullptr) {
                const auto declared = word(actual_node, 0x1c);
                word(parent, 0x20) = word(parent, 0x20) - declared;
            }
            // Reload after the parent debit, matching BE9E2B. Aliasing storage
            // must observe the actual field schedule, not a cached host owner.
            void* const reader = pointer(actual_node, 8);
            const auto path_index = word(reader, 0x60);
            word(reader, 0x60) = path_index - 1u;
            word(actual_node, 8) = 0;
        }
        destroy_native_string_header_0041dd20(
            static_cast<unsigned char*>(actual_node) + 0x10, strings);
    } __finally {
        destroy_native_ref_counted_base_00bd30f0(actual_node);
    }
}
} // namespace bsp
