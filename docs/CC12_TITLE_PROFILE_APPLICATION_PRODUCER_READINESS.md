# Actual title profile/application producer readiness

Read-only packet `cc12_title_profile_application_producer_readiness`, current Source at
`325879e864505033b834fc5b2a8cd8c482b42f66`. **Not ready for application/profile binding.**
The first unresolved producer is the application owner at the established
`0073E150 / 0073E163 / 0073E1A1` allocation, whole-game zeroing and constructor schedule.
Current startup does not construct `GameNativeGameRuntime`, compose its contexts,
retain its raw `71A0h` game allocation, or attach it to `GameFixedStepHost`.

`GameNativeGameRuntime` is already implemented. It owns persistent operation frames
and its Source virtual table, but **borrows** the actual game allocation and the
constructor/lifetime service graph. Its existence does not provide the missing
application owner. Production references and complete files are retained in the
[report](../reports/cc12_title_profile_application_producer_readiness.json).

## Actual game, profile and publication order

`src/native_game_construction.cpp:141` calls `007FEE20` with exactly `game+650h`.
The profile constructor initializes the raw headers/sentinels and calls `007FDB20`.
It does not initialize the existing mission-progress pointer at profile `+64h`
before reset. The known caller's complete zeroing of the `71A0h` allocation therefore
matters; a default-constructed raw struct or a separate zeroed `F8h` profile would
not establish the actual application construction contract.

The game constructor publishes the actual `E188A8` game cell only after profile,
resource parser, grid and mission-Lua construction, before processor/physics-world
initialization (`src/native_game_construction.cpp:202`). Runtime state becomes live
only after the entire constructor returns. A failed constructor can thus retain
partially initialized storage and a published game; its operation must survive.
Diagnostic acknowledgement does not free, undo publication, or permit replay.

The runtime checks equality of constructor/lifetime game and grid publication cells
and canonical engine/world/physics identities. It does not automatically establish
that the separately supplied profile settings context uses that same game cell.
The missing application composition must make that identity explicit.

## Settings, online state and private owning strings

`GameNativeSettingsProcess` already owns the real `BCh` settings (`F88980` domain),
current online-manager `F8ABE8` and game `E188A8` cells, and the profile settings
context. Its `settings()`, `profile_context()` and `game_00e188a8()` accessors are
the existing providers. A future application context must borrow these actual
cells, not duplicate them in a profile-specific owner.

The process settings provider already uses `GameNativeStringProcess`. The VFS App
and singleton domain borrow its canonical pool, disabled flag and manager cells.
Game/profile owning strings and the raw profile destruction context must remain
in this same private domain. Different storage bridge objects are permissible
only when they borrow the same actual cells. Every raw string header has an
ownership lifetime; converting semantic host strings does not establish it.

`SoundServices` binds its loaded `XLiveLibrary` to the real typed settings adapter;
unbinding is rejected while the online owner still exists. The adapter resolves
ordinal `5331` and preserves the original seven-DWORD SDK signature.
`008D4820` stores five control defaults, then follows the current online manager's
phase `+28h == 2` and current selected-user byte `+119h` into `008D45D0` when reached.
That path performs the two SDK reads, reloads the selected user at `+11Ch`, and on
success writes the **current** game `+6ACh` as well as settings. The initial SDK
status is ignored and the second-call failure leak remains in Source.

Current `GameStartupHost` explicitly leaves online publication null because the
parent pipe/IPC/application owner is unresolved (`src/game_hosts.cpp:2132`). This
is a current startup gap, not proof that a future complete startup may suppress
the selected-user branch. Profile construction occurs before the constructor's
late game publication: preserve this ordering and establish the actual branch
domain rather than publishing the new game early or fabricating SDK results.

## Existing profile providers and full-game dependencies

The normal reset has concrete Source providers:

- `007FD780` destroys captured old mission progress, then the profile caller frees
  that captured `24h` owner. `00920E10` constructs its score and two counter-tree
  sentinels; destruction clears real full ranges and frees current sentinel cells.
- Owning counter keys, transient strings and mission-score payloads route through
  actual collection/string Source operations, including `00593570` payload teardown.
- The three string sets, counter/transient trees, captured vector range, alias
  lists and lobby nodes use existing Source. `RANK` is a real owning pooled key
  passed to `005070C0`; its temporary is released after indexing.
- `007FD8A0`, reached by normal whole-game destruction at `game+650h`, releases
  profile collections and strings through the same raw pool context.

These are implemented providers, not a new standalone profile factory. Full
constructor context still requires every reached external service in
`include/bsp/native_game_construction.hpp:78`:

| Context/service | Current readiness |
| --- | --- |
| Input configuration, arrays, embedded state, actual name and constants | Normal Source exists; application must provide actual inputs and ownership |
| Unit/rank tables, `008D9150 / 00727BD0` | Normal Source exists; no production `NativeGameTablesContext` composition found |
| `00432650 / 0087D7B0` global configuration and `Globals.lua` | Normal Source exists; actual Lua/files/sound/FOV loader context is not composed for the game |
| `00717E80` resource parser registration | Actual VFS parser context accessor already exists |
| Three `0070BD70` grids | Renderer already exposes the genuine grid context; bind its real owners and preserve explicit descriptor preimage words |
| Canonical Dyn engine/world/contact graph | Existing process initializes it; runtime enforces matching identity, but startup does not construct/attach a game |
| Full game lifetime | Normal Source exists; actual reached virtual-scalar/terminal dispatch and borrowed context lifetime remain uncomposed |

The production source search finds no concrete application implementation of
`NativeGameLifetimeCalls`' required virtual dispatch, no game construction/lifetime
aggregate outside the runtime's borrowed copies, and no caller of its production
fixed-step attachment. `src/game_hosts.cpp:2186-2241` proceeds from the GUI diagnostic
sink to effects/decal/menu/title without the actual game construction step.

## Required closure before any title binding

The next producer work must establish one actual application owner that retains
the `71A0h` allocation, exact input name, constructor/lifetime contexts and all
borrowed services. It must resolve the table/config-loader composition and real
reached lifetime dispatch, preserve the allocation/zero/publication schedule, use
the actual settings and canonical string cells, and keep operations alive through
partial failure and normal destruction. Existing grids/parsers/Dyn providers
should be borrowed rather than replaced.

Only after that owner completes construction can the actual title host receive
the same `game+650h` profile with a fresh retained reset operation. No cohesive
Source/menu connection is claimed ready by this report. New x87 or register-ABI
recovery was unnecessary here; if the next producer packet requires it, that
explicit address range belongs in a bounded Astra packet before analysis.

## Evidence and limits

The capture retains **4,037 complete current Source/support files**, the whole
installed Original PE, **seven complete Native bodies (3,503 bytes)** and **21
existing export files**. Live Ghidra verified project/program/language/base before
the requested seven body-extent queries. Complete body bytes come from the retained
PE; this read-only packet does not claim new live-memory equivalence.

Six September archives (`R105`, `R106`, `R107`, `R109`, `R124`, `R155`) are retained
whole and match their recorded archive SHA-256. All **1,706 recorded members** match
byte counts and SHA-256; ZIP CRC checks pass. These include historical whole
Source, objects, libraries, executables and logs. They were **rehashed**, not rerun.
The October current Source snapshot is separately attributed and is not relabelled
as historical compiler input. Historical controlled providers/SDK tests do not
prove current ordinary startup, selected-user/account execution, or whole-game
destruction. R155's runtime/physics fixture does not close the application owner.

Artifact root: `local/cc12_title_profile_application_producer_readiness_20261008a/`.
The JSON report pins capture, source, native and archive manifests. Its
`whole_artifact_manifest.json` also seals the complete final local evidence and
two tracked deliverables. A first archive-validation attempt found the old ZIP's
`workspace/` prefix; the preserved corrected driver requires a unique path match
and verifies every member. It ran no game code.

Established original ABIs are copied with attribution from the retained prior
reports: `004DDB90` takes ECX game and one stack name header, returns game and RET4;
profile/empty-name constructors use ECX and return their receiver; reset has no
semantic return; settings routines use ECX and RET, with the control reset's tail
jump and selected-user SDK signature preserved. These Source context interfaces
are not binary entry replacements. No new Source/build/test/probe/game execution,
Ghidra mutation, ledger edit, packet promotion or menu wiring occurred.

Primary review accepted these readiness findings after rehashing all 4,098
retained artifacts, six complete historical archives and their 1,706 recorded
members, checking all seven Native spans against the current installed PE, and
checking 13 selected current application/profile/host Source inputs. Eight
other snapshot inputs differ through subsequent accepted string/shader and
registry integration; the snapshot is not current compiler-input evidence.
The full lifetime contract also includes inherited `virtual_04`, alongside
scalar and terminal dispatch, as the following construction-context review
establishes. The application owner remains unready.

Primary receipt: `local/cc12_title_profile_application_producer_primary_review/receipt.json`,
SHA256 `4aef870d87409928e7d65b3ae8da3926b3dd36ff49805c585fbb9474ab0d279c`.
No Source change, new build, entry execution, or ready-owner credit is added.
