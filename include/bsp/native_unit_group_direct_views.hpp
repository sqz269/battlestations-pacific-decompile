#pragma once
#include <cstdint>

namespace bsp {
// Complete raw Win32 ECX entries; unused EDX preserves native stack spelling.
// Borrow actual aligned 508h group storage and genuinely valid indexed record
// backing. Index*34h wraps in 32 bits; neither indexed routine checks count.
// The pointer-valued member word is returned unchanged, without dereferencing it.
void* __fastcall native_unit_group_member_at_0070d060(
    const void* actual_group, void* unused_edx, std::uint32_t index) noexcept;
void* __fastcall native_unit_group_record_at_0070d070(
    void* actual_group, void* unused_edx, std::uint32_t index) noexcept;

// FLD actual group+504h to ST0, plain RET: at least one free x87 slot is
// required. This is caller-retained float input; the current Source constructor
// leaves it unchanged. The separate 0070DA00 calculation remains external.
float __fastcall native_unit_group_retained_speed_0070d0f0(
    const void* actual_group, void* unused_edx) noexcept;

// Stack = member pointer, exact float DWORD bits; RET8. Signed count at4F8
// is reread each iteration. Nonpositive count exits before loading XMM0;
// positive count must fit genuinely valid records. Update every matching
// record+30h, including duplicate/null words, with MOVSS and no float coercion.
// Original class ABI/lifetime, synchronization, producers and game are external.
void __fastcall publish_native_unit_group_member_speed_0070d100(
    void* actual_group, void* unused_edx,
    void* actual_member, std::uint32_t speed_bits) noexcept;
} // namespace bsp
