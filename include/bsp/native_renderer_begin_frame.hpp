#pragma once

#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
struct NativeRendererResetProcessContext;

// Actual checked iterator: owner DWORD, record pointer DWORD. The vector's
// prefix+0 is unused here; begin/end/capacity are +4/+8/+C. The actual 6ACh
// F8D39C service embeds this vector at +68C (producer B14A10/B0F020).
struct NativeRendererRecordIterator {
    void* owner;
    void* record;
};

struct NativeRendererBeginFrameContext {
    NativeRendererResetProcessContext& reset;
    void* const volatile& actual_service_00f8d39c;
    NativeStringRawPoolContext& strings;
};

// Complete B13180..B1327D: native ECX destination, stack source, RET4;
// EAX destination. Copy the opaque 11Ch prefix by ordered DWORD load/store,
// then assign the two actual8h string headers at +11C/+124, in that order.
// Exact self-header guards and current post-resize field reads are retained.
void* assign_native_renderer_record_00b13180(void* destination,
    const void* source, NativeStringRawPoolContext&);

// Complete B10740..B107C2: native ECX record, RET, no semantic result.
// Release captured second-string data/size, then current first-string pair.
// Each release performs the current concrete 419CC0 lookup even for large or
// disabled-small returns. If second release throws, destroy the CURRENT first
// header (CBBC80); no first-header cleanup after its pointer capture/disarm.
// Native bytes are not cleared, nor are opaque payload fields destroyed.
void destroy_native_renderer_record_00b10740(void* record,
    NativeStringRawPoolContext&);

// Complete B13280..B132B7 and B13630..B13687. Both are native cdecl, six
// caller DWORD slots, plain RET; only first/last/output are consumed. Ignored
// iterator/tag slots and indeterminate tag padding are absent in this C++ API.
// Copy forward until first==last. B13630 ignores the copy result and returns
// output + 300 * (signed32(wrapped last-first) / 300), with DWORD wrapping.
void* copy_native_renderer_records_00b13280(const void* first, const void* last,
    void* output, NativeStringRawPoolContext&);
void* copy_native_renderer_record_tail_00b13630(const void* first, const void* last,
    void* output, NativeStringRawPoolContext&);

// Complete B14480..B144FD: native ECX vector and five DWORD stack slots
// (out, first.owner, first.record, last.owner, last.record), RET14, EAX out.
// Owner validation tests first.owner nonnull/equal last.owner, without a new
// comparison to vector. The fixed CRT invalid-parameter service may return.
// Capture end before copying, reread end after copying, destroy ascending tail,
// publish new end only after destruction, then write out.owner/out.record.
void* erase_native_renderer_records_00b14480(void* vector, void* output_iterator,
    NativeRendererRecordIterator first, NativeRendererRecordIterator last,
    NativeStringRawPoolContext&);

// Complete B15090..B150CC: native ECX actual service, RET, no result.
// End capture precedes first validation; begin is reread before the second
// comparison to CURRENT end. Clear preserves the prefix and capacity words.
void clear_native_renderer_records_00b15090(void* actual_service,
    NativeStringRawPoolContext&);

// Complete B2B200..B2B263: native ECX actual renderer, RET, AL1 only.
// Active+1998 skips all work. Otherwise publish active1, decrement captured
// nonzero inhibit+1D90, run full Reset, conditionally call CURRENT device's
// BeginScene+A4, then reread actual F8D39C and clear its records if nonnull.
// Ignore BeginScene HRESULT. Exceptions preserve all preceding writes.
std::uint8_t begin_native_renderer_frame_00b2b200(void* actual_renderer,
    NativeRendererBeginFrameContext&);

// All reached storage must be valid; address arithmetic follows Win32 DWORD
// wrapping. Borrow the SAME actual pool publication/gate/raw manager used by
// native_render_diagnostic_labels. No second service/pool, semantic release
// callback, lifecycle policy, rollback, general STL port or new CRT is added.
// These source contexts are not original thiscall/cdecl/EH binary replacements;
// C++ cleanup is covered, original FH3/SEH/concurrency/gameplay is not inferred.
} // namespace bsp
