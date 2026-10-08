# Title initialization FileBlock Source composition

The title initializer's existing `open_named_block` / `close_named_block` pair
now enters and leaves a real FileBlock over the application's retained VFS.
Previously both methods only logged unimplemented calls. Only this title pair
is connected; the other menu/mission/loading-queue consumers remain separate.

Five Source/header files change. The existing runtime now retains unmount,
PAK-block, concrete observer-dispatch and FileBlock-scope contexts. The host
retains an immovable Source invocation frame containing the actual **1Ch
FileBlock owner** and both acquired frames. The menu keeps its already-supplied
`GameVfsHost&`. No CMake, CLI, test, Native provider body, ledger or Ghidra edits
were needed. **Additional Original-function credit is zero.**

## Actual production domain

The existing archive tail registers the real MPAK factory and calls
`00736C30`; its ordinary `00BB4FB0` construction creates a 1Ch registry with
reference count 1, ordinal 100 and an empty raw vector. The tail stores that
actual registry at VFS `+88`, stores the requested cached-load flag at `+78`,
initializes and publishes its retained `SharedLock`, then marks completion.
An initially null `010904DC` cache is valid: `00BB83A0` explicitly allocates and
constructs a real 44h MPAK provider in this case. `00BB82F0` handles later cache
publication through the existing provider/runtime/storage domain.

`borrow_fileblock_scopes()` admits only the completed, nonretired domain. It
checks the current publication against the retained A0 allocation, current
`+88` against the real nonnull registry publication, the actual registry's
`D64190` profile, the actual published lock against retained storage, and
current `+78` against the produced flag. It also obtains the actual verified
mapped table and checks `D64190+8 = BB5770` and `D64190+C = BB5910`.
`ready()` alone is not used for admission. The existing raw-services view and
its pre-core borrowing contract remain unchanged.

The new contexts all borrow the same current publication, string pool,
canonicalizer, name resolver, mount/provider dispatch and invalid-parameter
services. Verified Original bytes supply `CE7898` (`/`), `CE3A70` (`.`) and
`CE3A0C` (zero). The existing application supplies the retained empty
`0109CEF0` Source byte. Observer dispatch reaches the existing complete
`BB5770` / `BB5910` implementations, including actual provider creation,
registry-vector changes and `BE0750` unmount/release. No registry, provider,
profile, gate stack or callback is replaced by a host projection.

The original game uses an allocating lock wrapper; this composition retains
the existing Source `SharedLock` storage and its real Win32
`InitializeCriticalSection` call. This is not new reconstruction credit for
the Original allocating wrapper.

## Ownership and cleanup

The outer Source frame is **4Ch** in the current Win32 build: actual FileBlock
bytes at `+0..+1B`, caller pooled header at `+1C`, context at `+24`, construction
and destruction acquired frames at `+34/+38`, captured caller-name return
state at `+3C/+40/+44`, and `opened/closed` at `+48/+49`. It is appended to the
existing host; the supplied VFS reference is appended to the menu's private
state, preserving older private field offsets.

Normal entry publishes the complete frame before even constructing its pooled
caller label. `BE0A30`, with the title's native gate value 1, copies and
identifies the owner's name and enters the actual VFS scope. The caller label
is then returned through the same raw pool getter/return pair before title
work proceeds. The NativeString header has no implicit pool destructor.

Normal exit calls ordinary `BDCB30`: reload current publication, leave the
real observer/gate scope, return the owned name, and stamp the base. Only after
that returns does Source store `closed = true`, clear the host pointer,
destroy the completed acquired frames and free the outer Source allocation.
There is no scalar-delete call or automatic Native destructor replay.

`invoke_native` marks an escaping operation and rethrows. **Any retained title
frame also requires process retention**, including when later title work
throws outside the open/close methods. The existing startup destructor,
application-shutdown and singleton-drain entry points check this predicate
before deleting menu state or draining the shared graph. They reach the
existing `_Exit` path, retaining the host, application, runtime, raw services,
publication cells and mapped data together. The frame's C++ destructor also
rejects incomplete teardown instead of guessing Native cleanup.

