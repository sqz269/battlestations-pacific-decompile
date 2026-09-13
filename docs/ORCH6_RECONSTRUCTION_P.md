# Orchestrator 6 reconstruction batch P

Addresses:009F1420 (post-goal fragment),00415620,00811D80,00953A80,00694200,00694280,00694EA0,00BD0400.

The three worker packets and the shared raw observer-lifetime adapter are integrated. Detailed ABI, call-site, producer and uncertainty evidence remains in their individual reports. Descriptive names are hypotheses, not recovered symbols.

| Packet | Recovered behavior | Focused proof |
| --- | --- | --- |
| [Brain prepass](SHIP_AI_BRAIN_PREPASS_SCHEDULE.md) | Both timers and complete normal torpedo/list6/special-chain schedule under the captured lock | 18 native/source pairs,1558 bytes,225 events per side,zero mismatches |
| Throttle clamp (`reports/ship_ai_throttle_clamp.json`) | Selected unordered clamp branches preserve native behavior | 18 native/source pairs at PC24/PC53; old source fails12 |
| [Dummy binding](UNIT_NEIGHBOUR_FIELDS.md) | Actual DummyObjectID producer, first matching dummy, HUD transfer and fresh mode-dependent kill | 12 whole-body native/source cases |
| [Raw observer lifetime](OBSERVER_RAW_LIFETIME.md) | Actual singleton-manager publication, tracked lock and CF7E70 deletion | Source lifecycle over real raw manager, edge refcounts and locks |

Review corrected the prepass API labels: world+21C/220 is torpedo list slot43. Dummy objects use list45 at234/238. This naming-only correction does not change the native order or vtable slots; the combined build includes it, while the retained worker fixture belongs to its original commit/interface. The prior throttle document now points to its correction.

MSVC Win32 Release and both existing CTests passed at `0f074d19734406a92937ee7a06a6db9c1f8b4ab2`. The preserved executable SHA256 is `8a1f1496371d35e0c6ed69176e39debfabfc797a29d0bb0950610c72a7cec998`. A120-frame USN01 process run completed successfully:18,557 finite trajectory rows,241 unchanged Airfield2 samples,2,400 avoidance queries,1,080 cruise reads,10,080 generic ticks over42 units, and420 world registration nodes with no invalid lists or unavailable owners in the reported paths. `reports/orch6_reconstruction_p.json` carries the exact command, source hashes and counters.

The mission run is a regression check for that build. It does not establish execution of the new prepass, dummy binding or observer adapter, nor original-game visual/gameplay parity. The native fixtures use the external services and limits documented by each packet. The raw observer check is a source lifecycle fixture, not differential execution of the original adapter. New C++ APIs are not binary replacements.

The local archive contains 245 verified ignored worker artifacts, including the recorded exact build libraries. Its map is `local/orch6_worker_archive_p.json`. The source and result hashes remain in individual reports. False-free instructions have been decoded, but several Ghidra stored body ranges still omit those bytes; the flow-repair reports explicitly distinguish these two facts. The disabled script endpoint was not reconfigured.

## Follow-up packets

Q recovers the actual dispatch singleton owner and CRT vector alias, complete raw90h obstacle-node lifecycle, and canonical scene flags in existing unit storage. Event production/delivery and candidate runtime integration still require actual owners and complete call contracts.
