# Race records and indexed configuration loading

Packet `orch4_race_config_g`, branch `agent/orch4-race-config-20260911g`,
2026-09-11. Verified target: `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Worker Ghidra access was read-only. Names below
are descriptive hypotheses, not recovered symbols. Source and interface:
`src/race_config.cpp`, `include/bsp/race_config.hpp`.

## Canonical record

Native race records are20h and carry vtable00D08D20. Its first three entries
are007FFB10 (scalar deleting destructor),007FFC00 (row loader),007FF190
(unfollowed consumer). `RaceRecord` preserves the native Win32 field layout:

| Offset | Field |
| --- | --- |
|00|native vtable address word|
|04|converted race index, used as an unsigned table index|
|08,0C|owned NativeString length/data|
|10,14,18,1C|four float color lanes|

`007FF9D0` writes only the vtable and the empty name header. Index and color
are untouched. The same constructor sequence is inlined at008002C7..008002D0.
The C++ shell requires `RaceRecordAllocationWords` (index word plus four color
words) and copies their bits without floating-point evaluation before the
recovered constructor runs. The loader overwrites index before publication;
Lua callbacks during row loading can still observe the initial color words.

`007FFC00`, ECX=record and LuaObject pointer on stack, `RET4`, is the actual
vtable+4 target. It gets `Name`, calls the real Lua string accessor, computes
zero length for a null result or strlen otherwise, resizes with preserve=false,
and copies into the current header. The name temporary is released before
looking up `Color`. Color entries1,2,3 are each looked up, converted through
`lua_object_number_00b66270`, stored to the corresponding field, and released
before the next lookup. After releasing entry3, alpha is set to1 from00D7A24C,
then the Color reference is released. Values are not divided by255 or clamped.
Missing Name clears the string; missing numeric components receive Lua's actual
numeric conversion behavior. Missing/non-indexable Color remains a Lua error.

`007FF9F0` resets the vtable and releases the current name allocation without
clearing its header. `007FFB10` does the same, then tests bit0 of the flags
argument and frees the record allocation when set, returning the original
pointer value even after free. These use actual NativeString and existing
singleton allocation/free primitives, with no implicit string destructor.

## Full Races.lua loader

`00800160` has no explicit native arguments and returns at008003A7 (`RET`,
one byte). Its158-instruction listing is complete. It constructs a temporary
Lua state, opens library mask1, constructs `Scripts\datatables\Races.lua`,
runs the file and its real VFS override sequence, and releases the path before
reading globals. Globals and `Races` references remain alive through iteration.

The table owner is00F87460: opaque allocator word+0, begin00F87464,
end00F87468 and capacity-end00F8746C. `RaceRecordTable` aliases the existing
`SingletonPointerSlots` type and supplies these actual pointer fields; it is
not a new owning map or a substitute vector of fabricated records.

For each actual Lua entry:

1. Convert the key using `lua_object_integer_00b66290`: Lua number, float32
   spill/reload, then the live CRT SSE2-mode test. SSE2 uses CVTTSD2SI; x87 uses
   the existing complete BF7456 kernel and retains low32. Interpret the result
   as an unsigned word, without clamping or validation against a new policy.
2. If begin is null or count<=index, resize to `index+1` with unsigned wrap,
   supplying a null pointer fill value.
3. Allocate20h through the real allocation primitive, construct the canonical
   record and write its converted index.
4. Reload the current table header. If its bounds check fails, invoke the
   required returning invalid-parameter policy and reload begin afterward.
5. **Overwrite table[index] with the new pointer without deleting the old one.**
   Then call the recovered canonical vtable+4 loader using that retained record.
6. Release iterator value/key and advance through actual Lua iteration.

At completion, release value, key, Races and globals, then close the Lua owner.
Existing records are not cleared before loading. Records omitted by a later
script survive. Distinct Lua keys that narrow to the same integer replace the
same slot in actual iteration order. Overwritten allocations remain alive and
can become unreachable, matching the native ownership behavior.

`RaceConfigContext` supplies NativeString storage, a reference to the live CRT
conversion-mode flag, explicit allocation words and existing validation policy.
No whole record-loader or race-loader callback remains. The valid fresh
constructor path fixes vtable00D08D20, whose slot+4 is concretely007FFC00; no
intervening callback changes it before dispatch on a valid bounds-check path.

## Pointer-table resize and growth

`00800090` is ECX=vector owner, count then a pointer **by value** on the stack,
`RET8`. The pseudocode omits that second argument; assembly preserves it by
passing its stack cell to007FFEC0. It grows at the captured end position,
does nothing for equal size, or erases the suffix for smaller size. Pointer
cells carry no retain, release or record destruction.

The supporting007FFEC0 control flow is reconstructed in `insert_slots`:
capture the fill pointer before reading the container; reject addition beyond
0x3FFFFFFF entries; grow capacity by old+floor(old/2), resetting that candidate
to zero on overflow and taking at least the required count. Allocate the new
pointer backing, copy prefix, fill inserted cells, copy suffix, and read the
current begin/end again to calculate final size. Free old backing **before**
publishing begin, capacity-end and end. Within capacity, preserve both native
tail-length branches, overlapping moves and fill order. It does not retain
record pointers or initialize unused capacity cells.

The code reuses real CRT `memmove_s`, `std::length_error`, and the existing
allocation/new-handler/free primitives. Pointer arithmetic uses Win32 address
words, including native SUB/SAR2 and DWORD wrapping, rather than undefined C++
subtraction between unrelated pointers. Returning validation callbacks retain
the native captured positions and later field reloads; no `at()` exception or
silent repair replaces the policy.

Helper evidence establishes the direct contracts:007FF340 is a checked4*count
allocation wrapper;007FFAB0 copies pointer ranges with memmove_s;007FFDD0 fills
count cells, rereading its source cell;007FF7F0 fills a range;007FF810 moves a
range backward;007FF910 erases a range and updates end;007FFE00 genuinely
constructs/throws length_error. These remain library primitive boundaries,
not claimed ports of CRT, generic STL checked iterators or exception classes.

`release_race_record_table_storage` is a host convenience that frees only the
pointer backing. Record owners call the concrete scalar deleting destructor
for records they still own. No automatic mechanism can reclaim overwritten
pointers that the caller did not retain; adding such ownership would change
the reconstructed loader.

## Pending saved-analysis repairs

The worker verified both continuations from live bytes and disk assembly but
did not change Ghidra. Root will repair and refresh exports after integration:

| Function | False CALL_RETURN site | Missing inclusive bytes | Continuation |
| --- | --- | --- | --- |
|007FFEC0|007FFFBC `_free`|007FFFC1..007FFFC3|83 C4 04, ADD ESP,4; then header publication|
|007FFB10|007FFB3E `_free`|007FFB43..007FFB45|83 C4 04, ADD ESP,4; then return this|

Final instructions are00800071 `RET10h` (three bytes) for007FFEC0 and
007FFB49 `RET4` (three bytes) for007FFB10. No missing function definition is
required. Other leased listings have no flow gaps. The report records current
listed counts separately from contiguous raw decoding and pending repairs.

## Validation and limits

Standalone MSVC Win32 Release build and existing reconstructed_math1/1 passed.
One ignored fixture (`local/race_config_fixture.cpp`) uses the real Lua5.1
interpreter/runtime with actual NativeString and pointer-table storage. Lua
`__index` callbacks invoke the real DoFile/VFS path to inspect newly published
records and each preceding color write. It passed float32 index narrowing
(3.99999999->4), x87 low32 conversion (2^32->0), sparse null-filled growth,
capacity1.5 growth and in-place growth, nullable Name, alpha1, overwrite/shrink
without pointee disposal, close order and balanced string storage. The recipe
`local/run_race_config_fixture.ps1 -Repository <tree>` can relink against the
parent's combined library. There are no permanent new tests.

Although the record field layout matches Win32 native20h, these interfaces do
not implement the native calling convention, executable vtable, LuaObject
stack/registry ABI, CRT allocator bookkeeping or MSVC SEH. Normal valid storage
and successful allocations are the verified domain. Callbacks must keep
retained record/position storage valid; malformed indices and callbacks that
clobber the fresh canonical vtable are outside that domain. Index conversion
is deliberately not clamped: negative/overflow results can trigger the native
length guard or returning invalid-parameter path. Signaling-NaN/unmasked trap
and exception-status equivalence are not claimed. No game-runtime or native
differential validation is asserted.

## Parent integration correction

The parent repaired both reported false CRT no-return gaps under owned leases and
the Ghidra write lock. Saved listings now have zero remaining call gaps. Prior
comments were preserved, new names/evidence read back, and exports refreshed.
See `reports/race_config_flow.json` and the `parent_ghidra` record in
`reports/race_config.json`. Worker-side pending-repair notes above describe the
earlier read-only snapshot. Native ABI and game validation remain unclaimed.
