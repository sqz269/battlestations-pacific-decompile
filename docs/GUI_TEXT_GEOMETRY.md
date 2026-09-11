# Actual mapped Text glyph writes

Address reconstructed: partial `00AB98F0`. Read-only consumers: `00AB9FD0` and
`00ABA270`. Names are hypotheses. Project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe` was verified for every live evidence batch.

| Routine | Original ABI and body | Coverage |
| --- | --- | --- |
| `write_gui_text_quad_00ab98f0_fragment` | ECX Text; ten stack DWORDs, `RET 28h`. Body `00AB98F0..00AB9FC2`; final `RET 28h` at `00AB9FC0`, length 3; early return at `00AB9CD8`, length 3 | partial: mapped writes/matching through `00AB9D32` and ordinary return behavior; optional Text-child continuation `00AB9D33..00AB9FAD` excluded |

This adapter consumes the same `GuiTextLifetime` used by content, style and
destruction. It writes directly through the actual logical stream's mapped
pointer `+08`, with the producer-established stride `+0C`, position offset
`+10`, UV offset `+28`, packed-color offset `+34` or float-color offsets
`+38/+3C/+40/+44`. Current vertex count `+64` supplies the supported capacity.
The caller passes the actual six-index slice returned by index mapping.
There is no new vertex snapshot, duplicate Text state, implicit map, upload,
material owner or renderer callback.

The existing x87 `write_font_quad_00ab98f0_fragment` kernel writes positions,
UVs and white colors into these native mapped bytes. Parameters use the
current canonical Text `font_scale` (`+1D8`) and the supplied live vertical
scale `00E12FD4`. Glyph bearing is signed16; width and height are unsigned16.
The supplied quad index and first vertex remain separate. Six local scalar
index results are published into actual mapped index memory in native order
`1,0,3,2,4,5`, retaining low16 wrap. Native arguments 5 and 6 are not consumed
by the covered body and receive no invented meaning in the new interface.

After writing, the native optional branch checks current `+1B0`, `+1AC`, the
same widget owner's `+DC`, and substitution string `+1A4`. Scanning stops at
the first NUL. A nonmatching or disabled branch returns `complete`. A match
zeros all four UV pairs in actual mapped storage, reloading stream writer
fields for each vertex, then returns `needs_glyph_child` at native `00AB9D33`,
immediately before the pool setup and allocation call at `00AB9D38`.

That result is a partial continuation, not successful completion of the whole
native glyph function. Real Text allocation/constructor, child-vector append,
scene-model copy, font/material setup, recursive content update, attach and
positioning remain required. The header and report mark this boundary. Callers
must stop or implement that continuation; silently drawing the zero-UV quad
would omit the native child glyph.

Valid nonaliasing zero-offset mapped storage is required, matching the observed
Text builder calls. Existing scalar-kernel capacity/layout rejection occurs
before stores. This does not emulate native invalid pointer/overflow faults,
floating trap timing, concurrent mapped-storage mutation or original ABI.
The adapter does not normalize unusual floats or add finite-domain guards.
Actual logical mapping is implemented separately in
`native_logical_buffer_mapping.hpp/.cpp` over existing actual physical owners.
Because index Lock can run callbacks after vertex Lock, the glyph helper reads
current vertex `+08` when it writes, not a pointer captured before those calls.

Producer evidence is `00B61E20` for writer stride/attribute offsets and
`00B4BC00` for current vertex count. Existing `FontGlyphData` is the scalar
payload projection whose decoder establishes bearing/advance/width/UVs;
the canonical lifetime supplies Text substitution fields and widget listener.
The helper borrows all of them without introducing another glyph map or owner.

Both direct caller sites were inspected with exact function attribution:
`00ABA1F8` in `00AB9FD0` and `00ABA730` in `00ABA270`. Each passes glyph,
pen, actual vertex writer, current 12-byte index slice, two local stack
addresses, quad number, font-height lowword, first vertex and code unit; the
callee removes all ten slots with `RET 28h`. Complete-function register filters
show the writer in EAX before the optional branch, ESI first vertex, EBP the
fourth vertex, and EDI restored to Text after each color store. Both consumers
advance first vertex by 4 and index address by 12 after the call.

The two consumers themselves are not implemented by this packet. Their
existing scalar kernels remain available, but full actual builders also need
the canonical live font/resource association: native glyph `+18/+1C` texture
identities, first-glyph material bindings, mutable metrics/string behavior,
section counts/layout rebuild and optional children. At the time of this
packet `FontGlyphData` intentionally omitted those resource pointers and
`FontResources` held semantic texture wrappers. A separate font-owner packet
is establishing that association; this implementation supplies no placeholder.

Validation: both new translation units compile with MSVC Win32
`/std:c++17 /W4 /WX /O2 /fp:strict`, including the canonical lifetime header
from the sibling packet. The report records exact live call verification;
covered glyph stores themselves are leaves. No new tests, native differential,
runtime or visual result is claimed. Text factory/remaining virtual behavior
and renderer resource creation are still needed for a reachable executable
path. The integrator performs the standard combined build.
