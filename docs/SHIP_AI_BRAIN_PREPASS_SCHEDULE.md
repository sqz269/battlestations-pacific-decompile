# Ship AI normal post-goal pre-pass

Addresses: 009F1420, 009F158A, 009F163F, 009F1856, 009F1A44, 009F1B57,
009F1B7B, 009F0AD0, 009DB8F0, 00411EE0, 00CB0B70, 00856050, 00856360,
008561F0, 00855F00, 006E2860, 00812090, 00929E50, 009D53B0.

`ship_ai_brain_prepass_after_goal_009f158a` implements the normal schedule after
goal refresh: both timers, the actual optional lock, the torpedo candidate walk,
the existing ship candidate walk, the trailing entity chain, the optional
submarine helper and the existing avoidance-request publication. Names are
hypotheses. No GameHosts binding or missing runtime owner is created.

| Containing routine | Coverage | ABI |
| --- | --- | --- |
| BSP_ShipAi_BrainPrePass009F1420..009F1BBA | Partial: full normal post-goal009F158A..009F1B9F, except interleaved earlier-goal15B1..15BD and SEH bookkeeping | Native ECX brain, stack float, RET4 at1BB8. Source uses a new typed borrowed view/host ABI. |

All494 instructions and call sites were read; the body has zero flow gaps. The
earlier prefix009F1420..009F1589 and native epilogue1BA0..1BBA are excluded.
The prefix writes AF0 from D7A24C, clears B38 and3AD, then performs the existing
goal refresh. `ship_ai_refresh_goal_vector_009f1420` reuses the actual goal
state/target record and callbacks, but omits AF0/3AD and has its own numerical
projection limits. This module does not silently declare that prefix complete
or duplicate it. A caller must execute those actual preceding operations in
order before entering the new API. The producer tail in
`SHIP_AI_BRAIN_PRODUCERS.md` supplies the timer cells and lock; no defaults,
random draws or constructor calls are added by this schedule.

The first timer uses B48/B44 and places due in BL; the second uses B50/B4C and
places due atESP+17. Both perform the same ordered floating state rule as the
existing N timer helper. The native first sequence retains ST0 step for the
second sequence; source passes the same binary32 step to the reused helper
twice. Ordered step>=old means due and old+(period-step); unordered follows
old-step. There is one period addition, no catch-up loop. The native block at
15B1..15BD belongs to the earlier goal timer; it is unreachable from158A.

| Normal order | Gate and actual work |
| --- | --- |
| 158A..15FD | Update both timers before any lock or scan. |
| 15FE..1628 | If neither is due, jump directly to1B57. Otherwise capture brain+4 once; if nonnull, EnterCriticalSection then increment captured+18. |
| 162C..1855 | Only torpedo due: scan world list21C/220 in captured count/order. |
| 1856..1A43 | Only ship due: call the recovered N list6 walk with the real inherited host. |
| 1A44..1B3E | Only ship due: walk the live F89AD8 chain, actual pose refresh and two settings getter calls per node. |
| 1B3F..1B56 | Decrement/Leave the captured section, even if brain+4 changed during callbacks. |
| 1B57..1B70 | Reload AC4; if nonnull, call009DB8F0 with that helper and seconds. |
| 1B71..1B9F | Fresh settings return; read its actual byte4; publish existing avoidance-request rule. |

The section remains held across all scan callbacks and the trailing chain.
`ScanLock` borrows the real `TrackedCriticalSection`; it owns no allocation.
Its depth arithmetic uses the unsigned representation of the native dword.
The optional null section causes no OS calls. All original list nodes, units,
class/config records, returned velocity buffers and lock storage must remain
alive through their native uses. Pure host lookups cannot allocate, refresh,
default, mutate, snapshot lists or convert unsupported identities into null.

The native unwind map atDE5E10, referenced by function infoDE5E28 and
handlerCB0B78, maps state0 toCB0B70. That funclet passes the captured guard at
EBP-70 to00411EE0, whose body loads guard+4, decrements section+18 and calls
LeaveCriticalSection if nonnull. State0 is installed after Enter/increment;
normal flow marks it inactive before decrement/Leave. The source releases the
same captured section during ordinary C++ scan exceptions. This maps the cleanup
contract; it does not reproduce FH3 dispatch, hardware faults, asynchronous SEH
or native unwind frame/register ABI.

The torpedo list is producer-backed. MTorpedo constructor008560A2 installs
00D0C378 at entity+310, the **shot interface**, while008560AC installs its tick
table00D0C35C at+3E4. Primary table00D0C3E8 has registrar00856360 at+130;
0085638D passes the actual entity to00484540 on world+21C. That existing0Ch
push-back node contains the direct identity at+8 and live next at+4. This
registration appends in order; no wrapped payload or index-to-owner conversion
is inferred.

At scan entry capture initial self, refresh it if C8 is zero, then reload the
current self for full hull length9C8 and navigator level390. The fixed X/Z
origin remains from the initially captured self after its refresh. The hull
radius is binary32(length * doubleCEFF98), then squared/spilled; CEFF98 is the
double representation of0.6f. NavigatorBotConfig already owns the6 parameter
rows at globalF8A688, header0C and stride24. Producer009D53B0 / existing
robot_config maps TorpedoPredict[2] at14 and TorpedoObservation[2] at20.
The horizon is the extended sum of those two live fields plus the binary32
spill of (periodB44 + doubleD7A2B0), finally spilled to binary32. D7A2B0 is3.
Borrow actual parameter/literal owners; this source supplies no fallback level.

