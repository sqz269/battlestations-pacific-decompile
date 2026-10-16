# Native GUI raw group route43

This packet completes the Source caller route from `GuiNativeScene` plain page
roots to the existing raw-header `00B8F5E0` constructor. It adds no widget/model
tree, native render registry, name pool, material domain or native Text activation.
The semantic `create_group` body remains unchanged.

## Original evidence and interface boundary

The existing project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, was verified before the analysis batch. Full
`bsp.py show 00b8f5e0` pseudocode and assembly were reviewed. Original
`00B8F5E0..00B8F64A` receives ECX raw group and one stack name-header pointer,
returns the same group in EAX, and ends with `RET4` at `00B8F647`. It forwards
that exact header to `00B6F5A0` at `00B8F5E8`, reads actual one `00D7A24C` at
`00B8F5ED`, publishes group profile `00D634F8`, empties the actual attached-node
descriptor, sets flags/enabled/scalar, zeroes the first three pose words, then
reads actual positive bound `00CE4970`. There is no native pool allocation or
caller temporary-name destruction in this body. Existing source and ledger
already own the complete constructor; this packet does not relabel it or
recover a new Original function or enclosing native exception frame.

`src/gui_page_root.cpp:47` already composes this with the raw
`NativeStringRawPoolContext` and `NativeNodeRawConstants`. The new overload at
`src/native_group_owner.cpp:240` forwards the actual header unchanged through
the same node runtime, publishes the existing live group dispatch, and reuses
the existing prepared/live/dead owner phases. Its one/positive references must
identify the same cells as `NativeGroupEnvironment::constants`; negative bound
is explicit and has no inferred default. Raw node construction already owns
ordinary prefix cleanup on an exception (`src/native_node_construction.cpp:222`);
the wrapper ends its prepared derived companion once. It neither replays base
cleanup nor returns the physical slot. These are new C++ interfaces, not the
Original register ABI, FH3/SEH frame, or allocator/fault-time equivalence.

## Composition and retained failure state

Append `NativeGroupEnvironment::raw_node_constants` with
`&renderer.node_constants()` (`include/bsp/game_native_renderer_application.hpp:214`).
Use `renderer.camera_environment().nodes`, already bound to
`vfs.raw_strings()` (`include/bsp/game_hosts_vfs.hpp:124`), plus the existing
canonical group pool, VFS group types and actual group/node tables. The
environment's one/positive cells must be borrowed from that same renderer
constant provider. The optional pointer preserves existing semantic callers;
raw Scene admission rejects a missing/mismatched provider before pool entry.
Retain these contexts and cells through every group/name retirement.

The raw branch at `src/gui_native_scene.cpp:94` uses that same raw pool for its
actual temporary `NativeString` header and the group's copied native name.
`raw_pending_` owns the invocation record before pool allocation or any raw
getter. The same record, including actual slot/header/owner, transfers into the
existing `groups_` map after insertion. Pool/map/prepared-owner metadata failure,
temporary-name constructor failure, low-level constructor failure, throwing raw
temporary-name release, and canonical-reference metadata failure all preserve
that same Scene-owned frame. No entered exception rolls back, retries a getter,
releases a name twice, or returns a live/partial slot. This is deliberate Source
failure retention because the complete enclosing native caller cleanup frame
has not been proved.

Temporary cleanup marks ownership consumed before calling the throwing raw
`destroy_native_string_header_0041dd20`; it does not use a `noexcept` storage
adapter. No canonical `NativeGroupReference` has been constructed or published
at this point, so its Scene retirement callback cannot erase the record during
this release. Reference publication is the final potentially throwing work;
after it, only pointer capture, marker clear and return remain. Later actual
zero-reference retirement reuses the existing `finish_node` raw destruction
branch (`src/native_group_owner.cpp:124`) and canonical pool return/companion
retirement, preserving actual count, slot and companion identity.

`GuiNativeScene::native_operation_interrupted()`
(`include/bsp/gui_native_scene.hpp:35`) is metadata only and externally
serialized: true during entered raw construction and after any exception,
false after normal admission/retirement. It reads no raw cells and invokes no
callback. Reentry/replay is rejected. The destructor guard executes before
member destruction. **Future host composition must inspect this predicate and
preserve/terminate the process with `_Exit` before any manager or CRT drain on
an interrupted operation.** Destruction is a terminal guard, not recovery.

## Validation and remaining limits

`scripts/build.ps1` built MSVC Win32 Release `bsp_core` and `bsp_game`; both
enabled existing CTests passed (`reconstructed_math`, `tool_tests`). Diff
whitespace checking passed. No permanent tests, Ghidra mutations, ledger edits
or runtime activation were needed.

One ignored focused Source probe under
`local/cc12_native_gui_raw_group_route_43/` used one actual raw string-pool
owner/context, actual group/model pools and allocator list, and canonical
counter-derived type lineage. Constants/current tables came from the installed
Original executable's mapped PE data, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
It verified same widget/group/reference node identity, unchanged creator count,
actual copied name, one temporary-name return, page logical release with an
extra retained actual reference, delayed native-name retirement, final
companion erasure and return/reuse of the same physical group slot, then clean
allocator-list and actual raw string-pool retirement. Normal Scene interruption
state was false. The probe passed with `/W4 /WX` and `/MANIFEST:EMBED`.

The probe uses a controlled base widget implementation for its Screen-tagged
layout and an explicitly prepublished isolated string pool. It does not prove
the actual Screen/camera/store construction, lazy raw-manager registration,
exception/fault injection, native ABI parity, rendering, native Text drawing,
normal-host adoption or gameplay. Root owns subsequent host composition and
the separate primary widget-model route.
