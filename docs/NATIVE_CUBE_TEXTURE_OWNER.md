# Native cube texture owner

`native_cube_texture_owner.hpp/.cpp` reconstructs the three complete unnamed
cube owner entries over actual Win32 owner, COM, name, retained-memory,
renderer and pool storage. All three source symbols are linked from the same
frozen strict-build library in the original-caller fixture. Descriptive names
remain hypotheses; these explicit C++ interfaces are not binary replacements.

| Native entry | Complete range / bytes | Original ABI | Source entry |
| --- | --- | --- | --- |
| `B3D650` | `[B3D650,B3D6CA)` / 122 | ECX owner; stack cube COM, flags; EAX owner; RET8 | `construct_native_cube_texture_00b3d650` |
| `B3EAD0` | `[B3EAD0,B3EB6F)` / 159 | ECX owner; no stack arguments; RET; no semantic result | `destroy_native_cube_texture_00b3ead0` |
| `B3F410` | `[B3F410,B3F430)` / 32 | ECX owner; stack flags; EAX original address; RET4 | `delete_native_cube_texture_00b3f410` |

All 313 original bytes are freshly pinned against live Ghidra and the installed
PE. Current instruction listings cover the full returns with zero flow gaps.
The report is `reports/native_cube_texture_owner_audit.json`.

## Storage and exact construction

The actual cube owner occupies `30h` bytes of its `34h`-byte raw pool slot.
The slot's slab index at `+30h` is preserved. `NativeCubeTextureOwnerContext`
borrows the actual canonical pool, shared serial, renderer notification domain,
retained-memory domain and supported current renderer table; it creates none
of those owners or services.

Construction calls primary's complete
`construct_native_logical_texture_unnamed_base_00b34020` from
`native_cube_texture_base.hpp/.cpp`. That source preserves native profile/store
order, empty name, count one, borrowed COM and flags, and the old shared serial
plus current-DWORD increment at `0108D6E8`. The serial reference is the same
one used by named texture bases. No private serial, name copy or COM AddRef is
introduced. Primary's independent base fixture is recorded separately in
`docs/NATIVE_CUBE_TEXTURE_BASE_FIXTURE.md` and its report.

The cube constructor clears retained source `+2C`, installs `D61870`, then
arms its base cleanup. The original input COM pointer remains the receiver
for both queries, including when a COM callback changes `owner+10`.
It reads the input's current table and calls stdcall `GetLevelDesc(0)` at
slot `44h` with an uninitialized 32-byte `D3DSURFACE_DESC`. HRESULT is ignored.
It captures descriptor Width at `+18h` into `owner+24h`, reloads the input COM
table, and calls `GetLevelCount` at slot `34h`. Only after that call does it
read descriptor Format into `owner+18h`, then store the returned mip count at
`owner+14h`. Isolated DWORD operations preserve these read/write points.

The constructor directly leaves owner `+28h` and pool index `+30h` unchanged.
It does not check for null COM or recover when a failing HRESULT leaves
descriptor bytes unwritten. That failure does not establish deterministic
metadata. A throwing query runs only the established current-name/base
cleanup; the serial stays consumed, earlier metadata writes remain, and the
caller remains responsible for its COM reference and raw slot.

## Destruction and actual dependencies

The destructor installs `D61870`, captures retained source `+2C`, then arms
state 0. A nonnull source receives real `InterlockedDecrement(source+4)`;
zero invokes its current virtual-zero dispatch. Supported backing `D15AD8`
and stream `D642C0` tables contain `BD30E0`, whose current deleting slot routes
to the existing full `8D4470` or `BB8F90` source. These use the actual backing,
stream, reference counts, requested-byte accounting and shared CRT storage.
The cube field is cleared after a returning terminal, or after a nonzero
decrement. It is not cleared if that terminal throws.

The destructor next captures actual renderer publication `F8D394` and its
current table. Full `B33E40` forms `owner+8`, then the captured table's `+6C`
entry is read. The supported `D5F0A8` table selects the existing complete
`notify_native_renderer_texture_name_removal_00b32250`. Its actual temporary
strings, optional guard, alias-record removal and allocation domains are
reused. The captured receiver remains separate from later global renderer
reads inside that provider.

After notification, the destructor reads **current** `owner+10`. A nonnull
interface receives stdcall Release through its current table `+8`; only a
successful return clears `owner+10`. It then disarms owner cleanup and calls
existing `destroy_native_logical_texture_named_base_00b33f50`. Current name
destruction preserves the native header fields and performs the final
reference-base profile action.

The cube body has no generic-registry `B27D40` removal, support-singleton
diagnostic pair, surface cache, or tracking-counter operation. None is added.
Scalar deletion destroys first, then returns the original raw slot through
`return_native_cube_texture_slot_00b3d940(actual_0108DB70, owner)` only for
flags bit 0. Other bits do not select another action. The returned address
can already be reusable; a throwing destructor prevents pool return.

