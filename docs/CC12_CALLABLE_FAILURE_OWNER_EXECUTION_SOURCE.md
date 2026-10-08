# Genuine Source-owner callable failure diagnostic

**Implemented `--qualify-vfs-failure-owner` in the existing `bsp_game` target. The normal Win32 build and all three established checks passed. One actual diagnostic run through the normal parent/canonical-data child passed with exit 0.** Primary review is still required for acceptance.

Baseline: `0348b7badac173be2022545e92bb6c086d6f1b9f`. The only Source changes are `src/game_main.cpp`, the existing option parser in `src/game_hosts.cpp`, and an appended option flag in `include/bsp/game_hosts.hpp`. The application/runtime/callback providers, build registration, ledgers and Ghidra are unchanged. Additional Original function credit is zero.

## Implemented route

The optional branch runs after the real `CanonicalCrtV2` reservation, data validation and `GameNativeCanonicalDataOwner` creation, before normal pool/game startup. It constructs the current concrete `GameSingletonHost` and `GameNativeVfsApplication` with the actual log, verified data and image path, then initializes the genuine core once. It neither creates a manager fixture nor substitutes a publication, service, allocator or callback.

The real A0, shared singleton manager, production publication cells and three core factories are checked. Full A0 bytes, the raw14h singleton header and all active registration records are captured for comparison only. The existing borrowed finite binding is first called with its original numeric identity. The public owning application API then publishes the exact Source C3 target at `+90`, returns the same owner, and is called again to check idempotence. The real finite binding is called with that published target; finally the named raw notifier is called once on the same owner.

The loaded raw symbols must belong to the rebuilt Win32 image, lie in readable executable memory, and have the complete expected bytes `C3` and `8B8190000000FFD0C20400`. Code addresses come only from the declared symbols. No arbitrary target or C3-pattern search is accepted. EDX `13579BDF` and discarded stack word `2468ACE0` are independent scalar witnesses; neither is obtained from manager `+18`.

Heap-owned application/host variables outlive the failure handler. An interrupted initializer or drain records and flushes failure, then follows the existing `_Exit` retention policy. It does not unwind these owners first, retry initialization, invent rollback or run pool callbacks against partial state.

All borrowed use ends before the real shared drain. The application/runtime/data/services/code remain alive during `host.shutdown()`. Afterwards the check reads only the still-live publication cells and invokes the guarded application API; it never reads the freed A0/vector. Already-drained C++ owners are destroyed before normal return. With the option absent, execution continues into the existing normal startup.

## Observed one-run result

Run: `2026-10-08T16:13:54.047457+00:00` to `2026-10-08T16:13:54.203709+00:00`. One attempt, zero retries, parent PID `114356`, normal parent/child exit `0`.

| Live evidence | Observed value |
| --- | --- |
| Genuine A0 owner | `0078D6C0` |
| Shared singleton manager | `007C4718` |
| Existing VFS / singleton publication cells | `007810A4` / `00783980` |
| Active singleton vector | `007813B0`, six entries, includes A0 |
| Core factory count | 3 |
| Real physical-pool atexit registration | returned 0 |
| Rebuilt module base / timestamp | `10000000` / `6AC7BFA3` |
| Loaded C3 | `10218EE0`, RVA `00218EE0`, whole byte `C3` |
| Loaded notifier | `10218EF0`, RVA `00218EF0`, whole 11-byte body |
| Both raw code regions | `PAGE_EXECUTE_READ` (`20`) |

Independent log analysis reconstructed twenty complete record/code snapshots. The original finite dispatch preserved all records. Publication changed **only A0 byte offsets 90, 91, 92 and 93** to the exact loaded C3 address; `+8C`, `+18`, all other A0 bytes, both publications and all singleton registrations were preserved. Repeated publication, the published finite call and the one raw notifier call preserved the entire published record and registration snapshots.

The notifier returned once. The shared drain completed, both live publication cells became null, and a fresh owning application request hit the runtime-specific retired guard before owner access. The already-drained application and host were destroyed, and the child/parent returned normally. The physical-pool cleanup callback was not separately instrumented; its real registration result and normal process exit are recorded.

