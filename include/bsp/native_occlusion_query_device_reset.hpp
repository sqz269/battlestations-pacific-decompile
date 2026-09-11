#pragma once

namespace bsp {

// B5FE20: ECX actual owner, RET. Captures COM+10, releases once when nonnull,
// then clears current owner+10 only after that call returns. No other writes.
void release_native_occlusion_query_for_reset_00b5fe20(void* actual_owner);

// B5FE60: ECX actual owner, RET, no native device argument. The second source
// argument is the ADDRESS of the actual four-byte F8D394 publication cell.
// Reads its current renderer and actual B1FEF0 device, then CreateQuery(9)
// writes directly to owner+10. No old-query cleanup or added HRESULT/null gate.
void restore_native_occlusion_query_after_reset_00b5fe60(
    void* actual_owner, const void* actual_renderer_publication_00f8d394);

} // namespace bsp
