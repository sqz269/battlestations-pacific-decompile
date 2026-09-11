# Native cube texture owner: remaining dependency closure

This read-only discovery identifies two implementation packets: the unnamed
cube base and its unwind forwarder (two entries, 76 bytes), followed by the
cube constructor/destructor/scalar delete (three entries, 313 bytes). It does
not implement or rename them. Evidence is in
`reports/native_cube_texture_owner_next.json`; descriptive names are hypotheses.

All five complete bodies, their native exception records, and the selected
dependency/table spans match the current installed PE in 36 guarded live
Ghidra comparisons: 1,595 bytes, including all 389 candidate bytes. Each live
query checked project `bsp`, program `/battlestationspacific.exe`, language
`x86:LE:32:default`, and base `00400000` using the repository client guard.
Installed binary SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
No Ghidra mutation, ledger change, C++ change, build, fixture run, or game
validation is part of this discovery.

## Complete entries and current gaps

Ranges below have an exclusive end. Instruction listings and disk decoding
agree on every ending, including the return after the final COM/base action.

| Entry | Range / bytes | Original ABI | Current state |
| --- | --- | --- | --- |
| `B34020` | `[B34020,B34067)` / 71 | ECX owner; stack COM, flags; EAX owner; RET8 | Named `BSP_LogicalTextureBase_ConstructUnnamed_D5F280`; no complete-function record |
| `B34090` | `[B34090,B34095)` / 5 | ECX owner; full JMP to `B33F50` | Unnamed thunk; no complete-function record |
| `B3D650` | `[B3D650,B3D6CA)` / 122 | ECX owner; stack cube COM, flags; EAX owner; RET8 | Unnamed; no complete-function record |
| `B3EAD0` | `[B3EAD0,B3EB6F)` / 159 | ECX owner; no stack arguments; RET; no semantic result | Unnamed; no complete-function record |
| `B3F410` | `[B3F410,B3F430)` / 32 | ECX owner; stack flags; EAX original address; RET4 | Unnamed; no complete-function record |

The saved Ghidra prototypes still display `undefined ... (void)`. Assembly,
not those prototypes or the decompiler's inferred `__fastcall`, establishes
the ABIs above. The current flow audit finds zero gaps in all five candidates.
`B34090` is distinct from the already reconstructed named-profile forwarding
entry `B34010`, despite both jumping to the same full `B33F50` body.

## Actual owner storage and constructor order

The cube pool allocates a `34h`-byte slot: a `30h`-byte owner payload followed
by the pool's slab index at `+30h`. These owner entries preserve that index.
They do not construct a private pool or replace the raw owner with a C++ object.

| Offset | Established behavior |
| --- | --- |
| `+00` | Native profile word; base ends at `D5F280`, cube installs `D61870` |
| `+04` | Intrusive reference count initialized to 1; destructor does not decrement the owner's count |
| `+08/+0C` | Eight-byte native name header, initialized to length 0 / null data by the unnamed base |
| `+10` | Input cube COM pointer; no constructor AddRef |
| `+14` | Base writes 0; successful cube query stores current GetLevelCount result |
| `+18` | Base preserves; cube later stores descriptor Format |
| `+1C` | Supplied flags, copied verbatim |
| `+20` | Old shared serial DWORD from actual `0108D6E8` |
| `+24` | Base preserves; cube stores descriptor Width, the cube edge length |
| `+28` | Untouched by all five candidates; meaning is not inferred here |
| `+2C` | Retained logical memory source; constructor writes null; destructor releases and then clears it |
| `+30` | Raw-pool slab index, outside owner payload and untouched by these candidates |

`B34020` writes profiles `CEB130`, then `D5F1F4`, then initializes count,
name fields, `+14`, COM and flags. It reads the actual serial, stores that old
value at `owner+20`, increments the **current** shared serial with DWORD wrap,
then writes `D5F280`. The increment must reload after the owner store: a
caller-provided serial that aliases owner storage cannot be replaced by a
private counter or a captured-value increment. This constructor has no native
EH frame, name allocation, COM call, reference retain, or old-name release.

`B3D650` calls the complete unnamed base before initializing `+2C` and writing
`D61870`. Native state 0 becomes armed immediately before its first COM call.
The COM receiver is the original input pointer retained in EDI, even if a
callback modifies `owner+10`. The exact sequence is:

1. Read the current input COM vtable; call slot `44h` as
   `GetLevelDesc(input, 0, &descriptor)` using stdcall. Ignore its HRESULT.
