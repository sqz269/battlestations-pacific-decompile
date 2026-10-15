#include "bsp/native_string_vector_copy_construct.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_uninitialized_copy.hpp"
#include "bsp/native_string_vector_allocation.hpp"

namespace bsp {
namespace {

static_assert(sizeof(std::uintptr_t) == 4);
static_assert(sizeof(NativeString) == 8);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uintptr_t>(value);
}

void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}

std::uint32_t word(std::uint32_t base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(base + offset);
}

void put(std::uint32_t base, std::uint32_t offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(base + offset) = value;
}

std::uint32_t sar_three(std::uint32_t value) noexcept {
    return (value >> 3) | ((value & 0x80000000u) ? 0xe0000000u : 0u);
}

} // namespace

void* __fastcall copy_native_string_uninitialized_range_00556b10(
    const void* first, const void* last, void* destination, void* owner,
    std::uint32_t, std::uint32_t, NativeStringRawPoolContext& strings) {
    // Both normal loop and completed-prefix catch were compared in full.
    // The original handler metadata and native parent-frame entry remain distinct.
    return copy_native_string_uninitialized_range_0044d0c0(first, last,
        destination, owner, strings);
}

void __fastcall tidy_native_string_header_vector_00432440(void* header,
    NativeStringRawPoolContext& strings) {
    destroy_native_string_header_vector_004324a0(header, strings);
}

void* __fastcall construct_native_string_header_vector_copy_00557bc0(
    void* destination, std::uint32_t, const void* source,
    NativeStringRawPoolContext& strings) {
    const auto source_address = address(source);
    const auto destination_address = address(destination);
    const auto old_source_begin = word(source_address, 4);
    const auto count = old_source_begin == 0u ? 0u :
        sar_three(word(source_address, 8) - old_source_begin);

    put(destination_address, 4, 0u);
    put(destination_address, 8, 0u);
    put(destination_address, 12, 0u);
    if (count == 0u)
        return destination;

    if (count > 0x1fffffffu)
        throw_native_string_vector_length_error_00431f70();
    const auto backing = address(allocate_native_string_vector_0042bf40(count,
        nullptr));
    put(destination_address, 4, backing);
    put(destination_address, 8, backing);
    put(destination_address, 12, backing + count * 8u);

    const auto captured_end = word(source_address, 8);
    const bool begin_after_end = word(source_address, 4) > captured_end;
    // Native state0 is armed only after both reads above, before either report.
    try {
        if (begin_after_end)
            invoke_native_invalid_parameter_00bf6713();
        const auto captured_begin = word(source_address, 4);
        if (captured_begin > word(source_address, 8))
            invoke_native_invalid_parameter_00bf6713();
        const auto destination_begin = word(destination_address, 4);
        const auto unused_tag = captured_begin & 0xffffff00u;
        const auto completed_end = copy_native_string_uninitialized_range_00556b10(
            pointer(captured_begin), pointer(captured_end),
            pointer(destination_begin), destination, unused_tag, unused_tag,
            strings);
        put(destination_address, 8, address(completed_end));
    } catch (...) {
        tidy_native_string_header_vector_00432440(destination, strings);
        throw;
    }
    return destination;
}

} // namespace bsp
