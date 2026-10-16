#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error These B4 leaves require MSVC Win32.
#endif

// Provisional mechanical interfaces over actual backing storage, not recovered
// class names or semantic C++ prototypes. No global cell/provider is invented.
// Three dword stores at +0/+4/+8; measured EAX result zero.
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b401a0(void* backing);
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b401b0(void* backing);
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b47670(void* backing);

// New meaningful ECX destination / EDX source interface. Original takes ECX
// destination, source from entry ESP+4, and RET4. Only dword +4 is transferred.
// The Source API does not reproduce the Original argument-cell/fault context.
extern "C" void* __fastcall bsp_tagged_b4_00b403f0(void* destination, const void* source);

// New meaningful ECX source / EDX destination interfaces; EAX is destination.
// Original obtains destination from entry ESP+4 and RET4. Source removes that
// stack read/cleanup. Ordered x87 FLD/FSTP data operations remain literal:
// four words at +390/+394/+398/+39c, or three at +38/+3c/+40. No snapshot,
// FP environment reset, recovered class or Original ABI adapter is supplied.
extern "C" void* __fastcall bsp_tagged_b4_00b40750(const void* source_backing, void* destination);
extern "C" void* __fastcall bsp_tagged_b4_00b40780(const void* source_backing, void* destination);

// One actual ADD [ECX+4],-1 instruction, including its RMW/flags/fault effects.
// No established semantic return; no zero guard or saturation.
extern "C" void __fastcall bsp_tagged_b4_00b40f10(void* backing);
extern "C" void __fastcall bsp_tagged_b4_00b47ec0(void* backing);
extern "C" void __fastcall bsp_tagged_b4_00b48040(void* backing);

// Physical one-word cleanup envelope only: no word is read or receiver assumed.
extern "C" void __stdcall bsp_tagged_b4_00b44bf0(std::uint32_t physical_unread_word);

// Single unaligned-safe raw dword observations at the measured offsets.
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b47660(const void* backing); // +4
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b47830(const void* backing); // +4
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b47840(const void* backing); // +0
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b47870(const void* backing); // +4
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b47880(const void* backing); // +0
extern "C" std::uint32_t __fastcall bsp_tagged_b4_00b478f0(const void* backing); // +30h
