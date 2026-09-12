# Raw renderer gamma setter

`set_native_renderer_gamma_00b21960` reconstructs the complete 474-byte native
setter. The full raw power dispatcher, fallback, SSE2 implementation and error
providers now supply the dependency that blocked the earlier gamma discovery.
No host `pow` or restricted numerical replacement is used.

The original ABI is ECX renderer, one float in the actual callee argument slot,
RET4. The new source interface borrows that live four-byte slot by reference,
plus stable actual synchronization/power contexts and five separately relocated
native literals. It is not an original-address binary replacement.

Optional guard entry occurs before reading renderer+1B53. Cleanup arms after
that enabled-byte read. Disabled and ordered-equal requests skip generation.
Current mode is read before cleanup disarms, and normal guard leave follows
disarm. Exception cleanup invokes the complete actual B21110 guard provider;
a second C++ exception during cleanup terminates. The existing synchronization
domain excludes cleanup becoming enabled after entry skipped an uninitialized
guard record. Source private-frame/SEH/fault behavior remains qualified.

The 302-byte native math/device path is mapped instruction by instruction to
the 330-byte source helper. Its reserved/saved 628h frame preserves every ramp
and scratch offset. Added loads identify borrowed literals and the live input
slot; the extra power-context word is removed before native scratch accesses
resume. All other arithmetic and memory operations retain their widths and
order. The proof checks branch targets and every actual relocation.

In particular, the first x87 requested load remains on the stack for FADD10,
while a later MOVSS rereads the live slot for cache publication at +196C.
Normalization, lower x87 comparison, upper SSE comparison, reciprocal and
float spills remain explicit. Every ramp entry uses complete raw BFEB10 power,
a float32 result spill, multiplication by double65535, temporary truncation
control and signed DWORD conversion. It writes low WORD blue, then green,
restores the saved x87 control word, then writes red. No saturation or cache
rollback is added. Current calibration is compared before the current device,
table and SetGammaRamp selector are loaded. The method receives swapchain0
and the captured calibration flag; it has no HRESULT policy.

The strict Win32 build, two existing CTests and eight seed checks pass. The
actual compiler inputs, library and sixteen objects are pinned. All fourteen
power-chain objects match the earlier reviewed baseline except four COFF
header timestamps; every other byte, including executable code and metadata,
is identical. The final build after a header wording correction preserves
all sixteen objects' code, data, EH, symbols and relocations from the fixture
build; debug checksums are separately recorded.

One ad hoc fixture executes the captured native gamma body with eight relocated
data operands and one power call bound to the same complete raw source provider.
For requested values 0, 4 and -5, all 768 channel WORDs, floating-point status
and calibration flags match the reconstructed setter. The fixture selects the
raw x87 fallback under masked, 53-bit, nearest rounding controls. It separately
checks source equal/disabled skips and source C++ exception cleanup after a
capture-device call, retaining the already-published cache. No persistent tests
were added and no monitor gamma method was invoked: the fixture uses a local
COM-ABI capture object. The original native guard/EH paths and gamma's SSE2
power route were not executed by this fixture.

Independent review found no implementation mismatch within these contracts;
its wording correction about equal requests was applied. This closes the raw
gamma dependency under its explicit source bindings. Renderer recreation still
needs its remaining binding/cache ownership providers. Original caller ABI,
native FH3/SEH/private-frame aliases, hardware faults, monitor behavior, visual
correctness and full gameplay remain unproved. Exact evidence and hashes are
in `reports/native_renderer_gamma_audit.json`.
