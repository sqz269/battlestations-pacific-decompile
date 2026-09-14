#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uint32_t>(value);
}
void* pointer(std::uint32_t value) noexcept {
    return reinterpret_cast<void*>(value);
}
std::uint32_t load(const void* base, std::uint32_t byte_offset) noexcept {
    return *reinterpret_cast<const volatile std::uint32_t*>(address(base) + byte_offset);
}
void store(void* base, std::uint32_t byte_offset, std::uint32_t value) noexcept {
    *reinterpret_cast<volatile std::uint32_t*>(address(base) + byte_offset) = value;
}
void copy_float(void* destination, const void* source) noexcept {
    // Keep the load/store conversion, including signaling-NaN quieting and the
    // current x87 environment. A DWORD copy or SSE move is not equivalent.
    __asm { mov eax, destination }
    __asm { mov edx, source }
    __asm { fld dword ptr [edx] }
    __asm { fstp dword ptr [eax] }
}

template<std::uint32_t Stride, std::uint32_t FloatMask>
void reserve_records(void* header, std::int32_t requested) {
    if (requested < 1) requested = 1;
    if (static_cast<std::int32_t>(load(header, 8)) >= requested) return;
    const std::uint32_t bytes = static_cast<std::uint32_t>(requested) * Stride;
    void* const fresh = singleton_lifetime_allocate(
        {SingletonAllocationKind::object, bytes, bytes});
    std::uint32_t index = 0;
    if (static_cast<std::int32_t>(load(header, 4)) > 0) {
        do {
            const std::uint32_t row_offset = index * Stride;
            const std::uint32_t destination = address(fresh) + row_offset;
            if (destination != 0) {
                const std::uint32_t source = load(header, 0) + row_offset;
                for (std::uint32_t word = 0; word < Stride / 4; ++word) {
                    const std::uint32_t offset = word * 4;
                    if ((FloatMask & (1u << word)) != 0) {
                        copy_float(pointer(destination + offset), pointer(source + offset));
                    } else {
                        store(pointer(destination), offset, load(pointer(source), offset));
                    }
                }
            }
            ++index;
        } while (static_cast<std::int32_t>(index) <
            static_cast<std::int32_t>(load(header, 4)));
    }
    // Read current old base after copies, then publish only after free returns.
    singleton_lifetime_free(pointer(load(header, 0)));
    store(header, 0, address(fresh));
    store(header, 8, static_cast<std::uint32_t>(requested));
}
} // namespace

void __fastcall reserve_native_renderer_records16_00b228b0(
    void* actual_header, std::uint32_t, std::int32_t requested_capacity) {
    reserve_records<0x10, 0x007>(actual_header, requested_capacity);
}
void __fastcall reserve_native_renderer_records20_00b22940(
    void* actual_header, std::uint32_t, std::int32_t requested_capacity) {
    reserve_records<0x14, 0x00f>(actual_header, requested_capacity);
}
void __fastcall reserve_native_renderer_records24_00b229d0(
    void* actual_header, std::uint32_t, std::int32_t requested_capacity) {
    reserve_records<0x18, 0x00f>(actual_header, requested_capacity);
}
void __fastcall reserve_native_renderer_records40_00b22a70(
    void* actual_header, std::uint32_t, std::int32_t requested_capacity) {
    reserve_records<0x28, 0x3de>(actual_header, requested_capacity);
}

} // namespace bsp
