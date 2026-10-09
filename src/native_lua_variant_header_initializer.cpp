#include "bsp/native_lua_variant_header_initializer.hpp"

#include "bsp/native_lua_variant_link_record_allocation.hpp"

namespace bsp {

static_assert(sizeof(void*) == 4);
static_assert(sizeof(std::uint32_t) == 4);

void* initialize_native_lua_variant_header_006eeaf0(
    void* actual_header,
    void* volatile& actual_head_04,
    volatile std::uint32_t& actual_count_08) {
    void* const new_head = allocate_native_lua_variant_link_record_006edea0(); // 006EEAF3
    actual_head_04 = new_head; // 006EEAF8
    *reinterpret_cast<volatile unsigned char*>(
        static_cast<unsigned char*>(new_head) + 0x31) = 1; // 006EEAFB

    void* const head_for_04 = actual_head_04; // 006EEAFF: fresh read
    *reinterpret_cast<void* volatile*>(
        static_cast<unsigned char*>(head_for_04) + 0x04) = head_for_04; // 006EEB02

    void* const head_for_00 = actual_head_04; // 006EEB05: fresh read
    *reinterpret_cast<void* volatile*>(
        static_cast<unsigned char*>(head_for_00) + 0x00) = head_for_00; // 006EEB08

    void* const head_for_08 = actual_head_04; // 006EEB0A: fresh read
    *reinterpret_cast<void* volatile*>(
        static_cast<unsigned char*>(head_for_08) + 0x08) = head_for_08; // 006EEB0D

    actual_count_08 = 0; // 006EEB10
    return actual_header; // Native receiver identity at 006EEB17..006EEB1A
}

} // namespace bsp
