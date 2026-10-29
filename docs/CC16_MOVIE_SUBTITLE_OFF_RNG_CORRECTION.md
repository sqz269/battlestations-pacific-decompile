# Movie subtitle OFF RNG evidence correction

This additive audit corrects the RNG claims in `1e7078eafa460a5a3f00ff67703f8ab050b33895` and `85d548e2b57540ec6f767ee39e58e989fa4f0e11`. Ten of the thirteen historical OFF pairs contain matching genuine **logged, formatted host RNG samples**. BSM01, USN02 and LOMP06 are **UNOBSERVED**. Their inherited `1` was the `lof_expiry_draw=1` feature flag, so the original mandatory RNG dimension was not established for all thirteen rows. The old combined acceptance claims and corresponding worker summaries must be read with this correction.

The original logs, frozen JSON, evaluator and results remain unchanged. This packet ran no game, probe, build or test and changed no source, trace, plan, environment, bound, mask or comparator. Audit checkout: `83f99c91af748b29a5f5af040ec1604fcb40f6c4`. Runtime evidence remains exact baseline `409689e5e813d4aaaeb656abeb827d06a5842016`, candidate source `f49e064d5e393c5ad8d0dfdd54d686b5669b2585`, clean runtime metadata `063c4be212b2e475a706b65d2f4ca067fe2656f0`; it is not current-main gameplay proof.

## Defect and corrected interpretation

`local/off409_eval.py:21` used `draw=([-0-9.]+)` after `MissionLoad::`; it could select the underscore-suffixed flag. Its `rng streams: on` check establishes configuration only. Frozen `src/game_hosts_gunnery.cpp:20425` prints that flag from `kAaLineOfFireExpiryDrawBound`, not from a random draw.

The audit streamed all 26 complete raw logs. It required `\bdraw=` and a source-provenanced plane or landfort event format, retained all unexpected matches, and found none. It excluded 13 negative plane sentinel lines per side: `src/game_hosts_units.cpp:24460` assigns `-1` instead of calling the mode draw for that condition. The sentinel does not establish that other RNG paths did not execute.

Plane samples are printed to four decimals (`game_hosts_units.cpp:24539`); landfort samples use five decimals and are emitted only while `fire_starts <= 4` after successful chance selection (`game_hosts_ship_ai.cpp:13598,13624–13630`). Under the frozen labelled-stream environment, these are partial host observations. They do not prove the retail global stream, unlogged draws, raw-bit parity or a global-first RNG event.

## Corrected rows

Counts below are per side; each observed row has the same ordered literal genuine event lines on baseline and candidate. “First” means first **logged** genuine sample in that raw log. No masks were added. All mission-start frame observations remain 36.

| Row | Genuine samples | Excluded sentinels | First logged time / value | RNG evidence |
|---|---:|---:|---|---|
| ESMP08 | 308 | 0 | 316.05 / 0.1912 | formatted observations match |
| JM06 | 2 | 0 | 750.89 / 0.8964 | formatted observations match |
| USN21 | 441 | 7 | 489.35 / 0.4819 | formatted observations match |
| IJN18 | 1 | 0 | 67.05 / 0.4640 | formatted observations match |
| BSM01 | 0 | 0 | — | UNOBSERVED |
| USN12 | 82 | 0 | 999.28 / 0.6193 | formatted observations match |
| LOMP10 | 33 | 0 | 73.65 / 0.1119 | formatted observations match |
| BSM09 | 53 | 0 | 137.10 / 0.1431 | formatted observations match |
| PRCP01 | 77 | 6 | 119.35 / 0.9478 | formatted observations match |
| USN01 | 32 | 0 | 34.55 / 0.04169 | formatted observations match |
| USN02 | 0 | 0 | — | UNOBSERVED |
| LOMP06 | 0 | 0 | — | UNOBSERVED |
| USN04 | 51 | 0 | 93.60 / 0.4143 | formatted observations match |

Total: 1,080 genuine formatted samples and 13 excluded sentinels per side. Empty event lists receive no RNG-proof credit. The report retains both raw paths/hashes, first-event lines, source provenance, legacy selections, ordered event hashes and per-run arguments/environment.

## Evidence retained separately

All 755 recorded hash/provenance checks passed, including all 26 raw log hashes, manifests, archived runtime inputs, watchdogs, original acceptance/pair/ordered files, frozen binary/backend hashes, evaluator/controller hashes and candidate source hashes. The four event/counter source files are byte-identical between 409 and f49. This preserves the thirteen historical pair-diff, ordered dialog/message/Lua, teardown, start-frame and callback-absence results independently of the missing RNG dimension. It does not reinstate an all-dimensions 13/13 verdict.

The existing VFS checks also retain their hashes and scope: native provider CLI resolves/opens/fully reports the 97-byte `movies/us_outro.mws`, and rejects the absent path. That is VFS-provider evidence, not MovieBridge callback, Lua schedule, returned-buffer digest or subtitle playback proof.

## Natural witness facts for a separate gate

1. **IJN18:** on both retained logs the first accuracy pre-call marker is line 5490 and the first completed firepower-return marker is line 5511, between mission frame 73 and 74 (after native frame 112). The genuine `Recon2` event at line 14964 follows at `t=67.05 draw=0.4640`, after mission frame 1340/native frame 1379. The original executed scope is the full 64,000-frame mission, 64,300 outer frames, step 0.05, exact `ijn18_l67_i18_d.txt` plan and recorded environment. This is the earliest retained natural candidate; no shortened/current run is credited.

2. **USN21:** the first genuine event is line 87850, `Intrepid_sqn01|.-II t=489.35 draw=0.4819`. First accuracy pre-call / completed return are lines 382054 / 382073 between mission frames 29965 and 29966. The first genuine sample after that return is at `t=1502.39 draw=0.0493`. Final counters on both sides are 8,753,634 accuracy calls, 2,405,880 completed firepower ratings and 44,459 ring bearings. The retained scope is the full 80,000-frame mission; a 12k/600-second prefix would miss the recorded hot epoch.

3. **Unobserved rows:** no new run is required to make this correction truthful. If their RNG dimension remains a required acceptance claim, it needs separate, preregistered natural evidence. USN02 demonstrates why end counters alone are insufficient: 992,016 accuracy calls and 358,140 completed ratings coexist with zero genuine logged RNG samples. Do not retrofit old acceptance by forcing draws or retiming plans.

The frozen source emits the accuracy marker before `weapon_hit_accuracy_008386f0` (`game_hosts_ship_ai.cpp:5036`). The `rate_bearing_0095eb40` marker and `firepower_ratings` increments follow the return from `ship_ai_firepower_rating_0095eb40` (`:5148–5153`); `GameHostLog::implemented` logs the first actual invocation (`game_hosts.cpp:319`). The report provides raw surrounding frame markers rather than inferring a hot epoch from end-summary counts. A new current guard witness needs a fresh exact freeze, fixed budget and genuine post-return sample on each side.

## Scope and credit

`kMovieSubtitlesBound` remains OFF. Genuine decoder/clock, live settings/language admission, Text/localisation/widget ownership, extratitles, Lua library-set and callback/source reachability remain separate activation obligations. This audit adds no native-body, ABI, activation or playback credit. It does not relabel historical receipts as current-main evidence.

Exact data and corrected claim locations: [additive report](../reports/cc16_movie_subtitle_off_rng_correction.json). Local analysis only: `local/audit_subtitle_off_rng.py` and `local/rng_audit/`; all originals remain under the frozen `codex-nongui-backlog/local/off409` directory.
