#include "bsp/native_renderer_debug_records24_append.hpp"
#include "bsp/native_renderer_record_guard.hpp"
#include "bsp/native_renderer_container_lifetime.hpp"
#include "bsp/native_diagnostic_sink_lifetime.hpp"
#include "bsp/random_threads.hpp"
#include <cstddef>
#include <exception>

namespace bsp {
namespace {
using Word = std::uint32_t;
struct NativeGuard { Word profile; TrackedCriticalSection* section; };
static_assert(sizeof(NativeGuard) == 8);
static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);
volatile Word& word(void* base, std::size_t offset) noexcept {
    return *reinterpret_cast<volatile Word*>(static_cast<unsigned char*>(base)+offset);
}
} // namespace

__declspec(naked) void __fastcall append_native_renderer_records24_00b25750(
    void*, std::uint32_t, const void*) {
    __asm {
        push esi
        mov esi, ecx
        mov eax, dword ptr [esi+8]
        cmp dword ptr [esi+4], eax
        jne capacity_ready
        add eax, eax
        cmp eax, 1
        jg capacity_selected
        mov eax, 1
    capacity_selected:
        push eax
        call reserve_native_renderer_records24_00b229d0
    capacity_ready:
        mov eax, dword ptr [esi+4]
        mov ecx, dword ptr [esi]
        lea eax, [eax+eax*2]
        lea eax, [ecx+eax*8]
        test eax, eax
        jz copied
        mov ecx, dword ptr [esp+8]
        fld dword ptr [ecx]
        fstp dword ptr [eax]
        fld dword ptr [ecx+4]
        fstp dword ptr [eax+4]
        fld dword ptr [ecx+8]
        fstp dword ptr [eax+8]
        fld dword ptr [ecx+0ch]
        fstp dword ptr [eax+0ch]
        mov edx, dword ptr [ecx+10h]
        mov dword ptr [eax+10h], edx
        mov ecx, dword ptr [ecx+14h]
        mov dword ptr [eax+14h], ecx
    copied:
        add dword ptr [esi+4], 1
        pop esi
        ret 4
    }
}

void append_native_renderer_debug_record24_00b29270(void* renderer,
    NativeRendererRecordGuardContext& context, const void* sphere,
    void* camera, Word selector) {
    void* const guard_owner = get_native_renderer_record_guard_00b25be0(context);
    auto* const section = reinterpret_cast<TrackedCriticalSection*>(word(guard_owner,4));
    NativeGuard guard{0x00ce37fc,section};
    if (section) {
        EnterCriticalSection(&section->native);
        word(section,0x18) = word(section,0x18)+1u;
    }
    volatile Word record[6];
    Word radius_bits;
    Word captured_selector;
    // Native state is still -1 during these reads. The radius MOVSS is captured
    // before state0 and stored afterwards. MOVD preserves its bits without FP
    // conversion across the source compiler's exception-state bookkeeping.
    __asm {
        mov eax, sphere
        lea edx, record
        movss xmm0, dword ptr [eax]
        mov ecx, camera
        movss dword ptr [edx], xmm0
        movss xmm0, dword ptr [eax+4]
        movss dword ptr [edx+4], xmm0
        movss xmm0, dword ptr [eax+8]
        movss dword ptr [edx+8], xmm0
        movss xmm0, dword ptr [eax+0ch]
        mov eax, selector
        mov captured_selector, eax
        movd radius_bits, xmm0
        mov dword ptr [edx+14h], ecx
    }
    // Capture source values before native state0, just as the original argument
    // loads/header computation precede B292EF. Context/private stack aliases are
    // outside this source ABI; no argument is reloaded after append callbacks.
    void* const header = reinterpret_cast<void*>(reinterpret_cast<Word>(renderer)+0x1d0cu);
    try {
        __asm {
            lea edx, record
            movd xmm0, radius_bits
            mov eax, captured_selector
            movss dword ptr [edx+0ch], xmm0
            mov dword ptr [edx+10h], eax
        }
        append_native_renderer_records24_00b25750(
            header,0,const_cast<const Word*>(record));
        if (section) {
            word(section,0x18) = word(section,0x18)-1u;
            LeaveCriticalSection(&section->native);
        }
    } catch (...) {
        try { destroy_native_singleton_guard_00411ee0(&guard); }
        catch (...) { std::terminate(); }
        throw;
    }
}
} // namespace bsp
