# Native occlusion query device reset

This reconstructs the complete release at `00B5FE20` (27 bytes) and restore at
`00B5FE60` (33 bytes) over actual borrowed owner storage. The existing typed
`d3d9_query.cpp` projection remains separate. Descriptive names are hypotheses.

Both native entries receive the owner in ECX and end in plain RET. Neither has
stack arguments or its own exception handler. The new C++ interfaces in
`native_occlusion_query_device_reset.hpp` use explicit arguments and are not
drop-in binary replacements.

Release captures the current query pointer at owner `+10`. Null returns without
writing. Otherwise it calls the captured query's current vtable `+08` Release
once, then clears owner `+10` only after the call returns. It does not reload
the owner field before that call or clear. A callback can replace the owner
field; a normal return still clears it, while an exception leaves the replacement.
There is no balanced AddRef/Release pair, owner destruction or unregister step.

Restore reads the current renderer publication at native `00F8D394`, calls the
complete actual device getter `00B1FEF0`, then invokes device vtable `+1D8`
CreateQuery with query type 9 and **the address of owner `+10`**. The source API's
second argument is the address of that actual four-byte publication cell. It
does not accept a cached device. There is no check for a previous query, previous
query release, output initialization, HRESULT branch or exception rollback.

The source uses single raw x86 DWORD accesses for the observed pointer cells.
It composes `get_native_renderer_device_00b1fef0` from the existing actual
shader-reset provider. The profile cells at `00D62AE8` and `00D62AEC` agree with
these release/restore entries. No complete owner size or additional field layout
is inferred from these two functions.

## Verification

Five current live-Ghidra/installed-PE spans total 79 bytes: the complete two
entries, the seven-byte device getter, the four-byte renderer publication and
eight profile bytes. Every live query verifies the original BSP project and
`/battlestationspacific.exe`. All eight existing differential seeds agree with
the installed executable. The strict MSVC Win32 build and both existing CTests
pass; no permanent test target was added.

The ignored focused fixture links the frozen, completed main `bsp_core.lib`.
The original entries execute with their relative getter call intact and only
one declared four-byte absolute renderer-global operand relocation. There are
no entry or service bridges. All original code/data postimages are checked,
including the explicit mutable renderer publication. The main release, restore
and device getter are bound to their real library objects; complete linked
bodies match the frozen COFF objects with only the restore's genuine REL32
getter binding. Six runtime captures cover all three complete main bodies.

Two real HAL D3D9 devices per process create real occlusion queries. Temporary
observation tables restore and call the actual Windows COM methods before
applying the explicitly tested field/publication changes or throwing. Six
states cover null release, captured-query release with current-field replacement,
an exception after real release, direct creation over a nonnull old query,
current renderer selection on the next restore, and an exception after real
creation. Old queries are not released by production restore; the fixture
retains and releases its own references separately.

The original and main implementations agree on **369 DWORDs / 1,476 bytes**,
27 state snapshots and six exception records. Only actual query and renderer
pointer identities are normalized; every other sampled owner DWORD is exact.
Each process makes three real CreateQuery and two real Release calls. All ten
production call return addresses resolve to the expected original or compiled
instructions. Eighteen loaded-module entry captures match actual x86 PE files
with normal loader relocations only. The factory's executable import thunk and
resolved IAT target are verified separately; only the factory can resolve via
the Windows apphelp compatibility shim. Query creation/release remain actual
`d3d9.dll` methods.

`reports/native_occlusion_query_device_reset_audit.json` pins the exact source,
library, objects, executable, SDK, native spans, module captures, traces and
verification scripts. This proves the bounded reset entry behavior. It does not
prove a complete renderer reset, a failing native CreateQuery HRESULT, query
Issue/GetData behavior, rendered occlusion, original binary ABI or gameplay.
