# Orchestrator 6 reconstruction batch G

This batch adds16 complete normal-path reconstructions: three avoid-zone cache
routines, four engine/profile constructors, six task-manager routines, and three
dispatch initialization/cleanup routines. Work stayed in isolated leased
worktrees against `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.
The original installation and imported binary were preserved.

The engine and its real profile/task-manager owners now compose with dispatch,
world and scene construction in one fixture. No14h engine or358h manager input
projection supplies that comparison. The application physics connection remains
future work. The cache adapter operates on installed map geometry; ship sector
activation remains pending live director/request state and arc clipping.

| Evidence | Result | Boundary |
| --- | --- | --- |
| Win32 Release and existing CTests |2/2 pass|Separate from gameplay|
| Query native comparison |1,536 cases;8,484 callbacks;4,296 selected-node records|Six x87 settings, masked exceptions; output FPenvironment not compared|
| Installed-map cache probe |21 zones,2,193 corners,426 selected segments; reuse/crossing/cleanup pass|Explicit layer3 query; existing scene projection|
| Engine/profile native comparison |Five paths,12 paired buffers|Zero-worker owner and checked allocation-null branches|
| Task native comparison |Six snapshots,1,316 canonical words,16 LIFO callbacks per side|One worker/group0; real OS/CRT; fixture payloads|
| Dispatch native comparison |12 configurations,78 doubles each;26 lock lifecycle checks|Shared CRT service and documented exception callback boundary|
| Combined owner-to-world comparison |29 buffers and allocation records;938,692 bytes identical|Actual owners, source one-worker creation/join; collision/solver execution excluded|

The native task destructor retains worker handles and100 completion events.
Fixture cleanup closes those separately; source does not silently change native
ownership. Locked Ghidra repairs restored the scheduler/free tails and full
task-manager destructor, and defined the17-byte dispatch startup. All16 reviewed
names were saved with prior annotations retained, and affected exports refreshed.

The combined mission run at `2195d2f5803a6417cafc85ffe1275f4da12bb6c7` completed120 frames,
created77 units and14 ship navigation controllers, and exited0. Its executable
SHA256 is `353b16161d609a64bd52149dfa2c7e28fe0f289fb054a393c48c92c27ab4ccda`. The first attempt encountered another
orchestrator's live single-instance mutex; that attempt is preserved separately.
The movement summary still reports NaN for Airfield2, as the earlier F run did.
The successful loop therefore does not establish finite motion or gameplay.

Compiled-input changes after that run: cmake/startup.cmake, include/bsp/game_hosts_gunnery.hpp, include/bsp/game_hosts_units.hpp, src/game_hosts_gunnery.cpp, src/game_hosts_units.cpp.
The final integration record is `reports/orch6_reconstruction_g.json`; its
compiled/run commits and source hashes delimit the actual validation.

All171 worker artifacts are copied and hash-verified under
`local/worker_evidence_g/`; `local/orch6_worker_archive_g.json` is the inventory.
The query fixture's compiled source precedes one comment correction in5B888D82;
the proof records both hashes and verifies the change affects no executable logic.

See `SHIP_AI_AVOID_ZONE_SEARCH.md`, `SHIP_AI_AVOID_ZONE_SEARCH_PROOF.md`,
`DYN_ENGINE_RUNTIME.md`, `DYN_TASK_MANAGER.md`, `DYN_TASK_NATIVE_PROOF.md`,
`DYN_DISPATCH_INITIALIZATION.md`, and `DYN_WORLD_OWNER_CONNECTION.md` for
native ABI, call sites, exact comparison domains and remaining dependencies.
The world connection also corrects an unused F-fixture table relocation:
D7A0AC has four methods followed by RTTI; the erroneous fifth-word relocation
was not executed by the old construction comparison and is omitted in this proof.

The continuing H packets recover00415970 arc clipping/004F3BA0 geometry,
C420E0 world creation and engine-list registration, and actual avoidance-state
ownership. Its C420E0 free-tail flow repair is already saved separately.
The reconstruction goal remains active; full physics and gameplay validation
are still incomplete.
