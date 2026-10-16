#pragma once

#if defined(_MSC_VER) && defined(_M_IX86)
// New returning Source interfaces for complete 00BCEF40 bi_flush and 00BCEFC0
// bi_windup. The sole formal input is raw backing in ECX; each same-function
// MOV EAX,ECX prefix enters the full observed EAX-input/plain-RET schedule.
// The actual Source CALL frame supplies saved/return cells. Raw backing/output
// aliases can alter those cells; no all-pointer safe-return or full ABI promise
// follows. There is no borrowed ESP, fake argument/provider, clamp or snapshot.
// Source fastcall/void/noexcept are chosen interfaces, not recovered Original
// types or ABI. Correct library names and Original no_return=false are retained.
// Full widths, read/RMW/reset/POP ordering and limits are documented in
// docs/GOAL16AA_ZLIB_RAW_BITBUFFER.md; parents and game behavior remain unproved.
extern "C" void __fastcall
bsp_native_zlib_bi_flush_00bcef40(void* backing) noexcept;
extern "C" void __fastcall
bsp_native_zlib_bi_windup_00bcefc0(void* backing) noexcept;
#endif