2. Read descriptor Width at `+18h`; store it at `owner+24h`.
3. Reload the input COM's current vtable; call slot `34h` as
   `GetLevelCount(input)` using stdcall.
4. Read descriptor Format at `+00` **after that call**; write `owner+18h`, then
   write the returned mip count at `owner+14h`. Return the original owner.

The descriptor is the SDK's full 32-byte `D3DSURFACE_DESC`. Its native stack
storage is not zeroed. The installed SDK declarations and hashes establish
both vtable slots and the Format/Width offsets. A failed HRESULT is not a
safe-return branch; unwritten descriptor bytes have no deterministic value
established here. The implementation must preserve read timing, including a
COM implementation that retains and later updates the supplied descriptor.

## Destruction, pool return and native unwind

`B3EAD0` writes profile `D61870`, captures the retained source from `+2C`, and
arms state 0. A nonnull source receives real `InterlockedDecrement(source+4)`.
Only a zero result loads its current table and invokes slot zero with ECX
source. The actual supported retained-memory tables lead through `BD30E0`
and the existing deleting owners; the parent field is cleared only after
that call returns. A nonzero count still clears the parent field.

Next the destructor captures actual renderer publication `F8D394` and that
receiver's current table. It calls complete `B33E40` to form `owner+8`, then
reads slot `6Ch` from the captured table and invokes it on the captured
renderer. Current renderer profile `D5F0A8` has `B32250` at that slot. Reuse
the complete actual renderer-name notification implementation and its actual
string/storage/synchronization domains; retain its separation between the
captured receiver and any global renderer reloaded by its optional guard.

Only after notification returns does the destructor load current `owner+10`.
If nonnull, it calls that COM interface's current slot `8` (Release) with the
explicit stdcall receiver, then clears `owner+10`. It sets native state to -1
before calling complete `B33F50`, which destroys the current name and performs
the reference-base profile action. There is no explicit generic registry
removal `B27D40`, resource-support diagnostic pair, surface-cache destruction,
or tracking-counter update in this cube destructor. Adding the 2D destructor's
extra operations would change its behavior.

`B3F410` runs the complete destructor first. Only a successful return followed
by flags bit 0 set calls `B3D940` with canonical pool `0108DB70` and the
original owner. Other flag bits do not select a different operation. Both
paths return the original address; after pool return that address can be
reused. An exception from destruction prevents pool return.

| Entry | Handler / FuncInfo | Exact cleanup map |
| --- | --- | --- |
| `B3D650` | `CBED68` / `DF7570` | `DF7568`: state 0 -> -1 via `CBED60`; load owner `[EBP-30h]`, JMP `B34090` |
| `B3EAD0` | `CBEDC8` / `DF75FC` | `DF75F4`: state 0 -> -1 via `CBEDC0`; load owner `[EBP-10h]`, JMP `B34090` |
| Existing `B33F50` | `CBE098` / `DF690C` | `DF6904`: state 0 -> -1 via `CBE090`, then `A81880 -> BD30F0` |

All three records have FH3 magic `19930522`, one unwind state, no try/catch
map, and EHFlags 1. Handlers load their FuncInfo and jump to `BF6B43`.
`B34090` reaches the shared destructor's current-name release and final base
profile action; it does not retry retained-source or COM release, free the raw
cube slot, restore a consumed serial, or roll back partial metadata writes.
Constructor raw-slot cleanup belongs to its caller, not this owner frame.
Use the existing cleanup-only, search-time termination approach where a
second C++ exception escapes a destructor during unwind; do not introduce a
catch/rethrow boundary or retry cleanup. Normal base destruction can propagate
its own exception without an already-disarmed owner cleanup running again.

The selected dependencies also include complete returning-free tails, rather
than trusting old no-return pseudocode. `8D4440` continues after `_free` at
`8D444D` through both accounting updates and `BD30F0`. `8D4470` continues
after `8D447D` through accounting/base cleanup and, after optional free at
`8D44A4`, through EAX restoration and RET4. `BB8F90` continues after free at
`BB8FA0` through EAX restoration and RET4. `B33F50` continues after actual
string-pool return through state disarm, base call and SEH restoration. All
selected current listings contain these tails with zero flow gaps.

## Disjoint packets and exact providers

