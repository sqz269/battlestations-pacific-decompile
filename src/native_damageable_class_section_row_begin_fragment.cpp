#include "bsp/native_damageable_class_section_row_begin_fragment.hpp"
#include "bsp/native_crt_memset.hpp"
#include "bsp/native_damageable_section.hpp"
#include <intrin.h>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error DamageableClass row initialization requires MSVC Win32 instruction access.
#endif

namespace bsp {

NativeDamageableClassSectionRowBeginFragment::NativeDamageableClassSectionRowBeginFragment(
    void* descriptor, NativeDamageableClassSectionRowBeginFragmentScratch& scratch,
    const NativeDamageableClassSectionRowBeginFragmentAccess& access) noexcept
    : descriptor_(descriptor), scratch_(scratch), access_(access) {}

NativeDamageableClassSectionRowBeginFragment::~NativeDamageableClassSectionRowBeginFragment() noexcept {
    if (state_ == 14) {
        state_ = 13;
        destroy_native_damageable_section_00878ef0(scratch_.fresh_section_at_parent_a8,
            access_.actual_vector_access.actual_vtable_00d0df04);
    }
}

void NativeDamageableClassSectionRowBeginFragment::run() {
    void* const temporary = scratch_.fresh_section_at_parent_a8;
    fill_native_crt_bytes_00bf79f0(temporary, 0, 0x30, access_.actual_feature_word_0109eea4);
    const void* const default_cell = access_.actual_default_float_cell_00ce38b8;
    // CE12/CE1D/CE26 are one MOVSS load and two stores, with no x87 transfer.
    __asm {
        mov eax, default_cell
        mov edx, temporary
        movss xmm0, dword ptr [eax]
        movss dword ptr [edx+28h], xmm0
        movss dword ptr [edx+2ch], xmm0
    }
    void* const header = reinterpret_cast<void*>(reinterpret_cast<std::uint32_t>(descriptor_) + 0x18);
    state_ = 14;
    append_native_damageable_section_0087c870(header, temporary, access_.actual_vector_access);

    void* captured_owner;
    unsigned char captured_null;
    __asm {
        mov eax, temporary
        mov edx, dword ptr [eax+24h]
        mov captured_owner, edx
        test edx, edx
        setz captured_null
    }
    _ReadWriteBarrier();
    state_ = 13;
    _ReadWriteBarrier();
    const std::uint32_t table = access_.actual_vector_access.actual_vtable_00d0df04;
    __asm {
        mov eax, temporary
        mov edx, table
        mov dword ptr [eax], edx
    }
    if (captured_null != 0) return;
    auto* const count = reinterpret_cast<volatile long*>(
        reinterpret_cast<std::uint32_t>(captured_owner) + 4);
    if (_InterlockedDecrement(count) == 0) {
        using ReleaseOwner = void (__thiscall*)(void*);
        ReleaseOwner current_release;
        __asm {
            mov eax, captured_owner
            mov edx, dword ptr [eax]
            mov eax, dword ptr [edx]
            mov current_release, eax
        }
        current_release(captured_owner);
    }
    __asm {
        mov eax, temporary
        mov dword ptr [eax+24h], 0
    }
}

} // namespace bsp
