# Damageable fake-effect vector source closure

## Scope and evidence

This packet reconstructs the 10h-row vector used at damageable descriptor+8 by
`0087CA80`. The actual borrowed vector header is `{opaque, begin, end, capacity}`;
the first word is preserved. A row contains a float at +0, opaque DWORDs at +4
and +8, and a ref-counted owner at +C. The reader leaves the default row's +8
unspecified. The source copies that word and never supplies a guessed default.

The source and header are `src/native_damageable_fake_effect_vector.cpp` and
`include/bsp/native_damageable_fake_effect_vector.hpp`. Evidence comes from the
saved `bsp.gpr` `/battlestationspacific.exe` exports and installed PE instructions,
with the parent integrator's saved `0087C380` flow repair. Names below describe
observed behavior; they are not recovered original class/template names.

| Address | Complete source body | Original ABI |
| --- | --- | --- |
| 0087C920 | resize and incoming by-value owner disposal | ECX header; stack size, 10h owned row; RET14h |
| 0087C380 | insert with growth and two in-place branches | ECX header; stack iterator owner, raw position, count, row pointer; RET10h |
| 0087B8F0 | erase and returned checked iterator | ECX header; stack output, first owner/position, last owner/position; EAX output; RET14h |
| 008769B0 | checked allocation of 10h rows | ECX count, ignored EDX hint; EAX allocation; RET |
| 00878C40 | assign-fill | cdecl first, end, source row |
| 00878820 | forward assign-copy | cdecl first, end, destination; EAX advanced destination; three extra words ignored |
| 008788D0 | backward assign-copy | cdecl first, end, destination end; EAX retreated destination; three extra words ignored |
| 00879330 | forward copy wrapper | cdecl first, end, destination; EAX recomputed from original span |
| 00879870 | backward copy wrapper | cdecl first, end, destination end; EAX recomputed from original span |
| 008799F0 | uninitialized copy | ECX first, EDX end; stack destination and three ignored words; EAX new end; RET10h |
| 0087AB80 | uninitialized fill | ECX destination, EDX count; stack row and three ignored words; RET10h |
| 0087B5D0 | uninitialized-fill wrapper | ECX ignored header; stack destination, count, row; EAX new end; RETCh |
| 0087BC50 | uninitialized-copy wrapper | ECX ignored header; stack first, end, destination; EAX new end; RETCh |
| 0087B5B0 | release-range adapter | ECX ignored header; stack first, end; RET8 |
| 00878A60 | temporary/by-value row destructor | ECX row; RET |
| 0087A510 | vector length-error construction and throw | no inputs; no return |

`008788D0` and `00878A60` were discovered through actual direct/EH edges and
added to the leased packet with parent approval. `0087C5FC` is an internal
branch target, not a separate callable function. The repaired insertion listing
has 264 instructions and covers all 692 bytes, including the returning-free
tails at `0087C4FA` and `0087C52E`.

## Storage and lifetime details

The insertion temporary snapshots +0, +8, +4, then +C before touching the
container. +0 uses `MOVSS` and preserves raw signaling-NaN bits. Its owner is
retained even for insertion of zero rows, and is released at the normal exit.
The incoming by-value resize row instead consumes its existing reference.

Row assignment and row construction perform one ordered x87 `FLD`/`FSTP` pair
for +0, then two integer transfers. They are not `memcpy`: masked signaling
NaNs are quieted and set x87 status. Assignment captures source owner then old
destination owner, publishes the incoming owner, retains it, and releases the
old owner. Equal owners skip all refcount work. Construction first clears +C,
then reads source +C, so self-construction abandons the previous handle without
a release or retain. A null destination skips the current construction.

Refcounts are actual atomic DWORD operations at owner+4. A zero decrement calls
the captured owner's current vslot0 with ECX owner. The source reuses genuine
`0087AB30` row-range release and `0041DE40` slot release. `0087C260` remains the
existing whole-vector cleanup available to the reader/owner. There is no vector
provider callback, substitute owner object, guessed allocator or replacement
vtable. Canonical host CRT allocation/free, interlocked operations and returning
invalid-parameter dispatch are explicit external boundaries.

