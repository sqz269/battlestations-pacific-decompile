#include "bsp/native_string_uninitialized_copy.hpp"

#include "bsp/native_render_resource_alias_nodes.hpp"
#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* pointer) noexcept { return reinterpret_cast<Word>(pointer); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
} // namespace

void destroy_native_string_storage_argument_00431f40(void*, void* header,
    NativeStringRawPoolContext& strings) {
    // 00431F40's stacked-header schedule equals0041DD20's ECX-header schedule.
    destroy_native_string_header_0041dd20(header, strings);
}

void* copy_native_string_uninitialized_range_0044d0c0(const void* first,
    const void* last, void* destination, void* owner,
    NativeStringRawPoolContext& strings) {
    Word current_source = bits(first);
    const Word source_end = bits(last);
    const Word destination_begin = bits(destination);
    Word completed_end = destination_begin;
    try {
        while (current_source != source_end) {
            construct_native_render_alias_string_0044bcb0(
                pointer(completed_end), pointer(current_source), strings);
            // Native publishes completed end only AFTER constructor returns.
            completed_end += 8u;
            current_source += 8u;
        }
    } catch (...) {
        // 0044D10A: captured begin and completed end; owner is passed to a
        // destructor that ignores it. Do not destroy the failed current slot.
        Word current = destination_begin;
        while (current != completed_end) {
            destroy_native_string_storage_argument_00431f40(
                owner, pointer(current), strings);
            current += 8u;
        }
        throw; // Source-domain counterpart of __CxxThrowException(0,0).
    }
    return pointer(completed_end);
}

void* copy_native_string_uninitialized_dispatch_00505ad0(void* owner,
    const void* first, const void* last, void* destination,
    NativeStringRawPoolContext& strings) {
    return copy_native_string_uninitialized_range_0044d0c0(
        first, last, destination, owner, strings);
}
} // namespace bsp
