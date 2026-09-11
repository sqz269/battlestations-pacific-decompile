# Point-effect entry-array mutations

`00867210` removes an entry by replacing it with the current tail; `00867320`
appends one borrowed reference. The complete implementations use the existing
`PointEffectReferenceArray` and canonical reference helpers in
`src/point_effect_entry_array.cpp`. Its three words are backing pointer, signed
count, and signed capacity. Entries borrow their actual owners' atomic `+04`
counts through `RenderCommandReference`; the C++ interface is not a native
vtable/layout replacement.

Evidence is from saved project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and the installed executable with SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
`reports/point_effect_array_mutations.json` records byte hashes, original calls,
the fixture's scope, and descriptive names saved in Ghidra.

## Unordered erase: `00867210`

The complete 131-byte body is `00867210..00867292`, including `RET 4` at
`00867290`. Native ECX points to the array header. The single stack argument
points to an iterator slot, whose value is the entry-slot address. The routine
does not advance or overwrite that iterator. No stable return value is
established. Proposed name: `BSP_PointEffectReferenceArray_EraseUnordered`.

The implementation preserves the following instruction ordering:

1. Capture the iterator value at `00867214`, then capture count and backing
   pointer and derive the initial tail address at `00867227`.
2. If target and tail addresses differ, load the tail value before loading the
   old target value. Identical values do nothing. Otherwise publish the tail to
   the target at `0086723B`, retain it via `InterlockedIncrement` at `00867243`,
   and release the old target at `00867251`; count zero calls its current
   virtual `+00` at `0086725D`.
3. Reload count and backing pointer at `0086725F/00867262`, after any old-target
   terminal reentry. Capture the newly current tail slot and value. Release
   that value at `00867274`, and call its current virtual `+00` at `00867280`
   if the count reaches zero.
4. Clear the captured tail slot at `00867282` after that callback. An already
   null tail is not written. Finally decrement the then-current header count
   at `00867288`; there is no predecrement or final overwrite from an earlier
   saved count.

This is not ordered erase or vector pop-back semantics. A callback may replace
the header between the two releases, and the second callback may change the
count or captured slot. The native routine deliberately observes/overwrites
those changes in the order above. The C++ function is nonthrowing under the
existing canonical terminal contract. A nonempty valid array and iterator,
and valid storage across callbacks, are native preconditions; no bounds or
corrupt-header repair is added.

Indexed callers are `008673B0` and `00867790` (effect child update). Caller
relationships establish code use, not gameplay execution.

## Append: `00867320`

The complete 115-byte body is `00867320..00867392`, including `RET 4` at
`00867390`. Native ECX points to the array header. Its single stack argument
points to a borrowed source reference slot. No stable return value is
established. Proposed name: `BSP_PointEffectReferenceArray_Append`.

Growth occurs only when current count equals capacity. The doubling at
`00867343` wraps as a DWORD, then its signed result is used only if greater
than one; otherwise requested capacity is one. The sole direct call is
`00867350 -> 008670A0`, the existing complete reserve implementation.

After reserve returns, `00867355/00867358` reload count and storage and capture
the destination. Native wrapping address arithmetic is retained, including
the null-destination branch. For a nonnull destination, write null at
`00867365` **before** loading `*source` at `0086736B`. If nonnull, publish that
loaded reference and retain it at `00867377`. Increment the then-current count
last at `0086737D`. There is no source retain on entry or consumed-input
cleanup. If the source is the destination, the source read sees the null just
written; if reserve invalidates a source slot, the caller has violated its
required source lifetime. The implementation does not silently snapshot it.

The body registers handler `00C94E4C`, which selects FuncInfo `00DC6C9C` and
one-state map `00DC6C94` (`state 0 -> -1`, action `00C94E30`). That funclet
derives the current end and calls `00401130` at `00C94E43`. The complete normal
body sets its EH state to `-1` on entry and contains no instruction arming
state zero. No active local cleanup is inferred from the map alone. Ordinary
C++ reserve failure propagates before append's destination/count stores;
native exception dispatch has not been executed by the fixture.

Indexed callers are `00867790`, `008679D0`, and `00867B10`.

## Validation and limits

`local/extract_point_array_ag.py` verifies the project/program with the CLI,
checks the installed EXE hash, compares every live byte with disk, and checks
complete disassembly coverage. It records both full bodies, every return, and
the append EH metadata/code in `local/point-array-byte-evidence-ag.json`.

`local/run_point_array_probe_ag.cmd` compiles the changed implementation with
MSVC Win32, `/O2 /W4 /WX /fp:strict`, and links the focused probe with
`/MANIFEST:EMBED`. The probe passes against both complete original byte
bodies, checks stack balance after `RET 4`, and compares normalized headers,
all fixture slot identities, borrowed actual atomic counts, and terminal
observations. It covers:

- A first terminal callback replacing the array header, followed by a second
  terminal callback replacing the captured tail value and changing the count.
  The final clear/decrement and both callbacks' observations match native.
- Duplicate-reference erase and direct tail erase.
- Append's destination/source alias, which discards the prior destination
  value without retaining or releasing it, then appends null.
- Empty-to-one and full-one-to-two growth through the existing actual reserve
  implementation and real backing allocation/free boundary.

The original imports are redirected to real Win32 interlocked operations.
The fixture's native owners expose a diagnostic terminal that records and
mutates state; the C++ companions borrow those same `+04` words. During native
append's direct reserve call, an explicit adapter converts slot identities to
canonical companions, calls the real reconstructed reserve, and converts the
result back. It neither substitutes a fake reserve nor claims the reserve's
original bytes were executed in this probe. The parent integration records
the required combined Win32 build/CTests separately. No native EH dispatcher,
physical event destruction, application loop, or gameplay validation is
claimed by this focused array fixture.