Capacity is the native unsigned 3/2-growth calculation with a `0FFFFFFF` row
bound. Pointer differences retain DWORD wrap and signed `SAR 4` semantics.
Zero-row allocation still enters the allocator. Overflow uses the canonical
host `bad_alloc` boundary. `0087A510` constructs the actual counted legacy
string including its NUL byte (`18` bytes), constructs `00411700`'s native
logic-error storage and sets `00D69260`. It reuses `NativeAliasListLengthError`
as the already-established owning transport for this identical length-error
payload. This is a host catch type and exception ABI; the class name does not
imply list semantics for the vector throw.

Growth copies the current old prefix, fills new rows, then copies the old
suffix. It captures the current old size before releasing old rows. After
callbacks it reloads begin for the free. Publication is capacity, end, begin,
in that order. The in-place short-tail branch constructs the moved tail before
filling at the current end, adds the inserted byte count to current end, then
assigns the gap. The long-tail branch constructs the last N rows at old end,
publishes the returned end, copies backward, and fills the gap. Erase copies
forward, releases the remainder using the reloaded current end, publishes end,
and only then writes the returned iterator. No callback-induced header changes
are repaired or normalized.

Resize retains the native sequence of invalid-parameter checks and subsequent
header reloads, including continuation when the boundary returns. Checked
iterator containers are only validated where the original does so. Insert's
iterator-owner word is ignored.

## Exception state and cleanup

`0087C380` installs handler `00C96818`, FuncInfo `00DC8C4C`, five states and two
catch-all maps. State0 unwinds through `00C96810` to `00878A60` for its temporary.
States1..4 unwind to state0 with no extra action. The growth catch covers state1
and enters state2 at `0087C518`: it releases only `[allocation, checkpoint)`,
frees allocation, then rethrows through `00BF6885`. The checkpoint advances
only after each helper returns, and never after the suffix copy. The short-tail
fill catch covers state3 and enters state4 at `0087C5A1`: it releases
`[position + N*10h, current_end + N*10h)`, then rethrows. Tail construction before
that protected fill has no corresponding catch cleanup.

`008799F0` and `0087AB80` install handlers `00C96651`/`00C96761` with FuncInfo
`00DC88C4`/`00DC8A44`, but keep their state at -1 throughout their actual bodies.
Their dormant funclet metadata therefore does not authorize invented partial
construction cleanup. `0087C920`'s handler `00C968E8`, FuncInfo `00DC8D78`, has
one active state, whose action `00C968E0` destroys the incoming row through
`00878A60`. Ordinary epilogues set state -1 before owner release; a throwing
release must not release that same owner twice. The source places these normal
epilogues outside its protected bodies.

The true `00C96810`/`00C968E0` unwind actions terminate if their row destructor
throws a second C++ exception. This differs from the explicit calls inside the
two catch bodies: those cleanup calls may propagate their own exception and
then unwind the outer temporary. The source preserves this distinction.

`0087A510` arms its string cleanup only after counted assignment returns;
`00C966B0` then dispatches `004072D0` on exception-object construction/throw.
Completed vector writes and owner releases remain visible after exceptions.
There is no transaction, rollback, or synthesized cleanup of unfinished rows.
The C++ interface expresses these source-level C++ exception boundaries. Native
FH3 personality/RTTI compatibility, asynchronous faults, longjmp and double
exceptions remain unvalidated.

## Validation

Validation results and source/binary pins are recorded in
`reports/native_damageable_fake_effect_vector_orch4.json`. The focused native
comparison runs original ordinary vector instructions relocated as a block,
with unchanged relative control flow, adjusted absolute interlocked IAT slots,
shared canonical allocation/free, actual interlocked operations and an
instrumented actual owner vslot0. It compares complete active row words,
normalized size/capacity, opaque header word, owner counts, release-publication
observation and x87 exception flags. It does not execute original FH3 exception
paths and is not a binary replacement or gameplay validation claim.
A separate host subprocess injects a throwing invalid-parameter boundary and a
second throw from the by-value row owner's vslot0. Its registered terminate
handler must run; this validates the source's true-unwind boundary only.
