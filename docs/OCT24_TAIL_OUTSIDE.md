# October 24 outside-partition and unreferenced audit

Baseline: `515837c10`; copied read-only index built `2026-10-24T08:31:03.596686+00:00`.
The audit reproduces the board's **20,726 target / 316 game_outside / 791 unreferenced** exactly.
This packet performs no reconstruction, Ghidra mutation, C++ edit, or game run.

## What game_outside measures

`tools/progress_board.py::build` forms `game_any` from non-funclet function starts after its
library address ranges, tag categories/prefixes, library-name prefixes, and explicit
`library_instantiation_out_of_scope` exclusions. `FUNCLET` also contains `thunk_`.
The target is `game_any AND (referenced OR reconstructed_union)`, where the union consists
of historical whole-function ledger records and the board's historical column-zero,
address-suffixed C++ signature matches. `referenced_functions` accepts `call`, `tailjmp`,
`addr_taken`, `data_ptr`, `vslot`, and `lua`; direct edges from FUNCLET-named sources do not
qualify. Guessed `vcall?` and EH metadata do not qualify.

`game_outside` then counts target rows whose **functions.segment IS NULL**. It does not
subtract reconstructed rows. Consequently, it measures missing partition assignment,
not 316 missing implementations. There are 276 rows at/above `00c30000` and 40 earlier
holes. A null segment does not bypass any library exclusion. The per-function board JSON
omits null-segment rows, so its visible partition bands cannot explain this count alone.

The reference predicate is a conservative incoming-edge census, not a root-reachability
proof: an incoming edge from another dead function can still qualify a target.

## Exhaustive outside classification

| Disposition | Rows | Finding |
|---|---:|---|
| Existing implementation record pointing at an existing C++ file | 256 | 216 also match the historical source-signature detector; the remaining 40 use recorded bodies, macros, naked functions, or names/signatures the detector does not recognize. Their prior verification and ABI limits remain in their records. |
| Recorded-only dead-chain census, no implementation | 53 | Every record has coverage `none`, source `docs/CC13_DYN1.md`, and evidence explicitly saying `Not reconstructed`; none has historical source-body credit. |
| Explicitly partial implementation | 4 | Native shape/body creation and constrained hull projections; details below. |
| Compiler-boundary projection | 3 | C++ cleanup/boundary models with native exception transport excluded; details below. |
| **Total** | **316** | All addresses are listed by disposition below. |

By tag the same set is 220 `block_dyn`, 42 `rtti_vslot`, four `trivial_body`, and 50 untagged.
The 53 recorded-only rows are genuine missing bodies under the board's target policy,
not vendor-library exclusions. They have been corrected through `bsp.py ledger add-function
--replace` to `metadata_only_no_function_body_verified_oct24`. The original status is
preserved as `previous_status`, and the original dead-chain evidence, `coverage:none`,
source, name, and ABI qualification are preserved. No body is invented. The parent board
status filter keeps these rows in the target while removing false completion credit.
The old report's prose says 52, but its explicit Recorded-only list and the ledger contain
53 distinct addresses; no peer-owned report was rewritten.

The three implementation records without a literal address in their source were checked separately: `00c357a0` is an existing jump thunk to `dyn_convex_shape_free_00408040`; `00c35bf0` SupportVertex is composed into hull support-seed generation, and `00c35de0` AppendUniqueNeighbor is the local `add_neighbor` lambda in hull-data construction. These remain qualified semantic composition, not native ABI wrappers.

The four partial bodies remain correctly qualified in their existing ledgers:

| Address | Existing coverage / remaining scope |
|---|---|
| `00c35e80` | Hull CleanupPoints: packed XYZ, epsilon 0.001, non-null scale output; both native degeneracy paths covered. Original EDX output plus stack inputs, RET18h. General stride/epsilon/output variants are not claimed. Fragment ledger; source signature gives historical board credit. |
| `00c5dae0` | Hull GenerateTriangles: constructor flags 1, stride 12, epsilon 0.001, limit 4096. Original EAX output plus descriptor on stack, RET4. Alternate descriptors are not exposed. Fragment ledger; source signature gives historical board credit. |
| `00c5c940` | Native shape attachment accepts type 4. Original ECX shape descriptor / EDX body, RET. Native cases 0/1/2/5 and invalid selectors remain outside the C++ API. |
| `00c5d580` | Native static/dynamic body storage and convex shapes. Original ECX world / stack descriptor, RET4. Other shape types, complete world ownership, and native failure/SEH behavior remain qualified. |

See `docs/AVOID_ZONE_DYN_HULL.md` and `docs/DYN_BODY_CREATION.md`; existing build/fixture
claims are inherited evidence, not newly executed validation. These are real partial
bodies, so no metadata-only status was applied.

Follow-up assembly inspection classifies `00cb89ad`, `00cc1a01`, and `00cc1b36`
as `compiler_generated_out_of_scope`. Each is exactly two instructions/ten bytes:
MOV EAX,FuncInfo then JMP `00bf6b43` (`__CxxFrameHandler3`). Their FuncInfo values
are `00def8d4`, `00dfa900`, `00dfaaa0`; their references are parent prologue/EH
metadata, with no independent game cleanup instructions. The original
`compiler_boundary_*` values and qualified parent cleanup projections remain
preserved as evidence; native exception transport is not claimed.

