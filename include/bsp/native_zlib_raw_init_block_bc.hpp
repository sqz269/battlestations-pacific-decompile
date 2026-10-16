#pragma once

#if defined(_MSC_VER) && defined(_M_IX86)
// New returning Source interface for whole 00BCDE20 init_block (110B/29ops).
// Source consumes ECX backing; a same-function MOV EDX,ECX precedes the complete
// Original EDX-input body. Original consumes no ECX or explicit stack argument.
// Actual ESI save/control cells, raw WORD/DWORD writes, wrap and faults remain
// real. Extra prefix/code/ECX fault preimages/current-frame placement differ.
// No recovered class/formal return, universal safe return/nonvolatile/full
// Original ABI, SEH, caller adoption or game validation is promised.
extern "C" void __fastcall
bsp_native_zlib_init_block_00bcde20(void* backing) noexcept;
#endif