## Original FH3 cleanup

| Owner | Handler / FuncInfo | State 0 -> -1 action |
| --- | --- | --- |
| Constructor | `CBED68` / `DF7570` | `DF7568` selects `CBED60`; load owner `[EBP-30h]`, JMP `B34090` |
| Destructor | `CBEDC8` / `DF75FC` | `DF75F4` selects `CBEDC0`; load owner `[EBP-10h]`, JMP `B34090` |

Both native records have one unwind state, zero catch-map entries and
EHFlags 1. Full `B34090` forwards to `B33F50`; it is a distinct entry from
named-profile `B34010`. The new owner uses the primary's exact
`unwind_native_logical_texture_unnamed_base_00b34090` API. Armed scope cleanup
retains the original exception search rather than catching and rethrowing.
An SEH search filter terminates a second C++ exception during cleanup before
nested destruction; no retained-source/COM retry or raw-slot return is added.
Normal base destruction runs after the owner guard is disarmed.

## Verification and limits

Forty fresh guarded live/PE spans cover 1,727 bytes: all owner bodies, native
FH3 handlers/maps/funclets, exact provider bodies and returning-free tails,
native tables, and fixture global/IAT preimages. Every query checked project
`bsp`, program `/battlestationspacific.exe`, language and image base. Installed
PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

`./scripts/build.ps1` passed the strict MSVC Win32 build and both existing
CTests after all eight native math seeds matched. The ignored comparison
links the unchanged built library, SHA-256
`0b69bd7277ead690769fc1ebe53074a459ad0272715e89e26b15c813fb3994f8`.
The map confirms all three owner symbols and the actual base, name,
notification, retained-memory and raw-pool providers come from that library.
No owned source translation unit is recompiled to inject observations.

One ignored original-caller fixture matches **6,943 DWORDs / 131 events** in
53-DWORD observation frames across five focused states:

- Odd deletion with failing HRESULT but written descriptor, changed current
  input COM table, late Format change, current renderer/COM replacement,
  actual stream/backing destruction, alias removal, and raw slot reuse.
- Even deletion with a retained backing still referenced elsewhere and null
  COM; full destruction runs while the raw slot remains allocated.
- Constructor GetLevelCount throws after Width and serial publication;
  actual current name/base cleanup runs without later metadata stores.
- COM Release throws during deleting destruction; source cleanup is not
  repeated, the COM field stays uncleared, and the raw slot is not returned.
- Direct destructor entry with null retained source/COM and disabled optional
  guard, exercising the independently linked public destructor body.

The fixture reserves original addresses in its own hidden suspended child
before loader startup, reads the installed PE without modifying it, and
commits only pinned pages. Seven declared dependency/runtime entry jumps
compose the actual source providers and host FH3 runtime; one original IAT
word forwards real InterlockedDecrement. Two additional four-byte registration
operands at `B3D653` and `B3EAD3` point to host EXE jump-only trampolines that
enter unchanged original `CBED68` and `CBEDC8` handlers. The first run matched
both normal cases with unmodified registration operands, but the first
throwing case exited with unhandled C++ status `E06D7363`; the declared
trampolines resolve that fixture handler-admission boundary. All other 305
owner bytes remain original. Original handlers, FuncInfos and funclets remain
unchanged. The 440 runtime postimages verify declared patches and immutable
spans before and after each native/source state.

Real Win32 Enter/Leave imports in the fixture executable are observed through
two IAT wrappers that forward the resolved OS functions. Shared allocation
observation delegates to the unchanged actual singleton-lifetime source;
the existing `SizedStoragePool`/`PooledStringStorage` uses a compact fixture
geometry selecting real CRT-backed allocations. It is not a recreation of
the game's entire initialized string-pool image. COM endpoints are
deterministic ABI fixtures, not a D3D device. The current provider input-domain
restrictions remain, and secondary cleanup termination is retained from the
native map/existing pattern without an added artificial throwing-free test.

This is source/build/fixture evidence, not native binary compatibility, full
factory, loaded-file, GPU, visual or game validation. `B2A380` remains a
separate factory packet. Loaded-file cube `B3CED0 -> B34280` and whole
`B2C2D0`, other virtual methods, pool startup orchestration and reset behavior
are not completed by these three owner entries. The installed game and
permanent test targets are untouched.

The primary integrated permanent CMake registration and reran the unchanged
fixture against its frozen main library, SHA-256
`350b9bb962bdd5963eb9f0eea1e879d5984c824eaab0024313e7ff68dcb1db7f`.
It independently rechecked 99 worker artifacts, 22 current source files, all
40 fresh live/PE spans, 440 runtime postimages, both EH trampolines and
14 actual library providers. The full 6,943-DWORD / 131-event comparison and
both existing CTests passed. All three complete reconstruction records and
names are now registered; prior Ghidra names/comments were preserved, the
project saved and affected exports refreshed.
