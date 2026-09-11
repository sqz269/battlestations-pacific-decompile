# Native renderer index binding

This packet reconstructs the complete 236-byte `00B24B00..00B24BEB` binding
body and supplies an original-profile-token adapter for the complete 10-byte
`00B48DC0..00B48DC9` logical getter. Descriptive names are hypotheses. The
existing byte-identical naked getter remains available for relocated callable
tables; numeric game addresses are never called by this adapter.

The API contract was fixed before implementation: raw renderer and logical
storage, actual synchronization and full logical/physical lifetime services,
an immutable eleven-DWORD `D61DE0` profile, and both immutable eight-DWORD
`D61E10`/`D61E58` profiles. Lifetime and getter dispatch share these physical
views. Logical `+28` selects `B48DC0`; current logical `+08`, current physical
profile, and that profile's `+1C` select the full actual `B4B840` provider.
These are original code-address tokens. Unsupported profiles have no invented
rejection or callback path and lie outside this recovered domain.

Native ABI: `B24B00` receives the raw renderer in ECX and two stack DWORDs
(logical owner and base vertex), then returns with `RET 8`. `B48DC0` receives
logical storage in ECX, replaces ECX with its current physical owner, and
tail-jumps through the current physical table's `+1C`; that target returns EAX.
The C++ interfaces explicitly borrow context and are not binary replacements.

The optional guard enters before the first identity read. Native state zero
is armed after that comparison and before the unconditional renderer `+17BC`
base-vertex write. The captured comparison governs the outer branch. On change,
the old `+17B8` owner is loaded again and compared again; replacement publishes
new before real Win32 increment/decrement. A zero old count reads the current
logical slot zero, then full `BD30E0` rereads its current deleting slot and
selects full `B4C1F0` with flag one. Existing providers preserve nonempty
logical registry, physical ownership, COM release and pool-return behavior.

Original nonnull input then selects the current logical getter and current
physical getter. Null input produces null directly. Both paths load the current
renderer device after the getter, then its current table, and call `+1A0`
(`IDirect3DDevice9::SetIndices`). No HRESULT condition or rollback is added.
The counter at `+1BB8` wraps only after COM returns. An outer identity match
still changes base vertex and enters/leaves the optional guard, but skips
reference changes, getters, COM and counter. Inner identity skips references
while still reaching the getter, COM and counter.

The FH3 state is disarmed before normal leave. Exceptional cleanup reaches
actual `B21110`; a secondary C++ cleanup exception terminates during search.
Skipped entry leaves original guard storage uninitialized; a disabled-entry to
enabled-exit mode transition is outside the valid native execution domain.
Normal native leave loads a DWORD whose high padding bytes are unused by
`B33B00`; the C++ interface passes the defined low byte.

The isolated replay executes all 236 original binding bytes, the 10-byte logical
getter, the 14-byte zero-count invoker and the 4-byte physical getter. Relocations,
the host FH3 personality and bridges to full compiled lifetime/synchronization
providers are explicit and every modified image byte is verified. Twenty-six
fresh saved-Ghidra/installed-PE spans include complete logical and physical
provider bodies. No saved Ghidra state or game installation was modified.

Seventeen ordinary scenarios compare 17,184 DWORDs (68,736 bytes) exactly,
including 59 ordered writes and 30 getter/COM/catch records. They cover outer
and inner identity, null unbinding, nonempty logical/physical destruction,
entry/leave and COM exceptions, ignored HRESULT failure, mode changes, and
counter wrap. A read observer changes the renderer device after the actual
physical getter; another changes its physical profile after logical `+08` is
read. A real old-buffer Release observer changes logical `+08` before its getter.
The standalone original getter and adapter also return the same real buffer.
Both isolated secondary-cleanup exceptions terminate with identical state.

The nonempty lifetime cases exercise actual registry removal, COM Release,
physical-array free and both pool returns. Physical base destruction sets array
count to zero and leaves the freed pointer and capacity words intact; the
comparison preserves those post-free words. Throwing COM Release leaves COM
uncleared and skips both pool returns while actual base cleanup still runs.

The fixture uses real D3D9 HAL devices and index buffers, checks descriptors and
uploaded index data, and verifies SetIndices through GetIndices. Observers forward
actual D3D9/Win32 calls. Recorded getter/call/write instruction addresses and
48 loaded module entry/disk matches were checked independently. The strict
MSVC Win32 library, both existing CTests and all eight native seed checks pass.
No permanent tests were added. The fixture library and artifact hashes are
recorded in `reports/native_renderer_index_binding_audit.json`.

These results establish the bounded C++ contract and private native comparison;
they do not establish original game ABI compatibility, native CRT TLS identity,
or gameplay behavior.

## Primary integration

The primary registered the source in CMake and passed the strict Win32 build
and both existing CTests. It independently verified 107 worker artifact/source
pins, 18 current source/provider files and 26 fresh live-Ghidra/PE spans
(1,165 bytes). The unchanged fixture linked frozen actual main library
`5b3db94b2c1be9df32ff956fc81372e0f58a5c260678269a58714d1b218d9603`.
All 17 scenarios matched 17,184 DWORDs, 30 getter/COM/catch records and
59 writes per implementation. Isolated double-fault processes also matched
exit 91 and terminal state. Real HAL buffers and complete owner/registry/pool
lifetimes participated. All 16 named providers, 35 mapped closure fragments,
48 actual module-entry/disk checks, four native postimages and observed
instruction PCs passed. Both saved names received appended evidence; complete
records and forced exports retain the distinct existing naked getter contract.
No permanent tests, original-caller ABI, draw or full-reset validation is claimed.
