# Gunnery health callback pair (cc11_health_pair)

Addresses: `00877B90`, `00879070`, `008777D0`.

The pair tests the OFF binding in `GUNNERY_HEALTH_CALLBACK_GUARD_CC11.md` against
the routed USNOS HQ neutralization regression from `SHIP_AI_OPEN_ITEMS.md`
205.2/206. This is reconstruction-runtime evidence, not original-game or native
ABI verification.

## Pinned setup

Both `tools/pair_export.py` exports use commit
`f6c6d992aa18cc243c7854a6c1be83ef220830f9`.
`kCaptureStatePartyFromUnitBound=true` in both exports so the native one-party
field projection exposes recapture/neutralization loops. Only
`kUnitHealthCallbackGuardBound` differs: OFF versus ON. The capture switch
remains OFF in tracked source.

The 11,331 exported files were compared byte for byte: only
`src/game_hosts_gunnery.cpp` differs. Each variant has its own build directory,
executable and log under the worker's ignored `local/` directory. Builds are
serialized after the primary's main build.

The unchanged 85-line `s41_os_f12.txt` orders were copied from the existing
`cc9-ships41` worktree. SHA-256:
`10f74bff851a2b414814e2105de8c8868ee9da2af72e155fc6aa3081100ebbaf`.
Run parameters: USNOS, 80,000 mission frames, 0.05 seconds/frame, outer frames
80,200, press-start frame 30, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.
The random streams are the existing measurement substitution.

## Results

Both full Win32 export builds passed the two tests available in a clean export:
`reconstructed_math` and `tool_tests`. The native seeded test from the primary
checkout is not present in these clean exports; its earlier main result is not
counted as a variant result.

| Variant | Executable SHA-256 | CTest |
| --- | --- | --- |
| OFF | `63a2c96e7502ee1056ccc3521087a57e26cb4e428393efabc86173f3bb5f591e` | 2/2 passed |
| ON | `04f600208a1aa73fa7e907b6cd7a86cd8ce5e9ad79e1ab81ddd286f483215b4d` | 2/2 passed |

Both variants completed the 80,200-frame outer loop and all 80,000 motion steps
(4002.73 simulation seconds). The callback mechanism is supported by the pair;
neither run is a clean passing renderer/launcher run. Following independent
primary review, `kUnitHealthCallbackGuardBound` is **ON**. The capture-party
switch and every other tracked behavior switch retain their prior values.

| Measurement | OFF | ON |
| --- | --- | --- |
| Prior-party-0 neutralizations | 45 | **0** |
| Initial prior-party-1 neutralizations | 3 | 3, same bases and times |
| Health-zero callbacks reaching capture | 500 | 3 |
| HQ2 / HQ1 resynchronizations | 13 / 33 | **1 / 1** |
| No-dispatch candidates / suppressed | 805 / 0 | 857 / 857 |
| Final HQ1 / HQ2 / CB2 party | 2 / 0 / 2 | **0 / 0 / 2** |
| Score display, bases captured | 1 | **2** |
| Death rows / kill credits | 471 / 471 | 467 / 467 |
| Objective binding event sequence | 12 events | Identical 12 events |

Initial neutralization times match exactly: HQ2 1484.75, HQ1 1885.30, CB2
2047.15 seconds. OFF first resynchronizes HQ2 at 1702.90 (line 313214), then
neutralizes it again at the same time (313352). ON resynchronizes it once at
1702.90 (292100) and never neutralizes it again. HQ1 similarly resynchronizes
once ON at 2386.00 (488697); OFF resynchronizes it 33 times from 2381.05 to
2628.55. All three HQs remain living command buildings rather than death rows.

The exact summary lines are OFF 906681/911284/913017 and ON
837282/841820/843361 (capture, callback, damage respectively). Final HQ rows
are OFF 916408/916409/916647 and ON 846710/846711/846949.

## Death and objective review

The first 275 ordered death identity/time/killer triples match exactly; every
death before the first false neutralization at 1702.90 matches. The first
different death is OFF's `Storage - Raktar03 02` at 1790.72, attributed to HQ2
(341310). ON that same object dies at 1838.77 to NH (333092). Thus the first
observed death divergence occurs after the first ownership difference and
directly involves HQ2. This is consistent with a downstream targeting change;
it is not a proof of every subsequent target-selection decision.

