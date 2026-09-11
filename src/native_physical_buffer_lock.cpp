#include "bsp/native_physical_buffer_lock.hpp"

#include <cstddef>
#include <cstring>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native physical buffer Lock requires MSVC Win32 DWORD accesses.
#endif

namespace bsp {
namespace {
// Actual addresses may be unaligned or partially overlap an output store.
// Each operation is one x86 DWORD access, without a C++ alignment assumption.
std::uint32_t load_word(const void* address) noexcept {
    std::uint32_t value;
    __asm {
        mov eax, address
        mov eax, dword ptr [eax]
        mov value, eax
    }
    return value;
}
void store_word(void* address, std::uint32_t value) noexcept {
    __asm {
        mov eax, address
        mov edx, value
        mov dword ptr [eax], edx
    }
}
void add_word(void* address, std::uint32_t value) noexcept {
    __asm {
        mov eax, address
        mov edx, value
        add dword ptr [eax], edx
    }
}
void* at(void* owner, std::uint32_t offset) noexcept {
    return static_cast<std::byte*>(owner) + offset;
}
std::int32_t signed_bits(std::uint32_t bits) noexcept {
    std::int32_t result;
    std::memcpy(&result, &bits, 4);
    return result;
}
void diagnostic(NativePhysicalBufferLockContext& context) {
    (void)native_diagnostic_sink_get_or_create_004c14c0(
        context.actual_diagnostic_0109cf14, context.actual_lifetime_01090aa0);
}
using ComLock = long (__stdcall*)(void*, std::uint32_t, std::uint32_t,
    void**, std::uint32_t);
static_assert(sizeof(ComLock) == 4);

void* lock(void* owner, NativePhysicalBufferLockContext& context,
    std::uint32_t bytes, std::uint32_t extra_offset, void* base_offset_output,
    std::uint8_t read_only) {
    const auto checked_offset = load_word(at(owner, 0x1c)) + extra_offset;
    if (signed_bits(checked_offset) > signed_bits(load_word(at(owner, 0x18))))
        diagnostic(context);

    void* data = nullptr;
    std::uint32_t flags = read_only != 0 ? 0x810u : 0x800u;
    if ((load_word(at(owner, 0x14)) & 0x1000u) != 0) {
        flags = load_word(at(owner, 0x1c)) == 0 ? 0x2000u : 0x1000u;
        if (extra_offset != 0) diagnostic(context);
        add_word(at(owner, 0x24), 1);
    }

    void* const com = reinterpret_cast<void*>(load_word(at(owner, 0x28)));
    if (com) {
        const void* const table = reinterpret_cast<void*>(load_word(com));
        const auto offset = load_word(at(owner, 0x1c)) + extra_offset;
        // The original captures the table before the offset read, but reads
        // the actual Lock target only afterward. HRESULT is discarded.
        const auto call = reinterpret_cast<ComLock>(
            load_word(static_cast<const std::byte*>(table) + 0x2c));
        (void)call(com, offset, bytes, &data, flags);
    } else {
        data = context.actual_null_buffer_sentinel_00f8d4b8;
        store_word(at(owner, 0x1c), 0);
    }

    const auto current_cursor = load_word(at(owner, 0x1c));
    store_word(base_offset_output, current_cursor);
    // Output may overlap flags/cursor/depth; each read or RMW is performed
    // after the output store, including metadata changed by the COM call.
    if ((load_word(at(owner, 0x14)) & 0x1000u) != 0)
        add_word(at(owner, 0x1c), bytes + extra_offset);
    add_word(at(owner, 0x20), 1);
    return data;
}
} // namespace

void* __fastcall lock_native_physical_index_buffer_00b4b850(
    void* owner, NativePhysicalBufferLockContext& context, std::uint32_t bytes,
    std::uint32_t extra_offset, std::uint32_t, void* base_offset_output,
    std::uint8_t read_only) {
    return lock(owner, context, bytes, extra_offset, base_offset_output, read_only);
}
void* __fastcall lock_native_physical_vertex_buffer_00b4ba00(
    void* owner, NativePhysicalBufferLockContext& context, std::uint32_t bytes,
    std::uint32_t extra_offset, std::uint32_t, void* base_offset_output,
    std::uint8_t read_only) {
    return lock(owner, context, bytes, extra_offset, base_offset_output, read_only);
}
} // namespace bsp
