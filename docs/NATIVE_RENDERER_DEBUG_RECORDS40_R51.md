# Native renderer debug records40

`draw_native_renderer_debug_records40_00b2b580` recovers the complete
1,549-byte B2B580..B2BB8C body from assembly. No historical reconstructed
parent existed. It borrows the current generated-model, logical mapping,
camera, system-gather, render-entry and material-pass providers.

The caller must have bound exactly `context.cameras.pool_0108ffb0` through
the existing static camera-pool adapter before invocation. B71930/B71350 use
that private canonical binding; camera scalar cleanup uses the environment's
pool. The current API exposes no read-only identity accessor. Preparation
neither verifies this equality nor creates/binds a pool; it remains an explicit
application-domain precondition, outside the isolated fixture's evidence.

## Native schedule

An initial zero renderer+1D1C count returns without reading context or frame.
Nonzero input requires a distinct prepared persistent frame. The cold path
constructs `debugshader.mshd`, `pf43cc.mvfm`, and `2DSprites`, creates section
kind4 with zero initial counts, publishes model+19E8, destroys the local strings
in reverse order, then writes the current cached model's identity transform.
The raw model overload uses the same AA8/AA4/AA0 cells and `actual_names=null`.
The node constructor copies the name; temporary headers need only survive it.
An empty call leaves a supplied frame untouched; the caller explicitly cancels
any unused preparation. Mapping retains its actual CF14 diagnostic cell and
the same raw AA0 lifetime domain; pass diagnostics share AA8/AA4/AA0 as well.

The lock request saves the original count times six with DWORD wrapping.
Vertex expansion then reloads the live record count and data pointer. After
unlock, section+18 receives the current count times two, while section+10 gets
the earlier saved lock count. No clamp, capacity check or count undo is added.

A temporary camera comes from the already-bound actual 0108FFB0 pool. Its
existing owner/reference companions occupy caller-owned persistent frame
storage and share the existing node, viewport and actual render-entry domains.
B71490 receives identity view; B6FD60 receives identity projection; B46A70
receives null scene and that same actual camera.

The current 0108FE88 owner is captured once, its +08 count is atomically
incremented, and +04 supplies the actual 28h entry bank. B51A20 receives the
captured model/geometry, separately current section, camera, visibility1,
depth0, leading0 and flags0. The current section's material/effect/pass chain
dispatches virtual08 using the distinct fresh pass/constant frame. A returning
normal path unlinks/releases the camera, handles negative record capacity via
B22A70, drains the current positive count, and finally writes zero.

## Exact vertex interior and the R48 correction

Each 28h source row produces **six 10h vertices, 60h output bytes**, not six
18h vertices. B2B751 initializes ESI=output+18h; B2B763/B766 advance ECX and
ESI by60h per row. The resulting XYZ/color groups begin at offsets00,10,20,
30,40,50. XY order is left/bottom, left/top, right/bottom, right/top,
right/bottom, left/top. Six distinct color loads at B2B810/81D/82A/837/844/851
reload current renderer+1D18 before reading record+14. No extra attribute pair
exists. The primary corrected the R48 report after independently checking this.

The source fragment retains all 36 vertex x87 instructions, original temporary
spill offsets, ordered stores and loop branches. Before a nonnull owner release,
B2B878/B87B pop both constants. After the actual decrement/returning terminal,
FLD1 and a fresh D7A308 load restore the pair, including the final FXCH.
Record+00 clears only after this path returns. Valid source/output alias effects
remain; aliases into the new context, helper scratch or source call frame are
outside this interface. The parent's six entry-argument x87 rows are also kept,
including the bank/model reads between FLD and FSTP.

Opaque record owners have no recovered concrete type. At zero reference count,
the explicit binding matches the current token, reads the actual current
profile's virtual0, checks its asserted original identity, and invokes the
supplied genuine terminal. It never treats a numeric executable address as host
code. Unsupported bindings fail after the decrement; no fabricated release,
successful fallback or record clearing occurs.

## Construction failure and later retention

FuncInfo DF5CEC and unwind map DF5D10 identify the camera allocation state:
state3 invokes CBD488 -> B71350 raw-slot return. State4 first executes CBD490's
conditional camera-name cleanup, then state3. The concrete B71A80 constructor
performs its own reached cleanup. The source retains a published viewport's
identity when the native constructor leaves it live before returning the slot.

B2B99E sets state-1 before normal name destruction. All later failures retain
the camera and earlier effects. There is no outer stream unlock, entry-count
rollback, cached-model release, record clear, camera release or drain on failure.
Frame preparation reserves only host registration credits. Explicit retirement
requires actual terminal cleanup and host quiescence; it performs no native
recovery. The original null camera fault becomes a reached source-domain error.

## Validation limits

The report records fresh live-Ghidra/PE body, constant, literal and EH receipts,
strict Win32 build, existing CTests, exact object review and the frozen actual
compiler/library/source closure. The compact ignored fixture compares copied
original/source vertex output and effects under four rounding modes, using a
real nonzero decrement path and a valid source/output alias case. One copied
full-parent empty case checks the null-context early return. Its forced full
provider link does not execute the active parent or a zero-reference terminal.

Current application publication/startup and the record append producer remain
separate work. Source ABI, private scratch, host admissions and exception
machinery are explicit adaptations. Full parent execution, original FH3/SEH,
renderer composition, visual output and gameplay remain unvalidated.