The windowed process produced no captured stdout; the actual child `GameHostLog` is retained in `diagnostic01/diagnostic.log`. Rebuilt EXE SHA-256 `c43ab3a60b365b6fb4d4c59a457148ebe91e29bf8437d16e2a1e218a403b45f0` and Original input SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` were unchanged before/after execution. No Original code, mount/read path or game installation write was performed by the diagnostic.

## Whole-body and build evidence

Normal `./scripts/build.ps1 -Diagnostic` passed `reconstructed_math`, `native_math_differential` and `tool_tests`; no test or target was added. The existing native differential check retains its established seed scope.

All **968 accepted provider bodies** retained exact symbols, whole bytes and ordered relocations. Another **3,014 old CLI bodies** retained that same exact correspondence. Four affected C++ bodies and twelve generated WinMain EH path identities changed under old symbols; all sixteen complete old/new bodies are retained. Six removed cleanup names each have exactly one complete-byte/ordered-relocation correspondence across all 4,123 current functions. Those are actual compiler paths, without shortened-name matching or an Original EH/ABI claim.

Twelve complete objects were retained, including eight unique complete members of the actual core archive and four direct game objects. The fresh normal map/PE replayed **nine complete production bodies and all 38 relocation operands**: public/private publication APIs, qualifier, finite helper, C3, notifier, unchanged installer and both existing finite callers. The new diagnostic's complete **2,010-byte / 496-instruction** body also replayed all **84 operands**. Its direct named calls are application API three times (including retired rejection), finite binding twice, notifier once. Exact map symbols, sections and all same-VA aliases are retained; no global byte-pattern uniqueness is claimed.

## Retained inputs and boundaries

Evidence family: `local/cc12_callable_failure_owner_execution_source_20261008a`. Physical phase copies total **13,460 rows**: 4,486 before editing, 4,486 before build and 4,488 after build. All actual inputs for the changed CLI translation units were copied before build and agree afterward: 478 entries for `game_main`, 599 for `game_hosts`, 674 unique inputs.

Two unrelated incoming-main rows, `native_parent_list_header_storage.cpp/.hpp`, were copied **after build only** by this packet. Three project libraries have pre/post-build artifact copies; all 23 actual link libraries were also retained after execution. External SDK/CRT library copies keep that post-execution timing. No exhaustive compiler/linker/runtime-DLL/OS closure is claimed.

Static gate SHA-256: `5ecd16675c197ffcc97621cfe63f7ca2945833d950b9e691b72b871e3e5fb8a2`.
Independent runtime audit SHA-256: `611450f678ed2ebd8404d8fc84bdc269f3670659d67b121695dd1dbcfcd547fa`.

This qualifies one genuine current Source owner, callable publication, named notifier use and shared drain. It does not establish allocation-failure/partial-state cleanup, concurrency, complete Original register/flag ABI, BF5030/connected raw reads, later VFS services or gameplay equivalence.

The [JSON report](../reports/cc12_callable_failure_owner_execution_source.json) contains the full snapshots, exact changed-body/alias records, complete linked replay and physical evidence references.

## Primary acceptance

The primary review independently rehashed all 13,495 retained phase, proof and runtime/library artifacts. It verified the complete old-body correspondences, all 4,123 current functions through a bijection with exact symbols or constrained compiler path identities, eight unique current core members, ten complete linked bodies and all 122 ordered relocation operands. The primary normal Win32 build passed all three existing checks.

One fresh execution of the primary executable passed through the ordinary parent and canonical child, exit 0. The genuine owner was `007DDB78`; all twenty complete snapshots were independently replayed. Only offsets `90..93` changed; repeated publication, both finite dispatch paths, the one named notifier call and shared drain passed. The primary image's exact named raw entries had RVAs `00218ED0` and `00218EE0`; loaded addresses and whole bytes were correlated to its own map and PE rather than the worker image.

Primary evidence is `local/cc12_callable_failure_owner_execution_primary_review/receipt.json`, SHA-256 `aecab03f0ae0cd0b1d344ebc9306f5c7de35f5427c1f04ee20f02535ef345264`. The preceding static gate is retained separately. This accepts the bounded Source owner/use/drain diagnostic, contributes zero additional Original function credit, and leaves connected reads, complete Original ABI, startup and gameplay validation open.
