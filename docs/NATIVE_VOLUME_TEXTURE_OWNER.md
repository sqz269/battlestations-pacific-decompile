# Native volume texture owner

`native_volume_texture_owner.hpp/.cpp` reconstructs complete volume destruction,
scalar deletion, and its distinct base cleanup tail over actual Win32 storage.
The original code and unchanged strict-build source library match in one ignored
fixture using real D3D9 volume textures. Names remain descriptive hypotheses;
the explicit source interfaces are not native binary replacements.

| Native entry | Complete range / bytes | Original ABI | Source entry |
| --- | --- | --- | --- |
| `B3EB70` | `[B3EB70,B3EC24)` / 180 | ECX owner; RET; no semantic result | `destroy_native_volume_texture_00b3eb70` |
| `B3F430` | `[B3F430,B3F450)` / 32 | ECX owner; stack flags; EAX original address; RET4 | `delete_native_volume_texture_00b3f430` |
| `B340F0` | `[B340F0,B340F5)` / 5 | ECX owner; JMP `B33F50`; no extra state | `unwind_native_volume_texture_base_00b340f0` |

All 217 bytes are pinned against fresh guarded Ghidra reads and the installed
PE. `reports/native_volume_texture_owner_audit.json` contains the full evidence,
source/map provenance, original ABI, uncertainty and artifact hashes.

## Actual owner and pool

The owner occupies `34h` bytes in a `38h`-byte slot. Its trailing pool slab
index at `+34h` survives destruction. The context borrows the actual renderer
notification/string domain, retained-memory domain, current renderer profile,
and a `D3D9SurfacePool&` companion over the actual volume pool at `0108DBA8`.
The notification string storage and sized pool share their actual allocation
domain. Reached storage and actual COM tables must remain valid at native
access points; the existing optional-guard domain also applies.

The shared pool type is supported by the native routes themselves:

| Volume route | Native action |
| --- | --- |
| `CD7BA0` startup | ECX=`0108DBA8`; CALL existing `B3EC60`; register `CE0CC0` with atexit |
| `B3F2D0` allocation | ECX=`0108DBA8`; JMP existing `B3ED40` |
| `CE0CC0` destruction | ECX=`0108DBA8`; JMP existing `B3E2B0` |
| `B3DCF0` / owned `B3F430` return | ECX=`0108DBA8`; CALL existing `B3D860` |

Thus this is the same native initializer, profile `D6193C`, allocator-list
trim entry `B3E390`, and layout: 32 slots of `38h` bytes, `744h`-byte slabs,
free-index words at slab `+700h`, and free count at `+740h`. The existing C++
companion binds that actual allocator element and contributes no second native
list, slab table or slot state. Its return method reads its borrowed storage;
it does not route through the static surface adapter at `0108DB00`.

No volume pool constructor or startup implementation was added. Callers must
provide the initialized actual volume-pool companion. The fixture uses the
existing actual initializer, allocator, destructor and allocation domain.

## Destruction order and dependencies

`B3EB70` installs `D618B0`, captures renderer publication `F8D394` and its
current table, then arms state 0. `B33E40` forms the original `owner+8` name
address. The captured table's `+6C` word selects the existing complete
`notify_native_renderer_texture_name_removal_00b32250` for `D5F0A8`.
The captured receiver remains separate from subsequent publication reads
inside that provider. Its real temporary name, guard and alias removal run.

Only after notification returns does the destructor read **current** retained
source `owner+30h`. A nonnull source receives real
`InterlockedDecrement(source+4)`. Zero invokes its current virtual-zero word;
supported backing `D15AD8` and stream `D642C0` profiles select `BD30E0`, which
reloads the current deleting slot and passes flags 1 to the existing full
`8D4470` or `BB8F90` source. These use actual backing/stream storage, intrusive
counts, global object/byte accounting and shared CRT allocation. `owner+30h`
clears after a returning terminal or a nonzero decrement.

The destructor then captures COM pointer `owner+10h`. If nonnull, it invokes
stdcall AddRef through the captured interface's current table `+4`, reloads
that same interface's table, and invokes Release at `+8`. These calls preserve
the captured receiver even if instrumentation changes the owner field.

Next it independently reloads **current** `owner+10h`. A nonnull interface
receives Release through its current table. Only a returning final Release
clears the owner field. It then disarms cleanup and invokes the complete
current-name/base `destroy_native_logical_texture_named_base_00b33f50`.
Opaque owner fields, including `+28h` and `+2Ch`, and the pool index survive.

`B3F430` destroys first and returns the original slot through the actual
volume pool's `B3D860` only when flags bit 0 is set. Other bits add no action.
A throwing destructor prevents pool return. The returned address may already
be reusable. No generic-registry removal, tracking counter or surface cache
operation is present in this body.

