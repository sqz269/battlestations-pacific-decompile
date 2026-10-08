# Full title FileBlock execution readiness

**Decision: the readiness audit is complete; full title execution is not ready.**
The accepted FileBlock open/close ownership graph is connected, but the actual
`MenuTitleInitHost` still reaches mandatory unfinished title consumers. No Source,
build, diagnostic, startup, game, ledger, packet-state or Ghidra mutation is part
of this packet. Baseline: main `696f8ffff8027f0ad332c564970e9793a32698c4`.

The primary explicitly required the real title caller. Direct calls to
`GameVfsHost::open_title_fileblock` / `close_title_fileblock`, a custom
`TitleInitHost`, an intercepted return after open, or a synthetic provider/header
cannot establish that qualification. A future physical FileBlock owner witness
would have separate ordinary ownership scope.

The full receipt is
[`cc12_game_vfs_title_fileblock_execution_readiness.json`](../reports/cc12_game_vfs_title_fileblock_execution_readiness.json).
Local evidence is retained under
`local/cc12_game_vfs_title_fileblock_execution_readiness_20261008a/`.

## Actual Source graph and producer boundary

`GameStartupHost` constructs the real menu and calls its
`run_title_init_004c9a70` at `src/game_hosts.cpp:2241`. That entry constructs
`MenuTitleInitHost` and calls `bsp::run_title_init` (`src/title_init.cpp:178`).
Its first virtual calls the accepted VFS host open; its last calls close.
The production entry also requires frontend/device/fonts/script/locale/input
owners. It is not the initialization sequence used by the older physical-read CLI.

| Dependency | Current evidence and qualification boundary |
| --- | --- |
| Host/App/A0/current publication | `GameVfsHost` owns the actual `GameNativeVfsApplication`; `initialize_core` allocates A0 and registers the real core. Raw borrows reference the actual private cells. |
| String/pool/lifetime domain | Same application string service, process string-pool publication, returns-disabled cell and shared singleton manager. No stand-in `NativeString` allocator. |
| Phase 2 and archive tail | Production phase 2 performs its real core, loose mounts, two package scans and search defaults. `factory_tail` registers MPAK, gets the actual registry, stores VFS+88/+78 and initializes the retained `SharedLock`. |
| Genuine empty registry | `736C30 -> BB4FB0` produces a 1Ch registry, refcount 1, D64190 profile, ordinal 100 and pointer/count/capacity zero. This is valid produced state. |
| Observer/callback domain | The admission guard checks current A0, actual registry/lock identity, cached-load flag and mapped D64190 slots +8=BB5770/+C=BB5910. These original addresses identify the table entries; concrete Source functions perform dispatch. |
| Provider vector | BB5770 constructs the real package name and resolves it. Only a true result selects a device, mounts the actual provider under `.` and appends it. A false result legitimately leaves the produced vector untouched. |
| Cache and lock | A null initial cache is valid; BB83A0 constructs a real 44h provider if reached without a cache. A resolver-false witness does not exercise that allocation, cache replacement or lock transition. |
| Gate/depth/name | BE0980 links a real gate node, applies gate 1, calls the observer when +78 is true, increments VFS+14 and writes +C/+10. BDC9B0 calls leave, clears the name, decrements depth, restores the saved gate and erases the node. |
| Retained frame | Host publishes the immovable frame before allocating its pooled name. Complete BDCB30, including observer exit, owned-name return and base destruction, precedes `closed=true` and frame reset/free. |

Cached load must be **true** for the proposed callback qualification. Choosing
false to avoid the registry calls would not establish the requested path. The
registry, pool, lock, manager and frame must all come from their actual producers.

## Real installed input

The unchanged installation root is
`I:/SteamLibrary/steamapps/common/Battlestations Pacific`. A complete inventory
contains **67,814 files**, no `.mpak`, no `.mpkg` and no title-package name match.
The before/after path, size and modification-time inventories agree. This is a
metadata inventory, not a content hash of every installed asset. Six relevant
files were separately copied whole, hashed and checked through read-only Windows
handles for final path, volume serial and file ID.