| Proposed packet | Owned entries / files | Readiness and dependencies |
| --- | --- | --- |
| `native_cube_texture_base2` | `B34020`, `B34090`; primary owns new `include/bsp/native_cube_texture_base.hpp`, matching `src/` file, `docs/NATIVE_CUBE_TEXTURE_BASE.md` and its audit | Ready independently of cube pool lifetime/trim/static work. Borrow actual shared serial; call existing `destroy_native_logical_texture_named_base_00b33f50` through `B34090` using the actual `NativeStringStorage`. |
| `native_cube_texture_owner3` | `B3D650`, `B3EAD0`, `B3F410`; new `include/bsp/native_cube_texture_owner.hpp`, matching `src/` file, own doc/audit | Ready after base2 lands. Reuse the actual retained-memory, renderer-name notification, named-base and raw cube-slot providers below. |

The owner packet needs only the actual `NativeRetainedMemoryOwnerContext`,
`NativeRendererTextureNameNotificationContext`, shared serial reference,
actual canonical cube pool pointer, and current supported renderer/retained
table words. It needs no invented constructor/destructor callback, fake COM
reference count, replacement renderer, substitute string pool, or independent
cube allocation domain. Suggested context shape is an implementation planning
boundary, not a new exported ABI or a claim that arbitrary native table values
are already supported.

Primary has frozen the base APIs as
`void* construct_native_logical_texture_unnamed_base_00b34020(void*, void*, std::uint32_t,
std::uint32_t&) noexcept` (owner, borrowed COM, flags, actual shared serial) and
`void unwind_native_logical_texture_unnamed_base_00b34090(void*, NativeStringStorage&)`
in `native_cube_texture_base.hpp/.cpp`. Those actual source files now exist in
the primary workspace, with primary compilation/fixture validation pending.
Owner work must compose those exact providers rather than temporary callbacks.

Exact existing source providers are:

- `native_logical_texture_named_base.hpp/.cpp`: full `B33E40`, `B33F50`, and
  the established reference-base action. Use its real name storage contract;
  `B34010` alone does not close the separate `B34090` entry.
- `native_retained_memory_owners.hpp/.cpp`: actual backing/stream fields,
  counts, CRT allocation/free, and full `8D4470` / `BB8F90` deleting paths.
  Preserve current profile/table reads around the `BD30E0` flag-1 dispatch.
- `native_renderer_texture_name_notification.hpp/.cpp`: full `B32250`,
  actual receiver/publication, string pool, synchronization and removal chain.
- `native_cube_texture_pool_allocate.hpp/.cpp`: full
  `return_native_cube_texture_slot_00b3d940(actual_pool, actual_owner)` and
  caller-unwind adapter `return_native_cube_texture_00b3dce0`. These files are
  present in the discovery checkout. The initial lookup preceded their ledger
  integration; a final read on main `f8d4af5` confirms both full-function records
  are complete, named and exported. The source hashes match the discovery
  checkout. Do not duplicate these actual providers.
- `native_texture_2d_owner.cpp`: an existing implementation pattern for actual
  COM/current-table reads and cleanup search behavior, not the cube owner body
  and not proof of cube constructor/destructor completion.

Primary integration owns CMake, Ghidra annotations and shared ledger shards;
the two source packets should not race edits in the common named-base files.
Native pool lifetime, trim and static registration retain their separate
owners. The allocation provider requires a real initialized canonical pool
and actual trim binding where shared new-handler traversal is reachable.

## Caller boundaries and proportionate implementation proof

Factory `B2A380` is being traced independently and is a caller of `B3D650`.
Its publication, HRESULT, COM ownership and raw-slot unwind are not completed
by this discovery. Whole `B2C2D0` remains a semantic/fragment reconstruction.
Its loaded cube route uses a different constructor, `B3CED0 -> B34280`, before
retained-slot assignment at cube `+2C`; it does not use unnamed `B3D650`.
Those two named-route entries remain follow-on candidates, not complete owner
proof and not dependencies needed to implement the requested five entries.
The base profile's other virtual methods and all cube reset methods also
remain outside these packets.

Implementation should first use the existing Win32 build and native-caller
fixture infrastructure. A compact full-original-body comparison should cover
constructor store/query timing and actual shared serial, current COM table
changes between queries, normal retained/name/COM release and raw pool return,
and one concrete owner-unwind interruption. Preserve native FH3 records and
execute real provider allocations, counts, strings and pool storage. Pin
allowed boundary patches and runtime postimages. Do not broaden this discovery
into tests, install the resulting source into the game, or claim native ABI,
GPU, gameplay, or visual equivalence from byte equality alone.
