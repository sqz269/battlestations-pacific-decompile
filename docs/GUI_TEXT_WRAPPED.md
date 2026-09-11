# Actual wrapped Text builder

Reconstructed addresses: `00ABA270`, partial `004768D0`. Names are hypotheses.
Ghidra remained read-only in `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, verified by the repository wrapper per batch.

| Routine | Original ABI and exact body | Coverage |
| --- | --- | --- |
| `build_gui_text_wrapped_00aba270` | ECX Text; UTF16 wrapper and actual draw section on stack; `RET 8` at `00ABA8C4`, length 3, inclusive end `00ABA8C6` | partial projection: all initialized position formats through both unlocks; optional child callee `00AB9D33..00AB9FAD`, undefined position formats and uninitialized out-of-domain horizontal alignment remain pending; native string pool/SEH ABI excluded |
| `read_gui_text_float3_position_004768d0_fragment` | ECX logical vertex stream; out float3 and index on stack, native EAX returns out pointer; `RET 8` at `00476B3E`, length 3, inclusive end `00476B40` | partial: `004768D0..004768DF` and `00476B18..00476B40`; excludes packed/scale-bias path `004768E0..00476B17` |

The builder operates on the SAME `GuiTextLifetime`, retained widget primary
model, canonical font association, actual mesh section, logical streams and
material resource domain. There is no second Text state, placement vector,
texture wrapper, synthesized scene node or fallback renderer. Required services
are the already existing concrete font/mapping/owner domains, plus existing
Text-child clear lifecycle calls, the live vertical-scale global and a borrowed
actual `NativeD3dx9Float16Import`. Only getter type16 invokes that library import;
the supplied d3dx9_40 module stays loaded while its binding and calls are active.

The caller's source is a reference to its actual transformed `std::u16string`.
Child clear `00AB80C0` runs first. Current signed16 font height is then captured,
and source is assigned only when it is a different string object from Text's
live string. Typed UTF16 allocation replaces native `004C53E0` pool storage;
it preserves valid null-free content and same-object avoidance, not native
allocation address, pool callback timing, terminator-copy faults or SEH.
The enclosing content continuation must stay live until this builder and its
pending callees finish. Text string backing must not relocate while borrowed
native cursors are active, just as for the original native call.

The actual primary model's `+180` geometry is selected, then mesh stream0
(`+64`). Vertex mapping requests current string length times4; index mapping
reloads length and current mesh index stream and requests length times6. Actual
mapping functions preserve their existing guard/physical owner behavior. The
quad writer reads the CURRENT stream mapped pointer when it writes, including
changes from index Lock callbacks. Actual section `+10/+18` ranges receive
current length times4/times2 initially, and low16 emitted-quad count times4/
times2 at the end. There is no added 16384-glyph clamp or integer saturation.

Current font is reselected per scanned and emitted code unit. The cached space
glyph pointer comes from the font captured before container conversion. Initial
font height stays captured for this invocation. The first emitted glyph binds
actual `+18` texture to current section material slot0, then reloads that SAME
glyph's `+1C` and CURRENT section material for slot1. Resource-release callbacks
may change either; subsequent font/scale reads remain live. Only after both
bindings does the builder publish Text `+18C` first pen x.

Horizontal scanning retains native leading-space and LF handling, greedy
overflow/backtracking, saved last-space width and unsigned slack subtraction.
The saved width uses `CVTTSS2SI`, including its masked-invalid result. Container
width uses the actual x87 product by double960, temporary truncating control,
signed64 `FISTP`, and low DWORD. The previous control word is restored. No
finite guard, division-by-zero substitute or host progress/capacity clamp is
inserted. Justification with one counted space still executes signed integer
division by zero as native does. Modes outside0..3 suspend at `00ABA53A`:
the native can consume an uninitialized space step, so no replacement origin
or space width is invented. Invalid-pointer/backtracking faults and unmasked
FP trap timing across C++ helper boundaries are not emulated.

For non-space glyphs, pen x advances by current scale times glyph advance, but
emitted line width accumulates the unscaled advance. Literal spaces use the
computed space step for both. Both values spill to float at native arithmetic
boundaries. Measured width is updated only if the emitted line is greater;
this routine does not initialize it. Line count is reset once, then increments
with DWORD wrap. Line spacing keeps `(distance + double1 - double0.15000000596)
* captured_height + y`, with no algebraic simplification. Bounds use native
x87 ordered-greater tests; unordered values therefore follow the update arms.
Even empty input reaches sentinel-bound subtraction and finalization.

The final raw extent spills before it is written to live Text `+178`. Division
by double720 remains extended through the single live vertical-scale sample;
normalized height spills before subtracting it from current widget height
times that same scale. Center halves the result; bottom retains it; other
vertical modes keep zero. Each final vertex is read through the complete
initialized-format getter and written back as native float3 with current
stride/position offset/mapped pointer using DWORD address arithmetic. This
final store is still direct float3 even when the input used a packed format.
Index Unlock precedes vertex Unlock.
Wrapped has no final `00B865A0` call; that is a single-line builder operation.

The builder now calls `read_native_vertex_position_004768d0` from the concrete
[position reader](NATIVE_VERTEX_POSITION_READ.md). It supports null `+50` and
all initialized nonzero-metadata types2/3/5/7/8/10/12/13/16, including the
actual packed9-bit and D3DX half-conversion cases. The getter reloads current
`+18/+50` after decoding before applying scale/bias. The older zero-metadata
helper in this file remains an unused compatibility API; it no longer limits
the wrapped builder.

Only undefined nonzero-metadata formats now return
`undefined_position_format`. Native reaches uninitialized scratch for those
values, so there is no native-defined decoder continuation to resume. The
frame retains both mappings and current index without output stores at that
index. Raw height, offset and final section ranges have already been
published. No resume entry retries this state or reruns those effects; the
child-only resume explicitly rejects it. Unsupported data is never treated
as completed geometry and does not silently unlock the streams.

The optional result is a borrowed native CALLER frame, retaining the selected
glyph, current UTF16 cursor, pen/line endpoints, counters, bounds, saved height,
cached space glyph, section and both actual mapped streams. No destructor
unmaps or releases them. `glyph_child` means the actual writer already stored
geometry and blanked matching UVs, then stopped at `00AB9D33`; caller advances
at `00ABA735` only after the real child tail has completed. The current writer
does not expose its own internal callee locals. That callee frame and actual
child factory/lifetime remain a prerequisite: the caller continuation alone
must not be used to reconstruct those locals after mutable callbacks.
The two extra native pointer arguments alias the same position local as arg2.
That SAME float3 has a stable allocation in the caller frame, distinct from
pen x/y, and survives moves into outer continuations. The optional child tail
can use all three aliases; the current ordinary writer does not receive those
additional slots. Consume a pending frame once.
Never run the content post-builder material/color/shadow tail while pending.

Call evidence and cleanup are recorded in `reports/gui_text_wrapped.json`.
The sole wrapped call is `00ABAB6A` within `00ABA8D0`: EDI section is pushed at
`00ABAB60`, source wrapper at `00ABAB67`, ECX Text from ESI at `00ABAB68`.
All18 getter call sites in nine functions were inspected; they pass current
stream, local output and either literal or caller-supplied index, with `RET8`.
No universal Text-only stream contract is inferred from those other callers.
Complete-function register filters establish EDI Text, ESI's changing roles
(string, stream, scan cursor, first vertex), EBP section/saved-width/glyph and
EBX source/mesh/current cursor. Native addresses denote evidence, never callable
game virtual addresses in this implementation.

Validation: strict MSVC Win32 `/std:c++17 /W4 /WX /O2 /MD /fp:strict` compile
passed. Source consumes current sibling/parent owner headers without copying
them. Exact report-call verification is recorded separately. No new tests,
game/native differential run or visual result is claimed. Native Text factory,
remaining virtual operations, resource loading and optional child lifecycle
still prevent this newly reconstructed actual-owner path from being reached
by the current executable. The primary integrator performs the combined build.
