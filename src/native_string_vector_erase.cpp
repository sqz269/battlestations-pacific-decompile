#include "bsp/native_string_vector_erase.hpp"

#include "bsp/native_invalid_parameter_noinfo_call.hpp"
#include "bsp/native_render_service_vector_cleanup.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_range_assignment.hpp"

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

void* __fastcall erase_native_string_header_vector_range_004954f0(void* header,
    std::uint32_t, void* output, const void* first_owner, void* first,
    const void* last_owner, void* last, NativeStringRawPoolContext& strings) {
    if (first_owner == nullptr || first_owner != last_owner)
        invoke_native_invalid_parameter_00bf6713();

    const auto first_address = address(first);
    const auto last_address = address(last);
    if (first_address != last_address) {
        const auto header_address = address(header);
        const auto captured_end = word(header_address, 8);
        assign_native_string_header_range_0048da50(last, pointer(captured_end), first,
            address(output), header_address & 0xffffff00u,
            address(last_owner) & 0xffffff00u, strings);
        const auto destruction_end = word(header_address, 8);
        const auto suffix_count = sar_three(captured_end - last_address);
        const auto new_end = first_address + suffix_count * 8u;
        destroy_native_string_header_range_00432050(pointer(new_end),
            pointer(destruction_end), strings);
        put(header_address, 8, new_end);
    }

    const auto output_address = address(output);
    put(output_address, 0, address(first_owner));
    put(output_address, 4, first_address);
    return pointer(output_address);
}

void __fastcall clear_native_string_header_vector_00498ff0(void* header,
    NativeStringRawPoolContext& strings) {
    const auto header_address = address(header);
    const auto captured_end = word(header_address, 8);
    if (word(header_address, 4) > captured_end)
        invoke_native_invalid_parameter_00bf6713();
    const auto captured_begin = word(header_address, 4);
    if (captured_begin > word(header_address, 8))
        invoke_native_invalid_parameter_00bf6713();
    std::uint32_t output[2];
    erase_native_string_header_vector_range_004954f0(header, 0u, output,
        header, pointer(captured_begin), header, pointer(captured_end), strings);
}

} // namespace bsp
