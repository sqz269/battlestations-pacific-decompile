# Gunnery health callback guard (cc11_health_guard)

Addresses: `00877B90`, `00879070`, `008777D0`.

The gunnery host discarded `UnitHealthWrite::dispatch_health_changed` and called
its health-zero/death funnel whenever the final health was nonpositive. A living
CommandBuilding at zero health can consequently be neutralized again after
SetParty. This is the defect routed by `SHIP_AI_OPEN_ITEMS.md` 205.2 and 206.

`kUnitHealthCallbackGuardBound` is **OFF** pending the paired runtime check.
When enabled, the five existing health-driven death endpoints require at least
one setter result with `dispatch_health_changed`. The damage rule, storage,
attribution, hit notices, counters for applied damage and other switches retain
their existing policies. The new summary records `no_dispatch` candidates and
actually `suppressed` callbacks; OFF can count candidates but suppresses none.

## Native evidence

Read-only `bsp.py` queries/export verified project `bsp`, configured project file
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language and image
base against `config/target.json`. Exports were refreshed without Ghidra writes.

| Routine | Evidence | Coverage in this packet |
| --- | --- | --- |
| `00877B90` SetHealth | `00877BAB..00877BC1` compares existing `+370h` against requested health; `00877BC7..00877C04` clamps/stores; `00877C2A..00877C40` excludes mode 2 and dispatches slot `1B0h` | Partial host binding: transport the existing pure rule's callback decision only |
| `00879070` ApplyDamage | `008790B4..008790CA` rejects positive damage to nonpositive health; `008790D0..00879121` handles the invincibility floor; `00879123..0087914B` subtracts and calls SetHealth | Partial host binding: refused calls produce no callback decision; damage arithmetic/policy retained |
| `008777D0` ApplyHitRecord | Hull positive-damage test `008778AD..008778B7`, slot `1ACh` call `008778D2`; part winner positive test `00877A11..00877A1E`, slot `1ACh` call `00877A37` | Partial host binding: accumulate setter callback decisions across the existing hull/part host paths |

The call at `00877C40` is indirect; `00877C3A` loads the slot, not the call itself.
Live containing-function checks put the two AddDamage sites in `008777D0`, the
SetHealth call at `0087914B` in `00879070`, and callback `00877C40` in `00877B90`.
The three native entry points take ECX `this` and one stack argument, returning
with `RET 4` (`00877CC6`, `0087923C`, `00877A41`); the portable C++ is not an ABI
replacement. Native virtual AddDamage implementations remain outside this packet.

`FUCOMIP` at `00877BB9`, `LAHF`, `TEST AH,44h` and `JNP` at `00877BC1` distinguish:

| Comparison | ZF/PF before TEST | Mask bits | Early return |
| --- | --- | --- | --- |
| Ordered equal, including signed zero | 1/0 | `40h`, odd parity | Yes |
| Ordered unequal | 0/0 | `00h`, even parity | No |
| Unordered | 1/1 | `44h`, even parity | No, with FP exceptions masked |

The existing pure rule's `current_health == requested` implements this equality
decision. The comparison precedes clamping: current 0/request -1 legitimately
dispatches even though the stored result remains 0. Conversely, a small nonzero
damage can round to the same requested health, and positive damage at the
invincibility floor can be refused before the setter. Neither nominal damage
nor `before - after` is the callback contract.

## Audited host endpoints

| Endpoint | Decision lifetime |
| --- | --- |
| `Impl::apply_hit` | `ShipHitBinding` ORs the setter flag across hull/part `add_damage` calls |
| `Impl::apply_gunless_blast_hit` | Same binding, with no gun attribution |
| `Impl::run_damage_control`, pending explosions | One decision per queued explosion |
| `Impl::run_damage_control`, repair/water/fire | OR across the existing unit pass's damage calls |
| `GameGunneryHost::apply_script_damage_0095da00` | One decision per script damage call |

All `set_health_00877b90` and `kill_unit` sites in this file were inspected.
OverrideHP writes through the setter but currently has no health-zero callback;
this packet does not add one. `FailureBinding::set_health` remains an explicitly
unreached branch. Explicit kill/destroy APIs bypass this guard. Per-part segment
health is separate from unit `+370h` and does not fabricate a setter decision.

## Validation and limits

- `git diff --check`: passed.
- `verify_report_calls.py`: one direct call checked, zero failures. Three
  indirect calls were skipped by the tool and checked against the listing and
  live containing-function bounds.
- MSVC x86 `/std:c++17 /W4 /WX /fp:strict /Zs` on the modified gunnery source:
  passed. This is syntax/type verification, not a linked game build.
- Ignored focused probe `local/cc11_health_guard_probe.cpp`, built together with
  existing `src/unit_damage.cpp` using the same flags and `/MANIFEST:EMBED`:
  **12/12** checks passed. It covers ordered/signed-zero equality, changed lethal
  and same-storage clamping, unordered inputs, dead/floor refusal, rounded-away
  damage, and multiplayer-client callback exclusion. Run with
  `cmd /c local\cc11_health_guard_check.cmd`. This checks existing rule outputs,
  not execution of the changed host endpoints or the original executable.
- No new tracked tests, shared CMake edits, ledger changes or Ghidra mutations.
- Full Win32 build/CTest and runtime OFF/ON pairs remain for serialized primary
  integration. No native differential, game validation or ABI parity claimed.

This remains a partial binding: callbacks are coalesced at the existing hit/pass
end and only the health-zero path is represented. Positive-health callbacks,
native per-write timing and native multiplayer ownership are not added. Quiet
NaN **callback eligibility** follows the native equality test, but the existing
pure setter stores 0 for a NaN request; native `JBE` at `00877BD3`/`00877BED`
preserves the unordered request when exceptions are masked. That storage
difference and signaling-NaN exception behavior are not repaired here.

## Runtime handoff

Existing artifacts are in
`J:/PROG/battlestations-pacific-decompile-cc9-ships41/local/`.
`s41_h12_osf12.log` lines 292620/293434 show SetParty HQ2 2 -> 0 followed by
neutralization with prior party 0 at 1706.15 seconds. Its summary at line 846107
has `health_zero=478 neutralized=49`. These are prior-run evidence, not a result
from the new guard.

The existing reproduction is `s41_rows.ps1 -Rows osf12`: USNOS, 80,000 mission
frames at 0.05 seconds, 80,200 outer frames, press-start frame 30, helm orders
`s41_os_f12.txt`, with `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`.
From PowerShell, with the absolute path to each paired executable:

```powershell
& 'J:/PROG/battlestations-pacific-decompile-cc9-ships41/local/s41_rows.ps1' -Rows osf12 -Prefix cc11_health_off -Exe '<absolute OFF bsp_game.exe>'
& 'J:/PROG/battlestations-pacific-decompile-cc9-ships41/local/s41_rows.ps1' -Rows osf12 -Prefix cc11_health_on -Exe '<absolute ON bsp_game.exe>'
```

These existing commands launch hidden processes; await each `.done`/log before
comparing. Keep `kCaptureStatePartyFromUnitBound` ON in both experimental variants
to expose the routed defect, without enabling it in the committed baseline.
HQ2 capture starts near 34,000 frames and HQ1 near 47,600. Compare the complete
80,000-frame pair: no repeated prior-party-0 neutralization, the expected HQ
resynchronizations, guard counters and deaths. The guard remains OFF until the
primary records and reviews those results.