The actual literal at `00CE7680` is `GGame::OnInitTitle`: 18 characters plus NUL.
BDF950 replaces the two colons, producing `GGame__OnInitTitle`; BB5670 appends
`.mpak`, yielding **`GGame__OnInitTitle.mpak`**. No long-name shortening applies.
Installed absence makes the no-package branch a candidate, not runtime proof:
a later witness must observe the real resolver return false and the genuine
registry vector remain unchanged. BB5910 still resolves and calls BE0750 on exit
even when the vector count is zero. Do not bypass this exit callback.

`interface/textures/allbutingame.ats` does not exist as a literal file. Its real
DXT1 and DXT5 ATS/DDS variants and `interface/fe_attract.lua` do exist and are
retained. Native atlas loading has split-name enumeration; absence of the base
file alone is not a failed lookup. The current sprite bridge merges multiple
atlases independently and does not prove the actual atlas-manager producer.

The installed original executable remains 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`,
volume `F08A6966`, file ID `00030000000345AB`. Its first 64 bytes remain a finite
real input for a separately scoped physical-owner diagnostic. That file is not a
title package and must never replace the actual title label.

## First missing title consumers

Immediately after successful open, `src/title_init.cpp:184` calls
`reset_player_profile`. `MenuTitleInitHost` currently only logs that operation at
`src/game_hosts_menu.cpp:691`. The native caller uses
`LEA ECX,[ESI+650h]; CALL 007FDB20` at `004C9AFA/004C9B00`; its receiver is the
already constructed profile within the actual game, not a new independent F8h
buffer. The complete normal raw reset and constructor already exist in
`native_player_profile_owner.cpp`; the missing work is the genuine application
producer/binding and its dependencies.

| Consumer | Actual current frontier |
| --- | --- |
| Profile reset, 007FDB20 | Unconditional log-only method. Needs the produced game+650h profile, real settings F88980, current online/game publications and SDK settings context. |
| Atlas load, 00AF0060 | Unconditional log-only method. Needs the actual F8C26C manager, descriptor registration AEF280, VFS enumeration/resolution and real texture ownership. Existing host projections do not supply that proof. |
| Frame selection/title activation | Current Source behavior uses `GuiLayoutPage`, `title_object` and projected screen ownership. An implemented log entry is not raw producer/lifetime proof. Source handover also returns on a null screen where Native dereferences. |
| Attract creation/init, 00689D90 | Fresh Source state takes the null-attract branch; creation returns null and init logs. Needs the actual 4Ch owner, base/GUI/page producer, recovered vtable+4 target and full lifetime. `00689D90+4` is not an identified init function. |
| Sign-in query/reset/rebind | 0067C8F0 logs/returns false, forcing log-only A40020 and 67C970. Raw A40020 Source exists, but these menu consumers need the coherent current online/input/binding graph. |
| Movie commit, 004F83B0 | Unconditional log-only method. Needs actual E18D48 owner and genuine child enumeration/visibility; a projected empty screen is not proof of a produced empty child list. Conditional exit needs its actual leaf target. |
| Conditional mission reset/title skip | 916980/68D8A0 remain unfinished when reached. Current Source defaults bound those branches, but do not prove Native global producers. |

## Prioritized next packets

1. **`cc12_title_profile_application_producer_readiness`**, read-only. Claim whole
   functions 4DDB90, 7FEE20, 7FDB20, 436710, 8D4820, 8D45D0 and 8D41C0; own new
   profile-readiness docs/report only. Reuse the raw profile/reset implementation.
   Trace the real game producer and retained lifetime, settings/online/game/SDK
   cells, mission-progress 7FD780/920E10, map/tree and string/pool dependencies.
   Do not fabricate a standalone profile or authorize a menu binding before the
   parent game producer is proved. Shared menu/startup integration stays primary-owned.
2. **`cc12_title_atlas_manager_producer_readiness`**, independent read-only audit.
   Claim whole AF0060, AEF280, AEF3C0 and AEEAF0; use separate docs/report. Establish
   actual atlas-manager construction/registration/lifetime, real texture providers
   and installed split-name routing. Reuse current 886280/BDF4C0 VFS and B33E40
   texture-name dependencies. Discover unresolved manager/texture producer
   addresses before claiming them; do not replace them with the merged sprite atlas.
3. Attract/frame/title/movie producer readiness follows with whole 689D90,
   684E10, 68D760, 68D8D0, 4C1AC0, 518250 and 4F83B0, plus genuine GUI/page and
   press-start dependencies. Recover unresolved virtual targets from actual tables.
   Sign-in/input binding readiness separately owns 67C8F0, A40020, 67C970,
   A3E470, A3E510 and A3EAC0, coordinated with current online/input owners.

These are proposed bounded ownership scopes, not packet-registry promotions or
Source/execution authorization. Exact dependencies and proposed files are in the
JSON receipt. Existing worker leases must be rechecked before any assignment.

## Conditional future witness

Only after all reached producer/consumer contracts close, a new mutually exclusive
option in the existing CLI could run the actual title caller once. Freeze current
inputs before a normal Win32 build, qualify whole linked bodies/relocations/MAP/PE
and cold control flow, then seek separate execution authorization. Use actual
phase 2 and archive tail with cached load true and hardware write-back disabled;
re-audit other startup side effects before calling this a read-only installed-input run.

The witness must record the real manager/registrations/pool, registry vector,
cache/lock/depth, gate list/current name and actual title frame before open, inside
the completed callbacks, after complete close and before shared drain. Existing
public booleans alone are insufficient; any later observation API must borrow the
actual private owners. It must run all real title consumers and accept no added
unimplemented calls. Empty-registry success covers only the observed resolver-false
route, not MPAK provider creation/cache or nonempty-vector behavior.

Keep **all** application/raw-services/menu/frontend/settings/online/SDK/input
owners and every acquired frame outside the exception scope until normal close
and drain. Any exception while the title frame is published/open, including
unrelated consumer or logging failures, retains the whole graph until `_Exit`.
Emergency diagnostics also need a catch-all fallback. No automatic Native
destructor, partial-frame free, shared drain or destructor replay is permitted.
Current `GameStartupHost` already checks retention before menu deletion/shared
drain; a future CLI must preserve that breadth. Successful complete BDCB30 is
required before frame free, then normal shared drain and retired-borrow rejection.

## Evidence level

The historical accepted title package was rehashed: **18,084 phase rows, 170 tool
rows, 9,302 unique retained files, 76 whole objects, 70 complete core members,
8,350 COFF extents, 490 linked extents and 3,207 ordered relocations**. This pass
did not recompile or replay linked-code analysis. Those results remain historical.

Fresh snapshots retain **4,372 current inputs** including full needed Source
providers and frontier owners. All **900 selected historical compiler inputs**
match current files. The one changed older broad-snapshot Source is the accepted
recursive `native_scene_property_bag_storage.cpp`, outside those selected inputs;
the new recursive header is also captured. Whole-repository equality is not claimed.
The primary's independent accepted title build receipt is retained as prior evidence.

Live read-only Ghidra queries verified `bsp`, `/battlestationspacific.exe`, x86
little-endian 32-bit and image base 00400000 before the batch. Ten complete original
body extents and 27 available export files were retained; attract had no existing
decompilation export, so its bytes/range are retained without a new behavior claim.
Two local evidence-driver corrections are disclosed in the report (literal byte
count and a Python `finally` syntax omission); neither ran reconstructed code.

Source normal ownership, historical build checks and fresh input identity do not
establish Native FH3/SEH, raw register/stack ABI, complete title/asset/render behavior
or game validation. This packet performs **zero** Source/probe/startup/game executions.

## Primary review

The integrator independently rehashed 9,302 retained historical files and
4,372 worker input snapshots, compared 4,030 mapped current Source files,
checked all ten installed Native spans and 27 retained exports, and verified
the unchanged 67,814-file metadata inventory. All six whole installed inputs
retain their actual read-only Windows handle identities. The only mapped
Source differences are the independently accepted unlink guard/header fix,
outside the title caller and owner providers.
Receipt `local/cc12_title_execution_readiness_primary_review/receipt.json`:
SHA-256 `d7aa02789989e776dede23b941f5bcf5f680986597a581ea2e8c962d6ebc4559`.
The execution frontier and proposed producer audits are accepted; full actual
title execution remains unready. No Source, build, runtime or ledger credit follows.
