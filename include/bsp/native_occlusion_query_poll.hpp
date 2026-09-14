#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native occlusion query poll requires MSVC Win32.
#endif

namespace bsp {

// Complete B5FCA0..B5FCD3 (52 bytes). ECX is the actual query wrapper;
// bool is returned in AL, plain RET. No projected owner or added context.
// Accessed owner words: +08 state, +0C samples, +10 IDirect3DQuery9 pointer.
// This is an accessed prefix, not an allocation-size or complete-layout claim.
// Original D62AD0 profile slot+10 selects this entry; no profile check is added.
//
// Captures current COM+10 once. Nonnull performs exactly one stdcall
// GetData(query, owner+0C, 4, 1) through its current vtable slot+1C, even if
// owner state is nonzero. Only exact S_OK stores state2 and returns true;
// pending/error return false. GetData writes to actual samples are preserved
// on every result. Null COM returns true without touching any owner word.
// No wait, retry, COM retain/release, error translation or EH frame is added.
// Live owner/query/vtable and writable native accesses must remain valid.
bool __fastcall poll_native_occlusion_query_00b5fca0(void* actual_owner);

} // namespace bsp
