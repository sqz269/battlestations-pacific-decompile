# Actual material-pass copy and secondary-pass creation

Addresses: 00B41C10, 00B41CE0, 00B41DD0, 00B41E80, 00B44690,
00B455C0, 00B45E00.

This packet composes the actual pass/base/state/string/pool owners to implement
the complete copy path and the effect's secondary-pass builder. It preserves
two unusual native behaviors: independent state allocations receive a plain
copy of the source reference count, and bindings are appended to the existing
destination count. These functions do not perform a whole-object assignment.
Names are descriptive hypotheses, not recovered symbols. The C++ interfaces
target MSVC Win32 and are not original ABI replacements.

## Complete functions and original ABI

End addresses below are exclusive. All seven stored Ghidra bodies already
cover these complete spans, without internal flow gaps.

| Function span | Behavior | Original ABI |
| --- | --- | --- |
| B41C10..B41C9D | Copy actual eight-byte render rows | ECX destination header, stack source; EAX destination; RET 4 |
| B41CE0..B41D83 | Copy actual twelve-byte third rows, role provisional | Same |
| B41DD0..B41E73 | Copy actual twelve-byte sampler rows | Same |
| B41E80..B41EF0 | Copy actual eight-byte pair rows | Same |
| B44690..B4474C | Construct actual 10h binding from word/owner/name | ECX fresh binding, stack word/owner/header; EAX binding; RET Ch |
| B455C0..B45896 | Copy selected state and append bindings | ECX actual destination88h, stack source88h; RET 4; no stable result promised |
| B45E00..B45EE0 | Build effect's mode-zero secondary pass | ECX actual178h effect; RET |

## Actual row and binding operations

The three state-row helpers first call their matching reserve(0) when the
destination capacity is negative, reduce a positive current count to zero,
store zero, and reserve using the **current source count**. The pair variant
uses B40E00 resize(0) before its reserve. Reserves use the previously verified
actual twelve-byte headers and shared heap.

Each iteration captures its source row before possible destination growth.
If current destination count equals capacity, it grows to the signed result
of twice the old capacity, clamped to one. It then copies two or three DWORDs
in order, increments the current destination count, and reloads the signed
source end. No detached vector or precomputed end replaces these live fields.
The third/sampler functions have identical instructions after their reserve
call operands normalize.

There is no self-assignment guard: an aliased header is cleared before its
source count is read, leaving count zero. The fixture executes that behavior
for all four variants. Valid readable extents and allocations remain caller
contracts; native corrupt-pointer/overflow failures are not reproduced as
host memory corruption.

B44690 constructs the binding's actual empty string at08, clears retained04,
stores word00, and copies the supplied actual header unless it is the same
header. Resize requests the source length; the subsequent nonzero-source
test, source pointer, destination pointer and copy length are read after that
call. Copy length is the current destination length, and the terminator comes
from resize. It then captures the current retained04, skips equal identity,
or publishes the incoming owner, increments its actual +04 and releases the
captured old owner. String callbacks can have changed retained04 before this
last operation. Its one-state unwind, DF7CF0/CBF3A0, releases only name08
through 0041DD20. Its caller owns the raw binding allocation.

## Selected pass copy, including reference-count copying

B455C0 first copies pairs24. In order18 (render),20 (sampler),1C (third),
it captures/releases the old state owner, clears the slot **after** its
callback, allocates and initializes a new actual14h owner, and publishes it.
Only then does it reread the source slot, copy the current source DWORD04
into the new owner's physical atomic04 with a plain store, and copy its rows.

This is an independent allocation, not a shared state-owner pointer. A source
count of two produces a destination count of two; the code does not normalize
that value to one. The native instruction fixture verifies this counter copy.
Composed pass cleanup then leaves one reference on each source/destination
state owner in this case. The fixture explicitly releases the source's held
reference and the copied surplus during teardown; production reconstruction
does not silently repair or reinterpret the original behavior.

The host registers each newly published state companion, without changing
ownership, after its counter copy and before row-copy allocation. This uses
the caller's existing canonical owner domain and must not throw or modify
native storage. There is no second counter or private child registry.