`00c36990` is the important edge case: its fragment ledger and existing
`pool36_free_thunk_00c36990` source body retain it in the target although all seven incoming
tail transfers are through unwind funclets. `007289e0` has only two unwind tail transfers,
no body credit, and is the sole game-eligible null-segment row excluded from this count.
Neither pool code nor gameplay-host ownership was changed.

## Unreferenced sample and complete edge cross-check

The 791 rows split into 743 `block_dyn` (all segment 0), 14 `game_code`, 33 untagged,
and one `trivial_body`. Across the entire set, 728 have no indexed incoming edges;
63 have only raw `tailjmp` edges: 138 sites from `Unwind@` sources and one from a
`thunk_` source. No omitted `addr_taken`, `data_ptr`, `vslot`, or `lua` edge was found.
The policy therefore excludes these rows correctly. This is not a claim that exception
cleanup can never execute: EH-only bodies are excluded by the board's explicit policy.

Every sample below was queried with `bsp.py callers ADDRESS --kind
call,tailjmp,addr_taken,data_ptr,vslot,lua,eh --limit 8`:

| Category/sample | Caller evidence |
|---|---|
| Dyn header / math: `00401040`, `00409750`, `00413560` | Zero incoming sites, sampling the start, middle, and end of the Dyn unreferenced address list. |
| game_code: `0076ada0`, `007a2930` | Zero incoming sites. |
| untagged fragment: `004b4f10` | One unwind tail transfer, normalized by callers to `00b1b680`; no qualifying ordinary/data/slot/Lua edge. |
| untagged cleanup: `009252f0`, `007289e0` | Three and two unwind tail transfers respectively; no qualifying edge. |
| untagged renderer: `00b28210` | Zero incoming sites. |
| untagged thunk chain: `008db510` | One incoming tail transfer from `thunk_008DB510` at `008dbaa0`; that thunk itself has only two unwind tail transfers. |
| trivial_body: `0078e770` | Three unwind tail transfers; no qualifying edge. |

No unreferenced body was reconstructed. Named cc13 segments 67/40b/58b and cc9 gameplay
hosts were not edited, irrespective of the empty lease registry at entry.

## Undefined-code edges outside the denominator

`bsp.py undefined --limit 12` reports eight referenced chunks from 30,139 undefined chunks;
`--direct --limit 10` reports four. Undefined chunks are not Ghidra function starts and
therefore are absent from both the 316 and 791 counts.

| Chunk | Evidence and disposition |
|---|---|
| `00529120` (211 bytes) | Ordinary tail transfer from `00529810`; existing `hud_counters_hide_right_of_edge_00529120` in `src/frontend_seg15_22_cc13.cpp`. Header explicitly calls it a shared tail with no Ghidra function. No reconstruction gap found. |
| `00530a60` (440 bytes) | Tail transfer from `00532360`; existing fragment ledger and `src/frontend_prompt_layout.cpp`. No reconstruction gap found. |
| `006e5f10` (220 bytes) | Data pointer / vslot 89 at `00cf9a7c`; existing function ledger points to `multi_rack_on_fire_message_006e5f54`, a build-tested reference model in `src/gameplay_seg33_cc13.cpp`, not wired. Preserve that qualification. |
| `007b3aa0` (128 bytes) | Data pointer / vslot 89 at `00d05634`; four outgoing calls. No source/ledger address match found; prior segment-47 source explicitly ends at `007b3a9b`. **Credible missing-body audit candidate beyond the board denominator**, requiring a separately owned packet and native listing/ABI verification. Guessed renderer `vcall?` rows do not establish callers. |
| `00a82100` (302 bytes) | Only a self-call; no external direct/data reference in the sweep. This does not establish a reachable game gap; adjacent STL naming alone is not a scope proof. |
| `00bf5752`, `00bf86e0`, `00c2e020` | Within board-excluded CRT/import ranges. Correctly outside the game target. |

No undefined chunk was promoted to a Ghidra function or reconstructed in this audit.

## Reproduce and validation

Run `python tools/audit_board_outside.py --board local/output/oct24_board_before.json` against
the baseline board JSON and index, or omit `--board` for a current audit. The source-history
cache is keyed by the exact `MAIN:src` tree; the full per-address evidence is written to
`local/output/oct24_outside_audit.json`. This includes every outside and unreferenced row,
its source/ledger credits, status, tag, and raw incoming-edge counts. The script uses
`progress_board.py` predicates directly and changes no ledgers.

Validation: exact baseline KPI reproduction; all 256 implementation-record source files
exist; all 53 metadata corrections preserve original record fields except the status and
appended audit evidence, adding only `previous_status`; unaffected shard records remain
identical. Python compilation and the post-correction audit pass. No C++ change requires
a new build in this worker, and no new tests were added. No runtime, native ABI, fixture,
or gameplay validation was performed by this packet.

