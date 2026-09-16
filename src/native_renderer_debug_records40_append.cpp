#include "bsp/native_renderer_debug_records40_append.hpp"

#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/native_renderer_record_guard.hpp"
#include "bsp/random_threads.hpp"
#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4 && sizeof(TrackedCriticalSection) == 0x1c);
static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);

std::uint32_t address(const void* value) noexcept {
    return reinterpret_cast<std::uint32_t>(value);
}
volatile std::uint32_t& word(const void* base, std::uint32_t offset) noexcept {
    return *reinterpret_cast<volatile std::uint32_t*>(address(base) + offset);
}
struct Guard {
    std::uint32_t profile;
    TrackedCriticalSection* section;
};
static_assert(sizeof(Guard) == 8);
} // namespace

void __fastcall append_native_renderer_records40_00b257b0(
    void* actual_header, std::uint32_t, const void* live_record40) {
    const auto capacity = word(actual_header, 8);
    if (word(actual_header, 4) == capacity) {
        const auto doubled = capacity + capacity;
        const auto requested = static_cast<std::int32_t>(doubled) > 1
            ? static_cast<std::int32_t>(doubled) : 1;
        reserve_native_renderer_records40_00b22a70(actual_header, 0, requested);
    }
    const auto used = word(actual_header, 4);
    const auto base = word(actual_header, 0);
    const auto destination = base + used * 0x28u;
    if (destination != 0) {
        // One live read/store pair at a time. Overlap and x87 exceptions make
        // a pre-copy, memcpy or DWORD/SSE float replacement inequivalent.
        __asm {
            mov eax, destination
            mov ecx, live_record40
            mov edx, [ecx]
            mov [eax], edx
            fld dword ptr [ecx + 4]
            fstp dword ptr [eax + 4]
            fld dword ptr [ecx + 8]
            fstp dword ptr [eax + 8]
            fld dword ptr [ecx + 0ch]
            fstp dword ptr [eax + 0ch]
            fld dword ptr [ecx + 10h]
            fstp dword ptr [eax + 10h]
            mov edx, [ecx + 14h]
            mov [eax + 14h], edx
            fld dword ptr [ecx + 18h]
            fstp dword ptr [eax + 18h]
            fld dword ptr [ecx + 1ch]
            fstp dword ptr [eax + 1ch]
            fld dword ptr [ecx + 20h]
            fstp dword ptr [eax + 20h]
            fld dword ptr [ecx + 24h]
            fstp dword ptr [eax + 24h]
        }
    }
    word(actual_header, 4) = word(actual_header, 4) + 1u;
}

void append_native_renderer_debug_record40_00b29330(
    void* actual_renderer, NativeRendererRecordGuardContext& guard_context,
    void* retained_owner, std::uint32_t x_bits, std::uint32_t y_bits,
    std::uint32_t width_bits, std::uint32_t height_bits, std::uint32_t color,
    std::uint32_t u0_bits, std::uint32_t v0_bits,
    std::uint32_t u1_bits, std::uint32_t v1_bits) {
    void* const owner = get_native_renderer_record_guard_00b25be0(guard_context);
    auto* const section = reinterpret_cast<TrackedCriticalSection*>(word(owner, 4));
    Guard guard{0x00ce37fcu, section};
    if (section) {
        EnterCriticalSection(&section->native);
        word(section, 0x18) = word(section, 0x18) + 1u;
    }
    // Native state0 starts after successful entry. It owns only this guard.
    try {
        volatile std::uint32_t record[10];
        record[0] = 0;
        if (retained_owner) {
            record[0] = address(retained_owner);
            InterlockedIncrement(reinterpret_cast<volatile LONG*>(
                address(retained_owner) + 4u));
        }
        void* header;
        __asm {
            movss xmm0, x_bits
            mov eax, color
            lea edx, record
            movss dword ptr [edx + 4], xmm0
            movss xmm0, y_bits
            movss dword ptr [edx + 8], xmm0
            movss xmm0, width_bits
            movss dword ptr [edx + 0ch], xmm0
            movss xmm0, height_bits
            movss dword ptr [edx + 10h], xmm0
            movss xmm0, u0_bits
            movss dword ptr [edx + 18h], xmm0
            movss xmm0, v0_bits
            movss dword ptr [edx + 1ch], xmm0
            movss xmm0, u1_bits
            movss dword ptr [edx + 20h], xmm0
            movss xmm0, v1_bits
            mov ecx, actual_renderer
            add ecx, 1d18h
            mov header, ecx
            mov dword ptr [edx + 14h], eax
            movss dword ptr [edx + 24h], xmm0
        }
        append_native_renderer_records40_00b257b0(
            header, 0, const_cast<const std::uint32_t*>(record));
        // Native state0 remains armed through the normal leave operation.
        if (section) {
            word(section, 0x18) = word(section, 0x18) - 1u;
            LeaveCriticalSection(&section->native);
        }
    } catch (...) {
        try {
            destroy_native_singleton_guard_00411ee0(&guard);
        } catch (...) {
            std::terminate();
        }
        throw;
    }
}
} // namespace bsp
