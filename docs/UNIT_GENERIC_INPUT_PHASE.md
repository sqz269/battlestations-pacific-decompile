# Generic unit input phase

This packet reconstructs the complete caller body `0095DC40..0095DDF8` in
`unit_generic_input_phase.cpp`, with borrowed unit fields, current role/local
slot cells, clock, global view slot, and the existing input/settings owners.
It also exposes the complete false-role arm for the constructor-unassigned
case. It does not add a unit owner or connect GameUnitsHost.

The native body uses `PlayerArtilleryThrow` settings. The older
`PLANE_FLIGHT.md` / `plane_flight.hpp` description as a turbo double-tap detector
is not supported by the actual settings producer. The proposed function name
`BSP_UnitGameObject_UpdatePlayerArtilleryThrow` remains a descriptive hypothesis.

| Entry | ABI and range | Coverage |
| --- | --- | --- |
| `unit_generic_input_phase_0095dc40` | Native thiscall ECX=unit, one float step; 441 bytes, 0095DC40..0095DDF8; RET 4 at 0095DD6E, 0095DDE0 and 0095DDF6 | Complete control sequence and state arithmetic, including assigned/unassigned branches, through two explicit singleton getter interfaces. New C++ ABI and borrowed field view. |
| `unit_generic_input_unassigned_0095dd71` | Interior branch 0095DD71..0095DD91, then 0095DDF1..0095DDF8 | Complete false-predicate arm only; does not stand in for assigned behavior or claim a separate native function. |
| `00927F30..00927F59` | Native thiscall(unit, role), RET 4 at 00927F52/00927F57 | Existing predicate read in full; its fixed-role-4 operation is inlined into this caller over borrowed canonical cells. No duplicate role owner or generic role-index API. |

## API and ownership

`UnitGenericInputFields` contains references to the same unit's float +63Ch,
float +640h, byte +644h, and float +708h. It constructs nothing and supplies no
defaults. These references must remain live and have the distinct native field
relationships for the whole call. They may refer to established process owner
fields; they are not a declaration of a complete native unit layout.

The common constructor `0095CC90` establishes +63Ch=1.0f at `0095CD9E`, +644h=0
at `0095CF02`, and +708h=-1.0f at `0095CF38`. ESI is the receiver from
`0095CCAC`; EBX is zero from `0095CCE0`. XMM1 is loaded from 1.0f at `0095CD63`
for +63Ch and from -1.0f at `0095CEE8` for +708h. The complete listing contains
no +640h store. The new view deliberately does not manufacture its initial
value. A caller of the assigned branch must already have a valid live value
for each reached field; the role-false arm does not read +640h/+644h/+708h.

`UnitGenericInputContext` borrows current role 4 (+1BCh), the local slot
(game+18ECh), the mission clock cell `00F876A4`, the actual global pointer slot
`00E198C4`, and the two getter providers. The predicate first reads the local
slot, rejects unsigned values above 7, and only then reads and compares role 4.
Current role 4 must come from the canonical current-role array, not the policy
array or inferred selected-unit state. Current role 4 == 8 proves false for
every possible local-slot dword. That proof permits the separate unassigned
entry without a fabricated local slot or other unavailable service.

The unassigned entry is not just the isolated store. Native `0095DD71` loads
1.0f, `0095DD79` writes +63Ch, `0095DD81` reloads 1.0f, and `0095DD89` compares
against that just-written field. `0095DD90 JBE` reaches the epilogue at
`0095DDF1`, with `RET 4` at `0095DDF6`. The helper preserves the store and SSE
comparison. Under the native stable-field domain, equality always takes the
return: there are no remaining singleton/global-pointer reads or field writes.

The assigned path reads the clock before obtaining the input owner. The
existing `NativeInputActionOwnerStorage` represents the actual 24h owner;
its +4 pointer must address a real array containing record 153. The read is
`base + 1CB0h` with 30h stride, byte +28h and float +24h. Existing record/tick
producers establish those as current input state/value; no action name is
invented and no additional count guard is inserted.

After its timestamp/latch operations, the body reloads the actual `00E198C4`
pointer, follows pointer +CCh, and tests its dword +4Ch. This decides which
existing rate field is read. This packet supplies neither that view owner nor
a replacement target flag. The lifetime/availability of those actual owners
remains a runtime integration prerequisite.

## Calls and arithmetic

