#pragma once

// New MSVC Win32 raw-frame transfer interface, not a recovered Original
// declaration, ordinary returning cdecl helper, zlib layout/version or owner.
// Real backing/descriptor/entry-stack inputs become EAX/ECX/ESP, then a JMP
// enters the complete private gen_bitlen register body. DWORD[entry-stack]
// is the actual current continuation. No actual stack arguments are consumed
// by that kernel; its plain RET normally leaves ESP=entry-stack+4.
//
// Its 32-byte locals and four saved DWORD cells below the chosen frame remain
// real and aliasable, with all raw index/count/pointer bits and ordered RMWs.
// No frame, descriptor, local or continuation is copied, initialized or repaired.
// Source-only noreturn describes transfer through this chosen Source frame,
// never Original no-return. Ordinary return/consumer/adoption/SEH/full Native
// ABI/game proof remains outside this interface. See the committed doc/report.
extern "C" __declspec(noreturn) void __cdecl
bsp_native_zlib_gen_bitlen_frame_00bcdf60(
    void* backing, const void* descriptor, void* kernel_entry_stack) noexcept;
