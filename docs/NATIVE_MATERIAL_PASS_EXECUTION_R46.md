# Native material pass execution

The raw pass layer adopts five bodies from historical `a6b2884f2`, whose
integrated pass evidence is recorded at `d91f24d3`:

| Entry | Native span | Source role |
|---|---|---|
| B5E5E0 | 3 bytes | Base virtual08, exact RET4 |
| B454D0 | 15 bytes | Derived virtual08, same entry and override zero |
| B44750 | 948 bytes | Geometry, stream, effect plane and index binding |
| B43410 | 600 bytes | States, shaders, textures, constants, draw and diagnostics |
| B1BFA0 | 4 bytes | Borrow current queue context at +30 |

The two leaves live in the same source file as the pass layer. They retain
their literal native instructions. Existing current providers supply every
substantive child, including B42350; no linker stub or successful fallback is
introduced. This packet does not adopt the B2BB90 debug-record parent.

## Actual domains and diagnostic frames

All contexts borrow the same application publication cells, renderer,
synchronization, logical buffers, layouts, textures, raw strings, queue,
material owners and diagnostic storage. The pass's VS/PS bank pointers must
be exactly the constant builder's banks at 0108EBF4/0108DBEC, including the
live header DWORD immediately preceding each bank. Copying either bank or a
publication cell creates a different, unsupported domain.

Each dispatcher invocation needs a distinct fresh pass frame referencing its
own persistent constant-builder frame. The dispatcher rejects any non-fresh
pass frame before native effects. This is an explicit source-domain admission
rule; the original executable has no such frame, and the historical source
also admitted completed frames. Internal helpers form the admitted invocation
and do not reset or replace their referenced child frame.

Reached failures retain prior native writes and the failed frame. There is no
implicit rollback, child acknowledgement, retry, release or cleanup disarm.
An unsupported current numeric profile fails at the reached dispatch. A raw
material callback must be an actual callable pointer; the register trampoline
preserves the recovered ECX/EDX/EAX/EBX/EBP/ESI/EDI inputs and adds no arguments.

## Current-source adaptations

- Current shader setters take `(renderer, &synchronization, logical_shader)`.
  Their native stacked logical argument and current renderer capture remain.
- The effect-plane direction call uses the existing raw no-normalize kernel
  on actual scene+F0 float storage. It does not construct a matrix overlay.
- B434D7 reloads the selector, B434DB captures the material pointer, and
  B434DE skips the material+34 count read when that selector is negative.
  The historical source eagerly read the count; this adoption keeps the gate.
- B435C0/B435EE capture the section primitive count before the renderer's
  current draw-slot check. Earlier source performed these two reads in reverse.

The remaining native schedule keeps captured versus current renderer/section
references, texture-loop masks and selectors, system-register count reloads,
x87/SSE effect-plane arithmetic, material callback registers, draw-count
wrapping, and diagnostic publication capture order.

## Evidence limits

All five native bodies and the six supporting profile/jump-table spans are
freshly compared between verified live Ghidra and the installed PE. The full
B44750/B43410 listings are reviewed against the adopted source. The report
separates inherited historical evidence from this corrected source.

The restored ignored fixture executes only the copied original base RET4 and
queue-context getter against the current source leaves. It checks volatile
registers, flags and stack cleanup for RET4, and both nonnull and null current
queue slots. The derived dispatcher and its complete genuine provider chain
are forced into the link; they are not executed by this fixture. Loaded module
paths and I386 headers are checked in the same probe process.

Strict Win32 /MD /W4 /WX /fp:strict build, the three existing CTests, linked
object evidence and the frozen compiler/library/source closure are recorded
in the report. Full derived-pass execution, original private-stack/FH3/SEH
identity, complete renderer composition and gameplay remain unvalidated.
