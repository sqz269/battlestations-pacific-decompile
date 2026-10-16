#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Tagged B9-BB leaves require MSVC Win32.
#endif

namespace bsp {
extern "C" {
// Complete RET-only Native entries: no register/argument-cell input is read.
// Physical unread stack words describe cleanup only, not recovered declarations.
void __cdecl bsp_tagged_b9bb_00b94b20();
void __stdcall bsp_tagged_b9bb_00b96e50(std::uint32_t unread_word);
void __stdcall bsp_tagged_b9bb_00b9a1c0(std::uint32_t unread_word);
void __stdcall bsp_tagged_b9bb_00bb8ff0(
    std::uint32_t unread0, std::uint32_t unread1, std::uint32_t unread2);
void __cdecl bsp_tagged_b9bb_00bbb630();

// Full Native XOR AL,AL; RET10h. Upper EAX bits remain incoming bits.
std::uint8_t __stdcall bsp_tagged_b9bb_00bbb620(
    std::uint32_t unread0, std::uint32_t unread1,
    std::uint32_t unread2, std::uint32_t unread3);

// Actual single unaligned raw reads at wrapping ECX+offset. No class, ownership,
// null repair or Boolean conversion is inferred; byte results retain all 8 bits.
std::uint32_t __fastcall bsp_tagged_b9bb_00b9a3d0(const void* backing); // +BCh
std::uint32_t __fastcall bsp_tagged_b9bb_00b9a4b0(const void* backing); // +14h
std::uint8_t __fastcall bsp_tagged_b9bb_00b9a500(const void* backing);  // +10h
std::uint32_t __fastcall bsp_tagged_b9bb_00ba7170(const void* backing); // +08h
std::uint32_t __fastcall bsp_tagged_b9bb_00ba7180(const void* backing); // +30h
std::uint32_t __fastcall bsp_tagged_b9bb_00ba7190(const void* backing); // +44h
std::uint32_t __fastcall bsp_tagged_b9bb_00bbce40(const void* backing); // +94h
std::uint32_t __fastcall bsp_tagged_b9bb_00bbce50(const void* backing); // +08h
std::uint8_t __fastcall bsp_tagged_b9bb_00bbce70(const void* backing);  // +99h
}
// Formal Original types/classes remain provisional. Whole body identities do
// not establish original placement, caller adoption, binary or game validation.
} // namespace bsp