Head220 and signed count21C are captured once. A nonpositive count skips the
walk after the preceding scalar work. Each positive iteration reads node+8,
then calls shot+38 and, if nonzero, shot+2C. It next requires byte5D zero and
pointer4F8 different from the current self. The byte/pointer remain opaque fields;
the torpedo constructor writes4F8 zero at008560D1. No additional activity,
null, party, class, altitude or ownership filter is inserted.

The actual MTorpedo slot38 body008561F0 returns shot+148, i.e. entity458;
the bomb constructor initially clears458. Slot2C body00855F00 can refresh the
entity pose, returns2 when twice shot+16C is strictly above world Y, otherwise
returns the nonzero test of shot+44. It is an effectful query, not a cached
boolean. These concrete bodies establish the contract; runtime must preserve
the actual dispatch of every admitted identity.

After candidate refresh, snapshot candidate X/Z with x87 binary32 spills and
capture the current self for primary slot34. Compute fixed-origin minus
candidate deltas, call self slot34 first and candidate slot34 second. Preserve
the returned self pointer until after the second call; then read relative
velocity = candidate minus self in X/Z. Projectile006E2860 copies318/31C/320
to the supplied buffer and returns it with RET4. Ship table examplesCFC3D0
andD0BF80 select00812090, which multiplies the local forward axis94/98/9C by
the actual body-axis speed0092D730 and returns the supplied buffer, RET4.
No generic body velocity substitute is used here.

The existing exact00414C60 helper computes relative planar speed. Multiply its
returned binary32 by the horizon, spill, square and spill. The planar squared
distance also uses the native extended sum and binary32 spill. Admit through
009F0AD0 if distance squared is **strictly less** than hull radius squared; else
require strict dynamic reach and a strictly positive planar dot product of
origin-minus-candidate with relative velocity. Equality and unordered comparisons
reject their corresponding gates. The native x87 stack and spill schedule is
transcribed in the private geometry helper. Remaining count is decremented and
the current node's next is reloaded after callbacks, including the last iteration.

009F0AD0 is a real external nav-owner service, not a notification: its whole
listing/decompile was read. Nav+400 is the signed count and+404 the128-pointer
array; duplicate candidate identity at node+48 refreshes node+14 lifetime from
settings1F0 plus a constant. Otherwise it makes actual stream1 draws, uses
navigator/unit data and allocates68h before009EACA0 construction and append.
This packet does not fabricate that storage or implement its callee algorithm.
Both native call sites1808/1839 pass the unchanged candidate with ECX=brain+8
and RET4. Existing N admission009F0D20 remains independently reconstructed.

F89AD8 is the actual head of the tickable-entity chain: constructor00929E50
prepends self at00929FA9..00929FCE, setting next340 and previous344. The trailing
pass captures head once. Its inlined dirty-pose sequence is instruction-equivalent
to existing00414DB0: refresh/reload parent, optionally multiply local74 by parent
worldCC, x87-copy all16 cells to worldCC, setC8 and clear10C. Source reuses that
implementation. Every node then makes two real settings getter calls even though
the values are unused, and reloads its next340 after those calls. These calls
and mutation points are preserved; no new collection owner is introduced.

The whole009DB8F0 body was read. AC4 is the constructor's4-byte unit holder.
The helper checks submarine mode1268, world Y against depth1204 +/- a constant,
role1B0 and participant/controller state, and conditionally writes122C while
preserving value2. It consumes a float argument with RET4 but never reads it.
Those actual owners and effectful callees remain outside this caller packet.
The final settings byte4 has real producers0083BCD5 and008D0852; because
GameplayTuningSettings leaves it in a gap, an explicit pure lookup resolves the
byte from the captured returned owner. A zero-valued gap is not used as input.

The ignored fixture executes the original1558-byte post-goal slice through a
synthetic entry, with all18 direct calls, four virtual calls and two OS imports
retained. The unreachable earlier-goal exit is redirected to a fail-fast trap.
It shares recovered length/matrix/timer/candidate/pose operations where source
calls them; pose notifications, dispatch results, both admissions, settings and
the submarine helper use explicit observation providers. Sparse fixture tables
contain only the called slots and are not claimed to be complete native vtables.
Actual Win32 sections exercise capture, depth, recursion and release.
No native constructor/setup, whole goal prefix, admission internals, game run or
independent CRT/OS fidelity is claimed. Exact results and all compared/excluded
state are recorded in the report and local artifact manifest.

Validation passed18 original/source normal pairs at x87 precision24/53/64,
225 events per side and zero mismatches, including full x87 status. A separate
source exception at the shot query released the captured section. Win32 Release
and both existing CTests passed; the call gate verified18 direct call rows.
The four virtual calls and two Win32 imports were checked from their actual
assembly/table/producer evidence and are listed separately in the report.

Read-only skill-row follow-up found three exact missing bodies; root subsequently
defined/saved/exported them and the final live prototypes were rechecked.006D1EB0
loads the embedded subobject vtable from unit+38C, loads slot0, adjusts ECX to
unit+38C and tail-jumps (16 bytes).006D1EC0 reads unit+390 (7 bytes), and0047F2F0
is RET4 (3 bytes), used by MTorpedo primary slot128. Their final names remain
FUN; this packet neither implements nor renames them. The raw390 input remains
borrowed, consistent with existing `kUnitOffSkillIndex`; no skill owner is added.
