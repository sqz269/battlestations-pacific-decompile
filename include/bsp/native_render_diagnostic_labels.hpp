#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Full B13030. Original ECX=actual service, stack=actual source8h header,
// RET4. Destination is that same service's actual8h header at +684. Exact
// header identity returns; otherwise resize from the initial source length,
// then reread source length/data and destination length/data for the copy.
// The borrowed service/header/storage must remain valid across callbacks.
// No service construction, source ownership, null guard or local EH cleanup.
void set_native_render_diagnostic_label_00b13030(void* actual_service,
    const void* actual_source_header, NativeStringStorage&);

// Full B13510. Original no inputs, ECX unused, RET. Borrow the actual global
// F8D39C slot; initially null returns. Construct a fresh actual8h string from
// fixed "X", then reload this slot before selecting destination service+684.
// Capture temporary data/length for normal copy/release. Only the copy branch
// has an unwind guard; its cleanup reads the temporary's CURRENT header with
// 41DD20. Disarm before releasing the captured normal buffer. No second null
// check after construction, no retry or reset-to-empty policy.
void reset_native_render_diagnostic_label_00b13510(
    void* const volatile& actual_global_00f8d39c, NativeStringStorage&);

// New C++ entry interfaces, not drop-in thiscall/register replacements. The
// only owned temporary is the reset's native string; the full diagnostic
// service/singleton, statistics and system-time owners are not implemented.
// NativeStringStorage remains the existing sized-pool boundary. Its release
// is noexcept: this does not establish native pool-release exception behavior.

} // namespace bsp
