# Copied Text runtime adoption

`GuiTextRuntimeFactory::begin_copy_00aa1380` now prepares the nonnull-source
`00AA1380` branch on the same `GuiWidgetOwnerRuntime`, Text pool and render
ownership domains as the source. Its retained `GuiTextRuntimeCopyOperation`
allocates the native Text pool slot, composes canonical `00AA9520` base copy
with the existing `00ABB2C0` derived continuation, and admits one concrete
`GuiTextRuntimeImplementation`. It never runs the default Text constructor
on a copied destination. This is a conditional C++ runtime path, not a binary
replacement or an enabled complete `00AAB4C0` subtree clone.

The implementation owns exactly one `GuiTextLifetime`. The derived copy frame
and its service references are held in place in that implementation, whose
address stays stable when its `unique_ptr` transfers to the widget owner.
Strings, content, glyph children, primary model, shadow and cursor all remain
in the existing canonical lifetime/model domains. The factory's existing
allocation map still holds opaque pool transport only; no Text state or native
reference header is manufactured in that map or in the raw allocation.

## Evidence and sequence

Read-only Ghidra queries verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, port 8089. `00AA1380..00AA1403` has 40 listed
instructions and no flow gaps. `00ABB2C0..00ABB623` has 195 listed instructions
and no flow gaps. The last instruction of the latter begins at `00ABB621`
and occupies three bytes (`RET 4`). Neither end is a guessed next-function
boundary. The report records every CALL in these two bodies separately from
the type-switch tail jump and indirect callee dependencies.

The type-3 arm of `00AA6560` moves EDX to ECX at `00AA6581` and **jumps** to
`00AA1380` at `00AA6583`. Inside the factory, ESI captures source ECX at
`00AA1397`. `00AA139E` calls the existing `00AB79E0` Text pool allocator before
testing the source. With source and allocation nonnull, `00AA13B7` pushes the
source, `00AA13B8` puts destination allocation in ECX, and `00AA13BA` calls
`00ABB2C0`. The original returns the destination in EAX, with plain `RET`.
Allocation null reaches `00AA13F6` (`XOR EAX,EAX`) without construction.

The C++ operation preserves that allocation-before-base sequence. Base copy
uses the actual clone-flags table and current source Model `+10` route. For
Text, `D5C0B8[3]` is `0x3E`, with parent zero. The supplied base services must
provide that exact dependency; the Text auxiliary `0x26` model path is not a
substitute. The after-base lifetime admission requires the canonical owner's
completed-AA9520 predicate and distinct live source/destination primary models.

The existing copied lifetime performs the derived string/field sequence and
preserves the native unwritten-field validity boundaries. The continuation
then invokes `00AB8910` cursor construction, `00AB8530` section construction
and `00ABB1D0` content rebuild, in order. See
[GUI_TEXT_COPY.md](GUI_TEXT_COPY.md), [GUI_WIDGET_COPY_RUNTIME.md](GUI_WIDGET_COPY_RUNTIME.md)
and `reports/gui_text_copy.json` for their separately recovered bodies. This
packet composes those implementations rather than duplicating their fields,
render primitives or strings.

## Ownership and admission

The caller passes its sole fresh destination `unique_ptr` by reference to
`begin_copy_00aa1380`, with explicit base unwritten-field preimage and actual
copy services. Pure host validation and acquiring the source borrow precede
moving that pointer. Rejected preparation leaves the caller's destination
untouched. No native allocation occurs until `run_00aa1380` on the stable
retained operation.

The operation holds `GuiWidgetCopySourceBorrow` before allocation. The same
matching token admits only its canonical AA9520 preflight. External source
update, retirement and additional copy admission are blocked; direct Text
mutation also rejects the held borrow. This host guard is not a native AddRef.

After canonical base copy, the operation first retains an empty concrete
implementation shell and publishes its borrowed constructor dispatch before
allocating copied fields. It then creates the copied lifetime and its in-place
continuation. The shell's active-operation guard also protects the base owner
if copied-lifetime allocation throws before association. Thus allocation or a
provider exception cannot unwind a local
`unique_ptr` containing an already-published incomplete Text lifetime. The
operation and destination retain partial model creators and opaque pool
transport. Native SEH cleanup is not reconstructed here.

