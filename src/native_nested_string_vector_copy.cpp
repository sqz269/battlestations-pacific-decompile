#include "bsp/native_nested_string_vector_copy.hpp"

#include "bsp/native_string_vector_copy_construct.hpp"

#include <cstdint>

namespace bsp {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const void* value) noexcept { return reinterpret_cast<Word>(value); }
void* pointer(Word value) noexcept { return reinterpret_cast<void*>(value); }
} // namespace

void* __fastcall copy_native_nested_string_vector_uninitialized_range_00559e00(
    const void* first, const void* last, void* destination,
    std::uint32_t, std::uint32_t, std::uint32_t,
    NativeStringRawPoolContext& strings) {
    Word source = bits(first);
    const Word finish = bits(last);
    Word current = bits(destination);
    const Word initial_destination = current;
    Word completed_end = current;
    try {
        while (source != finish) {
            // Native state1's placement-delete unwind action is proven RET-only.
            // The real child retains all its own constructor/cleanup effects.
            if (current != 0u) {
                construct_native_string_header_vector_copy_00557bc0(
                    pointer(current), 0u, pointer(source), strings);
            }
            current += 0x10u;
            completed_end = current; //00559E52 publishes only after success/skip.
            source += 0x10u;
        }
    } catch (...) {
        Word cleanup = initial_destination;
        while (cleanup != completed_end) {
            //00559DC0 and accepted00432440 have identical complete actual Tidy
            // effects/arguments; Source uses the already accepted real service.
            // No null skip here: native catch calls Tidy for EVERY prefix slot.
            tidy_native_string_header_vector_00432440(pointer(cleanup), strings);
            cleanup += 0x10u;
        }
        throw;
    }
    return pointer(current);
}

void* __fastcall copy_native_nested_string_vector_dispatch_0055a950(
    std::uint32_t dead_owner_ecx, std::uint32_t,
    std::uint32_t, const void* first, std::uint32_t, const void* last,
    void* destination, NativeStringRawPoolContext& strings) {
    return copy_native_nested_string_vector_uninitialized_range_00559e00(
        first, last, destination, dead_owner_ecx, bits(destination),
        dead_owner_ecx & 0xffffff00u, strings);
}
} // namespace bsp