The function next assigns retained54, retained58, vertex70 and pixel74 with
equal-identity skip or publish/increment/decrement ordering. Each source field
is read at its own call site. It copies indices78/7C, then walks the live
unsigned source binding count. Each source binding's word, owner and name
header address are captured before allocating16 bytes. B44690 constructs the
new binding; it is appended at the **current destination count6C**, then that
count increments. Earlier destination bindings remain. Finally the current
source borrowed14 identity is stored without ownership operations.

Root words/count/vtable, fallback84, byte80, padding and embedded row headers
30/3C/48 are not copied by this function. The packet does not impose a
whole-pass identity shortcut. Valid extents must remain within four binding
slots; self-copy with a nonempty binding list keeps growing the native end
and is outside that valid bounded contract.

There is no top-level rollback. Copy FuncInfo DF7E80 has one state at
CBF4A0: free the current raw binding allocation through BF65AC if its
constructor fails. Earlier state replacements and completed bindings remain.
The binding constructor's own unwind cleans its string. These two scopes
must not be replaced with whole-pass rollback.

## Secondary-pass publication and failure scopes

B45E00 walks all fourteen secondary slots of the actual effect. Only index0
can create a pass, and only if primaryC8 is currently nonnull. It allocates
from actual0108FBF8, constructs the full88h payload, rereads primaryC8,
publishes the new pointer at100, and calls B455C0. It then sets render states
F=0,1B=1,13=5,14=6 and root word08=1. Every override and the final store read
the current secondary100 again. Indices1..13 are zeroed; absent primaryC8
zeros all fourteen slots. Previous secondary owners are not released here.

The host registers the completed published pass in the same canonical owner
domain before copy can throw. Its original count1 is owned by the effect's
slot; registration adds no retain. The construction, copy and terminal path
use one shared lifetime context.

Secondary FuncInfo DF7F38 has one state, CBF580, which returns an unconstructed
raw slot through B41040/B40A40. That state is disarmed before publishing100
and calling copy. Thus constructor failure returns its raw slot; copy failure
preserves the already-published pass and does not zero later secondary slots.
Caller cleanup must use that actual published owner. The C++ port preserves
these scopes without emitting the original SEH encoding.

## Verification and remaining boundaries

Thirteen current live/installed-PE spans cover all seven functions plus three
unwind code/map pairs. The three one-state maps and cleanup targets were
independently verified. Seven selected names/comments are saved, read back,
and force-exported; prior annotation state is retained. No function creation
or flow repair was required in this packet.

One ignored fixture executes all seven complete original functions, with prior
frozen base/state, full pass constructor and pool-allocation dependencies.
Metadata entry adapters register canonical companions without changing actual
owners before continuing into complete original row-copy/pass-copy bodies.
Original heap/Windows/string targets are rebound to current shared domains.

Paired copy snapshots cover 1,584 source and 1,624 destination bytes; secondary
snapshots cover 1,584 and 1,600 bytes. They include the actual136-byte payload,
actual child headers/counts/rows, embedded row contents and binding data/name
bytes, with pointer identities normalized. Checks cover count2 copying,
binding append and distinct string buffers, preserved fallback/root fields,
four row-copy/self-clear pairs, secondary overrides and absent-primary zeroing.
Composed actual effect-owner cleanup dispatches both canonical passes, their
children and actual pool returns. Surplus state counts are observed before
explicit fixture teardown.

Two rebuilt failure cases distinguish constructor failure before publication
from binding-copy failure after publication, including the remaining secondary
slot preimages. Original native exception delivery was not executed. Original
virtual0 callbacks use relocated adapters that restore numeric profiles and
dispatch the canonical companion over the same already-zero atomic.

The strict Win32 build and both existing CTests pass. No permanent tests were
added. Generic actual context owners stand in for retained texture/shader
payloads in the fixture; real texture/shader loading, native exception ABI,
drawing and gameplay remain unvalidated. `reports/native_material_pass_copy.json`
pins source, captures, reused evidence and frozen tested build artifacts.

## Follow-up

The effect loader B45EE0 can now compose complete pass copying and secondary
creation with the established actual effect/pass/pool lifetimes. Its descriptor
loading and per-mode shader compilation remain to be recovered. B46950 policy
and the currently undefined B469A0 effect method also remain separate work.
Check current leases and named-but-incomplete dependencies before continuing.