## Exact cleanup tail

Handler `CBEDE8` loads FuncInfo `DF7628`: one unwind state, zero catch entries,
EHFlags 1. Map `DF7620` transitions state 0 to -1 through `CBEDE0`, which
loads the saved owner from `[EBP-10h]` and jumps to **`B340F0`**. Its complete
five bytes are `E9 5B FE FF FF`, a tail to `B33F50`. This is a distinct native
entry from the cube cleanup tail.

The named C++ adapter forwards to that existing full base provider. Armed
scope cleanup uses the adapter without catching and rethrowing the original
exception. An SEH search filter terminates a second C++ exception during
cleanup, following the established cleanup-only policy. Retained-source or
COM release stages are not retried, and their field clears occur only after
return. Normal base destruction runs after the guard is disarmed.

## Verification and limits

The strict `scripts/build.ps1` MSVC Win32 Release build passes `/W4`, `/WX`,
`/fp:strict` and both existing CTests after eight native math seeds match.
An ignored CMake include registers only the owned source; tracked CMake and
shared metadata are untouched. The fixture links the frozen `bsp_core` library
with SHA-256 `0b6711e15330c9a369ba2d4f73a609ef46ab35d5a516b39b99e3c76a6be25f75`.
No owned source unit is recompiled to insert observations.

One ignored comparison matches **12,214 DWORDs / 197 frames**, covering odd
deletion with captured/current pointer changes and slot reuse, direct null
destruction, even deletion with a real backing count 2→1, and throws after
the actual AddRef, captured Release, and final Release. Each throw preserves
earlier lifetime effects and runs the actual current-name/base cleanup.
Trace SHA-256 is
`b50ca9c6815d82c528de5f1df5c31dc90626ca009e8ae7b6ecc07e456b77efb1`.

The fixture creates one hidden HAL device and 48 actual
`IDirect3DVolumeTexture9` objects. Observer tables copy the real 22-slot tables;
24 observed reference calls forward to real D3D9 AddRef/Release methods and
record their results. Eight extra fixture references per object keep all
reached objects valid during explicit field/table changes. All 48 objects,
the device and the D3D factory release to zero. This tests reference lifetime,
not rendering. No game window, game installation or saved analysis is changed.

The private suspended child reserves the original address range before loader
startup and copies pinned PE spans. Four declared dependency-entry jumps compose
the existing actual name/base, notification and retained-memory providers on
both paths; one runtime jump enters host FH3. Three IAT words forward real
Win32 atomics/critical sections. Original `BD30E0` and all 105 bytes of pool
return `B3D860` execute unchanged. Within the 217 owned bytes, only the four-byte
handler registration operand at `B3EB73` is relocated to a verified host EXE
jump-only trampoline entering unchanged original `CBEDE8`.

Thirty-four fresh live/PE spans cover 1,854 bytes. Within 442 runtime span
postimages, all immutable bytes and initial mutable preimages match the
declared bindings. The whole fixture `.text` matches its
fixed-base PE before and after the run. The map verifies all three owner entries
and actual providers from the frozen library. Actual COM call return addresses,
table entries and loaded x86 D3D9 module identity/relocated code bytes are checked.
Allocation observations forward through an unchanged, separately symbol-aliased
`singleton_lifetime.cpp`; they retain actual CRT effects.

The report distinguishes complete reconstruction, strict build, focused native
comparison and real COM reference lifetime. It does not establish a native ABI
replacement, arbitrary-profile dispatch, volume construction/factory behavior,
full renderer reset, game execution or visual parity. There are no new permanent
tests. Static instruction/store evidence and observation frames are not claimed
as a per-store hardware trace.

## Primary integration

The primary registered the source in CMake and passed the strict Win32 build,
both existing tests and eight fresh native seeds. It independently verified
99 worker artifact pins, 16 current source files and 34 fresh live-Ghidra/PE
spans (1,854 bytes). Frozen actual main library
`12b239bcbf44379d4523d195917f31e31c2bc49073b0551d28b0c1a589b26e19`
replayed all six states: 12,214 literal DWORDs and 197 frames matched. All 24
real COM calls have source/original caller and actual x86-module evidence; all
48 real volume objects were released to zero. Eleven frozen provider objects
match exact main archive members. All 442 original span postimages and the
whole fixture executable text before/after were checked. Declared external
provider bridges, FH3 registration relocation and singleton allocation/free
observation aliases retain the boundaries documented above. Three names with
appended evidence are saved in Ghidra, exports forcibly refreshed and complete
reconstruction records registered. No permanent test was added. This does not
establish original-caller ABI, complete renderer reset or gameplay.
