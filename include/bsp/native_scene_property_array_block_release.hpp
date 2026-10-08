#pragma once

namespace bsp {
// Whole 008F03F0..008F0417: ECX is the actual eight-byte array header at
// record+20h: owned current-CRT pointer, then BYTE size. Preserves ESI and
// returns with plain RET. Only these two header fields are written.
void __fastcall release_native_scene_property_array_block_008f03f0(
    void* actual_array_header) noexcept;

// This leaf does not construct/destroy the surrounding 38h record or dispatch
// its type. The data must be owned by the same current heap used by the genuine
// allocator/free provider. Historical CRT and nested bag lifetime are unbound.
} // namespace bsp