| Site | Native callee | Contract |
| --- | --- | --- |
| `0095DC4B` | `00927F30` | Unit receiver, role 4, RET 4; unsigned local-slot guard and equality, inlined over borrowed cells |
| `0095DC6F` | `004BEC00` | Cdecl no-argument input-owner singleton getter, plain RET; actual returned owner +4 is read |
| `0095DCE7` | `00424C40` | No-argument settings getter; first +30h threshold |
| `0095DCFE` | `00424C40` | Retain this returned settings object for +30h |
| `0095DD05` | `00424C40` | Retain this return for +34h; then read both objects for the extended-precision sum |
| `0095DD36` | `00424C40` | Read +40h decay rate after the positive +63Ch test |
| `0095DD92` | `00424C40` | Read +38h or +3Ch recovery rate after the less-than-one +63Ch test |

The singleton bodies and existing owner APIs were reviewed. Providers must
preserve their real getter behavior, including repeated calls and publication
effects. Returned settings must remain valid across subsequent calls; the
first of the two consecutive returns is not snapshotted before the second.

A positive active input renews +640h to the captured clock and sets +644h,
unless a clear latch and the +708h/+640h comparison reject it. Otherwise the
latch clears and a binary32 elapsed value receives clock minus +640h. The
routine then selects recovery, a waiting interval, or decay. Recovery is
limited above by 1; decay below by 0. Positive-input/value and less-than-one
tests preserve COMISS branch predicates, including unordered results. The
+708h comparison preserves the original JC, which treats unordered as the
rejected-input arm. The sum of the first and second time thresholds stays on
the x87 stack until comparison. Multiply/add/subtract and explicit binary32
spills preserve the original instruction order and current x87 precision.

## Settings producer dependency

The existing `GameplayTuningSettings` layout is reused, including its legacy
member identifiers containing `time`. +30h/+34h store
`PlayerArtilleryThrow.AfterShot_FireTime` and `AfterShot_WaitTime` directly.
The three fields at +38h/+3Ch/+40h store reciprocal rates from
`ThrowIncrementTime_HasTarget`, `ThrowIncrementTime_NoTarget`, and
`ThrowDecrementTime`. Native rate stores are at `0083DA64`, `0083DAD9`, and
`0083DB4E`: each duration is clamped against the widened 0.1f floor, spilled,
then inverted with x87 `FDIVRP`. Selected unordered inputs follow the native
comparison, not a new finiteness rule.

At this worktree's base, `src/gameplay_settings.cpp:55` through :57 stored raw
durations. The independent `throw-rates-m` packet owns that producer correction.
This consumer requires the actual stored rates and does not invert them again.
The ignored fixture supplies explicit native-format rate values; it does not
claim the old loader produced them. The unassigned arm never reads settings.

## Evidence

`local/make_generic_input_probe.py` verified original bytes against the current
Ghidra project and original executable, then decoded every instruction and
recorded each relocation. Executable SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Body SHA256:
`34a055cb0d48d6bb4dc4e14ff6b5afff4cf6dbc46577807247e9f140dfed7e05`.
Role predicate SHA256:
`aee2fd9534cd033d743d8d2a8f6faa578cdeec24e27de15f1430c04828ce743c`.

One ignored probe executes the full 441-byte body plus the real 42-byte role
predicate, and compares it to the actual new source compiled with MSVC Win32,
`/O2 /fp:strict /W4 /WX` and an embedded manifest. Sixteen scenarios at x87
precision 24/53/64 give 48 paired cases, 89,088 compared unit bytes, and 165
getter events per side: zero mismatches, matching full x87 status words, and
matching ESP across the original RET 4 call. It covers both role arms, input
latch gates, waiting/recovery/decay, clamps, selected quiet/signaling NaNs, and
the retained settings returns with a third-getter mutation of the earlier
object. A clock mutation in the input getter verifies capture before that call.

Only the two singleton getters are fixture seams. Their fixture backing and
the game/view global projections are explicit input storage, not constructed
native owners; no input polling, Lua loading, view production or full game
execution is claimed. All unit bytes are compared with no masks or pointer
normalization. Getter events include the three writable fields at call time.
The fixture does not compare complete global/owner/settings buffers, MXCSR
status, original trap/SEH behavior, all possible aliases or arbitrary floating
point environments. Callbacks may mutate the borrowed live fields under the
normal synchronous schedule; concurrent mutation remains outside the contract.

The Win32 build and both existing CTests are recorded in the report; no tracked
tests were added. `local/generic_input_artifact_manifest.json` hashes source,
fixture, generated byte/relocation evidence, executables and logs. Ghidra was
read-only; no missing function or false-free flow repair was required.
