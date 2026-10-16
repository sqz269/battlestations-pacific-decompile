#pragma once

#if defined(_MSC_VER) && defined(_M_IX86)
// New returning Source interface for complete 00BCF530 _tr_align. One real ECX
// backing input, no explicit stack args; the full named body uses actual saves,
// raw memory/partial shifts/RMWs, child CALLs and two plain RET exits.
// Each real AD public flush CALL gets ECX=current EAX first. Its current complete
// production body physically leaves EAX=backing beyond its void declaration;
// that machine contract is verified, not an invented C++ pointer return.
// Extra call prefixes/ECX at faults and Source code/return/current-frame placement
// differ from Original. No guards, snapshots, private kernel, borrowed ESP,
// noreturn or universal safe return/full Original ABI/caller/game promise.
// Formal types/layout remain provisional; see NATIVE_ZLIB_RAW_TR_ALIGN_BC.md.
extern "C" void __fastcall
bsp_native_zlib_tr_align_00bcf530(void* backing) noexcept;
#endif