`begin_base_copy_type_admission` exposes a borrowed pointer to that same
implementation for native constructor dispatch. Only the actual active copy
call/resume scope allows Text current `+64` bounds and `00AB6BD0` height reads
against the copied lifetime. The owner rejects external operations throughout
this interval; direct Text mutation remains blocked. The constructor-read
scope ends before `finish_base_copy_type_admission`, so the final activity
preflight sees a completed continuation. Internal glyph/content operations
continue using the canonical owners and their established internal APIs.

Pending content remains in the copied implementation's exact outer submit /
content / child frames. `pending_content()` returns that same frame. Only
after its actual saved glyph-child operation completes may the caller use
`resume_after_glyph_child()`. Both the factory operation and generic Text
implementation resume enter the same retained factory caller. No prefix is
repeated. Completion installs the already-created implementation, clears its
operation borrow and immediately releases the source borrow. No second host
admission call is needed after generic Text resume.

A throwing native suffix changes the copy continuation and factory operation
to failed; acquired state remains inspectable and cannot be retried. Incorrect
after-child invocation is outside this entry's contract and is not permission
to skip the actual child. Destruction of a started pending/failed operation
terminates rather than silently discarding its frame or performing invented
rollback. The retained operation, destination, factory and resource domains
must remain alive for explicit intervention at that boundary.

`take_completed_layout()` transfers the same admitted layout and clears the
operation's implementation/allocation borrows. Holding or destroying the
completed operation thereafter does not touch the source or returned widget.
Allocation-null returns `nullptr`, never an unconstructed successful clone.
Completed Text uses the existing derived/base deletion and pool-return paths;
no default reconstruction or shared-pointer ownership replacement is involved.

## Required providers and validation limits

`GuiTextRuntimeCopyServices` requires the actual base-copy clone services and
`GuiTextCursorServices` from the same buffer, material, layout and live-constant
domains as the factory. `GuiTextCursorParameterOwnerServices` must implement
the real `00B18A40` retained Text binding: the original `00AB8BB3` push of one
requests retention of the actual Text object at `+04` / current `+00` terminal
dispatch. The current semantic owner count and opaque pool transport cannot
stand in for that object. Missing binding is an unresolved runtime dependency,
never successful null/no-op retention. This packet adds no fake raw header,
model fallback, or automatic renderer replacement.

The factory's existing null-source/default path remains separate. Its host
base-admission order, native strings/header layouts, native SEH/allocator
failure cleanup, and ABI-compatible `ECX`/EAX entrypoints are outside this
packet. The implementation's new read gate does not change the x87 bounds
kernel or its constant providers. A positive copied Text execution and its
internal bounds/child path remain unvalidated until an actual retained-Text
binding and the required current renderer/model dependencies are installed.

The operation explicitly captures canonical base-copy and `00AB8910` cursor
creator frames. Existing `ensure_gui_text_draw_sections_00ab8530` remains a
normal-return callee: its `create_mesh` and `append_default_section` creators
are local raw pointers, with no returned acquisition frame on exception.
The Text/model publications remain retained, but this packet cannot claim
complete callee-local creator recovery or full failure unwind across that
boundary. That buffer-owner exception work is a separate dependency packet.

Verification is recorded in `reports/gui_text_copy_adoption.json`. No new
permanent test or fabricated completed Text fixture was added. The primary's
separate source-borrow fixture covers matching-token base admission and an
actual Model-prefix interruption, not full copied Text execution. Build and
report-call checks are distinct from ABI, renderer and game validation.
The final Win32 Release `scripts/build.ps1` run passed, including the existing
`reconstructed_math` CTest (1/1). `verify_report_calls.py` checked all 14 CALL
rows with zero failures. Logs are retained under this worktree's ignored
`local/text_copy_adoption_*` paths; this test does not exercise copied Text.