The net victim-set difference accounts for the four fewer ON deaths:

- Six OFF deaths are `Airfield1_sqn45`, `Airfield1_sqn46` and their two wingmen
  each. Those six names do not appear anywhere in ON's spawn or other logs.
  OFF's first squadron-45 spawn is line 507900; its six deaths span
  525457..555790. These are absent creations, not surviving zero-health planes.
- ON adds Cargo1/Cargo2 deaths at 2783.04/2816.12, both attributed to TroopTrans3
  with nonzero hit damage (588372/596697). Their final health is zero
  (847043/847044), while both are alive at 2400 health OFF (916741/916742).

Of the full death identity/time/killer triples, 120 OFF and 116 ON differ after
the shared prefix. The changed ownership is consistent with the later combat
and spawn differences, but no full gameplay-identity claim is made. Airfield1
itself dies at 2461.88 OFF versus 2461.53 ON, both to NH; HQ2's destruction of
`Hangar, Small, 02 04` moves from 2360.83 to 2333.10. These observations do not
by themselves establish every link in the missing-squadron spawn chain.

Both runs complete objectives Bruh, Block and Troop and retain MissionPhase 2,
without a mission success/failure marker. Their 12 objective binding events are
identical. The score display correctly retains two captured bases ON versus
one OFF (837164 / 906563). CB2 stays neutral: f12 does not complete the third
capture, so this pair does not establish full campaign completion.

## Renderer qualification

Both runs return **exit code 1**. A reached Present with a failed HRESULT is
counted as neither presented nor skipped (`src/game_hosts.cpp:1007`), and the
check in `src/game_main.cpp:700` requires their sum to reach the outer frame
limit. The loop and simulation completed; no accounting was changed.

| Variant | Failed native Present frame(s) | Presented / skipped | Renderer loss summary |
| --- | --- | --- | --- |
| OFF | 12983, `0x88760868` (115248/115249) | 12981 / 67218; one missing | 231 polls, 221 holds, 1 reset, no create/reset failures |
| ON | 42444 and 68889, same HRESULT (416553/416554, 724849/724850) | 80071 / 127; two missing | 122 polls, 112 holds, 3 resets, no create/reset failures |

The renderer summaries are OFF 935606 and ON 865624. Outer-loop completion is
OFF 933688 / ON 863706; motion completion is OFF 917079 / ON 847372.
`D3D_DEVICE_LOST.md` section 4 records an earlier short forced-loss simulation
pair with no gameplay differences. That is supporting historical context, not
a clean pass for this pair. The present result supports the health/capture
mechanism while keeping rendering and exit qualification explicitly failed.

Primary review accepted the bounded simulation evidence with these renderer
qualifications. No further runtime was requested or launched.

## Artifacts

Worker root: `J:/PROG/battlestations-pacific-decompile-cc11_health_guard/`.

- `local/cc11_health_pair_off/` and `local/cc11_health_pair_on/`:
  `.pair_export.json`, `pair_export_build.log`, and `build/win32/Release/bsp_game.exe`.
- `local/cc11_health_pair_run.ps1`: existing `tools/run_game.ps1` launches with
  distinct `local/cc11_health_pair_{off,on}_usnos_f12.log` paths.
- `local/cc11_health_pair_orders.txt`: pinned f12 orders.
- `local/cc11_health_pair_receipts.json`: commit, switches and SHA-256 receipts.
- `local/cc11_health_pair_analysis.json`: event/summary comparison with log lines.

## Retained limits

The accepted finite-request behavior is the native equality/refusal gate at the
five existing health-zero endpoints, not a complete SetHealth reconstruction.
The host still coalesces health-zero callbacks at hit/pass end. It does not bind
positive-health callbacks, native per-write callback timing or native ownership
and multiplayer behavior. Unordered equality eligibility is preserved, while
the pre-existing NaN storage and signaling-exception limits remain as documented
in the guard packet. This completed USNOS pair does not establish those missing
native contracts.