## Exhaustive outside address lists

### recorded_only_no_body (53)

```text
00c316f0 00c320d0 00c32a90 00c33250 00c33350 00c33420 00c33490 00c33510
00c35390 00c35a00 00c36850 00c368f0 00c369e0 00c37030 00c37180 00c37390
00c37580 00c37a30 00c37b10 00c39040 00c39100 00c3f190 00c40800 00c40900
00c40a30 00c40b30 00c40cc0 00c40d50 00c40e50 00c40fc0 00c410e0 00c41190
00c41230 00c44360 00c48100 00c48e30 00c4c550 00c4c5d0 00c4c700 00c4cbb0
00c4cbf0 00c4cc90 00c4ce60 00c4d050 00c4d370 00c4d5a0 00c54da0 00c55a60
00c55d30 00c55df0 00c5a850 00c5a8b0 00c5daa0
```

### partial_body (4)

```text
00c35e80 00c5c940 00c5d580 00c5dae0
```

### compiler_boundary_projection (3)

```text
00cb89ad 00cc1a01 00cc1b36
```

### existing_implementation_record (256)

```text
00413920 0042a8a0 0042a8b0 0042a8c0 0042a8d0 0042a8e0 0042a8f0 0042a900
0042a910 00465570 0051e780 00568a30 00568b60 00680da0 0071b810 0071b870
0071b8d0 0071b940 0071b9b0 007289b0 007289c0 007289d0 008dcd70 00aa6740
00ad53a0 00af37a0 00b1bf20 00b1bf30 00b4b1e0 00b4b2c0 00b4b2e0 00b4b370
00b4b390 00b4b3f0 00b4b480 00b80f10 00b8a270 00bbddf0 00bd2ed0 00be4430
00c30900 00c30930 00c309d0 00c30d10 00c31060 00c313a0 00c31a40 00c31c30
00c31cc0 00c31d70 00c31dc0 00c31ea0 00c31f20 00c31f40 00c31f60 00c31f90
00c31fc0 00c32000 00c32050 00c321b0 00c32250 00c32420 00c32430 00c32450
00c32490 00c32510 00c327f0 00c32830 00c32860 00c32a60 00c32af0 00c32b10
00c32b30 00c32bb0 00c32bd0 00c32be0 00c32c20 00c32c30 00c32ce0 00c32d20
00c32d50 00c33140 00c33650 00c336c0 00c33710 00c33ca0 00c34240 00c347d0
00c34ba0 00c34cf0 00c34f70 00c35020 00c350c0 00c35160 00c35260 00c35300
00c35330 00c35360 00c35400 00c35480 00c35630 00c356d0 00c357a0 00c357b0
00c357d0 00c35810 00c358a0 00c35bf0 00c35de0 00c36730 00c36990 00c36ac0
00c36b60 00c36c00 00c36c30 00c36e20 00c36f10 00c37290 00c37300 00c37430
00c37450 00c375f0 00c37680 00c37690 00c37740 00c377f0 00c37860 00c37b50
00c37c40 00c37d30 00c37de0 00c37e00 00c37e20 00c37e50 00c37e70 00c37f10
00c37f40 00c37f60 00c38040 00c38070 00c38470 00c385b0 00c386e0 00c38810
00c38970 00c389c0 00c38f90 00c3cc30 00c3f410 00c3f4d0 00c3f650 00c3f760
00c3ffe0 00c40140 00c40d70 00c40d80 00c40f50 00c40ff0 00c412d0 00c41550
00c41ad0 00c420e0 00c421b0 00c42230 00c42530 00c42ba0 00c42ed0 00c431d0
00c437d0 00c43aa0 00c43c00 00c43ca0 00c43ea0 00c44000 00c44090 00c445b0
00c44780 00c47aa0 00c47ca0 00c47f60 00c47f90 00c47fc0 00c47ff0 00c48020
00c48050 00c48080 00c48170 00c482b0 00c48330 00c48be0 00c48fd0 00c49230
00c49a30 00c4b550 00c4b610 00c4b9b0 00c4bc50 00c4bcd0 00c4bd60 00c4be10
00c4c270 00c4c2a0 00c4c320 00c4c380 00c4cda0 00c4d980 00c4daa0 00c4dc60
00c4dde0 00c4de40 00c4f040 00c4f140 00c50310 00c50390 00c50470 00c50740
00c50dd0 00c51480 00c51740 00c517d0 00c518d0 00c51c20 00c51ef0 00c53010
00c535e0 00c53630 00c549d0 00c54aa0 00c55ea0 00c55f50 00c55fc0 00c56cf0
00c57020 00c57070 00c57880 00c57b90 00c57c40 00c57f50 00c57ff0 00c582e0
00c58520 00c585b0 00c58690 00c58840 00c58920 00c58eb0 00c5b1b0 00c5bb30
00c5c540 00c5c710 00c5c7a0 00c5cae0 00c5d900 00c5ddd0 00c5deb0 00c5df30
```