Native VFS depth, name, `+79` gate and the actual `+7C/+80/+84` gate list remain
owned by the existing bodies. `BE0A30` and `BDCB30` retain their current-
publication reloads. Failed acquired invocations are never replayed. Uses
still exclude concurrent publication mutation/startup/drain, and all normal
blocks must close before shared singleton drain.

## Current build and artifact evidence

The normal MSVC Win32 Release build passed with `reconstructed_math`,
`native_math_differential` and `tool_tests`. The first build found a missing
complete-type include in the new menu binding; it stopped before tests.
After the include correction, inputs were physically frozen again and the
second normal build passed. No additional test, probe, diagnostic or startup
execution was performed.

Current evidence covers **76 complete objects**, **70 unique complete core
archive members**, **8,350 complete COFF extents**, and **490 complete linked
extents / 3,207 ordered relocations** against the actual normal MAP and PE.
The actual selected compiler records identify **900 distinct inputs** from
the prefrozen files. This is not an exhaustive OS/toolchain closure claim.

The complete 473-byte entry body publishes the Source frame at offset 272,
calls raw label construction at relocation operand 340, calls `BE0A30` at
357, and returns the caller label before its opened store. The complete
204-byte admission body checks the actual domain and mapped callback entries.

The actual close invocation wrapper is 202 bytes. Its `BDCB30` relocation
operand is 116; the closed store is at 120, host-pointer clear at 132, and
outer-frame free relocation at 175. Three error branches reach two locations
in its separately retained and replayed **76-byte catch/cold extent**. Both
whole extents, all relocations and those actual branch destinations are
verified; the 202-byte extent alone is not the exceptional flow closure.

The actual Source `MenuTitleInitHost` vtable slots `+4/+8` point to the
current 18-byte open and 14-byte close methods. The complete 207-byte title
initializer dispatches through these slots before and after its work. All
three existing pre-drain callers and the expanded retention predicate are
included in the current complete-body proof.

Registry construction is visible as the full initializer/store sequence in
the actual linked 310-byte getter. Its separate 69-byte constructor COFF body
has no standalone normal MAP entry. Likewise, helper/template extents without
standalone MAP entries remain COFF evidence; no blanket inlining claim is
made. The linker selects `game_hosts.obj` for the VFS host's scalar deleting
destructor; that complete winner is replayed separately from the discarded
`game_hosts_vfs.obj` variant.

The 75-object comparison retains 8,118 unchanged preexisting extents. It uses
the previously existing normal objects, not a newly rebuilt accepted-main
baseline. The supplemental `platform_window` object adds 129 current extents
to coverage; these are not new implementations.

## Evidence and limits

The [machine report](../reports/cc12_game_vfs_title_fileblock_source.json)
contains the concrete ownership graph, whole-body identities, ordered gates,
producer states and artifact hashes. Local evidence is under
`local/cc12_game_vfs_title_fileblock_source_20261008a/`:

- `before_build02/manifest.json`: final actual prefrozen inputs.
- `run02/build_receipt.json`: normal build and all three existing checks.
- `after_build/whole_COFF.json`: the complete 75-object capture.
- `proof02/static_proof.json`: complete objects, unique members and current link proof.
- `supplemental03/supplemental_proof.json`: current strings, publication and real lock provider.
- `domain02/domain_gate.json`: complete production/lifetime order and title dispatch.
- `cold_edges01/cold_edges.json`: close wrapper's actual exceptional branch destinations.
- `original01/evidence.json`: supported Original PE, tables and nine attributed bodies.
- `final01/seal.json`: final owned files and artifact seal.

Accepted main at work start was `5120cc564a24b5f95030bfcbce4cfcc9ef6c7030`.
Earlier connected-read and callable-publication families remain sealed and
historical; none of their execution evidence is reassigned to this change.

Attribution covers `4C9A70`, `BE0A30`, `BDCB30`, `BE0980`, `BDC9B0`,
`BB5770`, `BB5910`, `BE0750` and `BDF950`. This is a Source composition of
existing ordinary implementations, using new explicit-service C++ APIs.
It does not establish Original ABI, FH3/SEH equivalence, complete bootstrap,
asset-load success or gameplay parity. Outer frame retention does not repair
the existing `BDD850` / `BE1740`, string or resolver internal-temporary and
cleanup limits. Primary review/integration and live title/game validation
remain separate.
