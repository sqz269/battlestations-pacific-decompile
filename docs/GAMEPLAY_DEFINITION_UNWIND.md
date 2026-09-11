# Gameplay-definition destructor cleanup

Packet `orch3_gameplay_definition_unwind_ac` closes a behavior gap in
`destroy_gameplay_effect_definition_00870d00`: a missing cached ID previously
threw before releasing the name or component array and before restoring the base
vtable. The C++ implementation now follows the recovered three-state cleanup and
rethrows the same error. A component callback that throws during normal array
destruction instead runs only base cleanup, preserving the native partial array.

The existing project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and installed executable SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` were checked.
Byte spans, hashes, annotations and fixture limits are in
`reports/gameplay_definition_unwind.json`. Names are descriptive hypotheses;
the correct `CG_scalar_deleting_dtor_00871440` name is retained.

## Native bodies and unwind map

| Address span, inclusive | Original ABI / role |
| --- | --- |
| `00870D00..00870DC1` (194 bytes) | ECX actual24h definition; RET; no meaningful result. |
| `0086FC30..0086FC46` (23 bytes) | ECX actual12h component-array header; RET. Resize0, reload/free current backing; no internal EH. |
| `00871440..0087145D` (30 bytes) | ECX definition, stack flags; EAX original owner, RET4. Raw free only after successful destructor and flags bit0. |
| `00C95FDE..00C95FE7` (10 bytes) | Compiler handler selects FuncInfo `00DC7F7C`, then jumps to `00BF6B43`. |

FuncInfo `DC7F7C` has magic `19930522`, three state entries, and points to the
unwind map at `DC7F64`. Each funclet obtains the complete owner from `[EBP-18]`.

| State | Next state | Funclet | Cleanup |
| --- | --- | --- | --- |
| 2 | 1 | `00C95FD3` | Add1C to owner; tail-call actual `0041DD20` name destruction. |
| 1 | 0 | `00C95FC8` | Add08 to owner; tail-call `0086FC30` component-array destruction. |
| 0 | -1 | `00C95FC0` | Tail-call `00BD30F0` base-vtable store. |

The normal body installs state2 at `870D26`, before the manager getter and map
find/erase. It installs state1 at `870D69`, before the pooled name release;
state0 at `870D8D`, before array resize/free; and state-1 at `870DA4`, before the
normal base call. Assembly is authoritative here: the decompiler confuses parts
of the exception-state slot with iterator locals.

## C++ behavior and ownership

`DefinitionMemberUnwind` mirrors these states. Getter/map failure destroys the
current name, then the current component array, then restores `CEB130`. It does
not suppress a missing-ID error, put the ID back in the weak map, free the raw
definition, or change its reference count. The scalar's raw-owner free remains
after the destructor call, so it is skipped on an exception.

`destroy_gameplay_effect_components_0086fc30` calls the existing `86EDD0` resize
with0, then reloads the header's current data pointer and frees it. It leaves
pointer and capacity stale. A callback may move remaining references to another
valid backing; the final free uses that current backing. The array destructor
has no cleanup or retry if resizing throws.

In particular, `86EDD0` decrements count before releasing the captured last slot.
If that component's zero callback throws, its slot has not yet been cleared.
During the normal definition array stage, only state0 remains armed. The C++
definition therefore restores the base and propagates the error, preserving the
decremented count, uncleared throwing slot, remaining references and backing
allocation. Re-running array cleanup there would invent behavior and could release
an already-zero component again.

Cleanup runs in a nonthrowing C++ guard: a second exception while already
unwinding terminates. That fatal path was not executed by the fixture. The existing
`NativeStringStorage::release` interface is also nonthrowing; failure while
recreating its pool and native hardware/SEH faults are outside the tested domain.
All referenced raw storage must satisfy the native valid-span preconditions.

## Validation and Ghidra extent limits

The strict MSVC Win32 build and both existing CTests passed. One ignored fixture,
`local/gameplay_definition_unwind_ac.cpp`, performed these focused comparisons:

- Complete original194-byte destructor and30-byte scalar, on the normal flags0
  path, against C++; balanced stack, full24h owner image with pointer normalization,
  actual weak-cache removal, pooled name release and descending component releases.
- Original map-selected name/array/base funclets, with a synthetic `[EBP-18]`
  frame, against C++ missing-ID cleanup. The C++ call uses scalar flags1, rethrows
  the original `out_of_range` message, and leaves the raw allocation to the caller.
- Complete original23-byte array destructor against C++ after a component callback
  replaces the backing. The captured old slot is cleared, remaining references
  come from the new backing, and the original free callback receives the new pointer.
- A C++ normal array-stage callback failure: name released once, ID already erased,
  count decremented, throwing slot uncleared, remaining reference alive, base
  restored, and no array retry. The fixture cleans the retained storage only after
  checking that state.

The original destructor's manager and STL calls use bridges to the same existing
canonical manager map. Its pooled string calls use the actual singleton and pool;
array resize uses the established C++ body. Original base and string helpers are
copied complete7/29-byte bodies. The compiler handler and exception dispatcher
are not invoked; the funclets are called directly in map order. This is evidence
for the cleanup operations, not native exception-dispatch or exception-object ABI
compatibility. Controlled component callbacks are fixture inputs, not reconstructed
game component terminal implementations.

The AB end-to-end definition/point/shake/teardown and name-allocation-failure fixture
was recompiled and passed against this change. No permanent tests were added.

Ghidra's false no-return override at `86FC3D` was cleared and its five tail bytes
were disassembled. The stored function extent still stops at the free call. The
definition's stored extent likewise omits its35-byte tail after `870D9A`.
Neither function was deleted or recreated. Full native spans and RET bytes were
verified independently against live memory and disk and included in the fixture;
the reports do not claim that these stored extents are repaired.

The new handler was defined, all affected evidence comments were saved, and
exports refreshed. Remaining work includes native exception-dispatch ABI,
remaining application component routes/terminal owners, concurrency and gameplay.
