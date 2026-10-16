#pragma once

#include <cstdint>

// Chosen MSVC Win32 transfer interface for complete 00BCF040 copy_block.
// Four real inputs select EAX backing, ECX length, EDX bytes and entry ESP.
// The caller owns the genuine current Source frame/SS protocol: DWORD[S] is
// its continuation and DWORD[S+4] is the actual mutable header/count cell.
// No fifth header argument, frame copy, snapshot, initialization or repair.
//
// The private body calls the entire public AD windup entry after MOV ECX,EAX.
// Its physical EAX and aliased EBX/ESI behavior is a pinned machine contract,
// not a consequence of the provider's void declaration. Raw addresses/counts,
// partial registers, real saves/POPs and continuation aliases remain admitted.
// Plain RET4 normally leaves the selected frame at S+8; this does not return
// through the public adapter's ordinary C++ frame. Source-only noreturn is
// never an Original/private-body annotation or an all-pointer safe-return ABI.
// Selecting ESP does not select SS. Intermediate ECX/fault state, Source
// return/code addresses, full Original ABI/SEH/parents/game remain qualified.
#if defined(_MSC_VER) && defined(_M_IX86)
extern "C" __declspec(noreturn) void __cdecl
bsp_native_zlib_copy_block_frame_00bcf040(
    void* backing, std::uint32_t length, const void* bytes,
    void* kernel_entry_stack) noexcept;

// Additional returning Source interface: ECX=backing, EDX=bytes, actual
// DWORD[T+4]=length, DWORD[T+8]=raw_header. The real current return is moved
// into the consumed length cell C=T+4; C+4 stays the mutable header/count.
// The same complete body and whole AD provider then execute on this frame.
// Intact RET4 normally leaves T+12. Raw aliases may corrupt saves, header,
// count or continuation; no universal safe return/nonvolatile guarantee is
// implied. Prefix stack accesses/ADD flags, DS/SS and Source code/return
// placement alter fault prefixes. This is no recovered Original fastcall,
// full ABI/SEH proof or parent adoption; the existing transfer remains valid.
extern "C" void __fastcall bsp_native_zlib_copy_block_00bcf040(
    void* backing, const void* bytes, std::uint32_t length,
    std::uint32_t raw_header) noexcept;
#endif
