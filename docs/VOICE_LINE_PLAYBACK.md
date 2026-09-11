# Voice line playback (bounded native sequence)

Packet: `voice_line_playback`; read-only Ghidra work on the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Verified wrapper queries
and exports were used. No names, comments, prototypes, function boundaries or
flow overrides were changed in Ghidra. Repository names below are provisional.

## Corrections to the earlier mission-events interpretation

* **Only slot zero is scanned.** At 005B724B..005B7254 the index increments
  from zero, compares with **1**, and loops on signed less-than. The 18h stride
  does not prove a second slot exists. 005BAE3B..005BAE44 repeats this exact
  limit. The only slot this packet establishes is manager+8..+1F.
* **Readiness polling has effects.** 007027B0 can stop a sound, release its
  reference, reset an auxiliary object, and call a sound-manager flag setter.
  Consequently 005B71D0 must poll again for each input clip; the prior reduction
  to an immutable busy-slot count is not equivalent under callback reentry.
* **The positional clip is passed by value.** 005BBDC0 receives three clip
  words and an entity pointer, returning with RET10. Its second stack word is
  a record pointer; that record's +8 signed ID gates playback.
* **Attenuation is an admission test only.** The float store at 005BBE9A
  overwrites the entity stack argument, while 005BBEC4 constructs a vector from
  the original three clip words. 005BBEDF passes vector, zero, zero to
  005BBC10. It does not apply attenuation as volume or alter clip+8.
* The five entity virtual +5C probes use raw kinds 8, 1A, 1B, 19 and 6.
  Previous prose saying four probes omitted one of those calls.

## ABI, implementation and evidence

| Address | Native ABI and inclusive final instruction | Reconstructed scope |
| --- | --- | --- |
| 005BBC10 | ECX manager; vector*, target dword, speaker*; 005BBDB6 RET0C (3 bytes) | Category selection, notification gate, 38h allocation request, bank argument reference, construction and queue append, including null allocation appended as null |
| 005BBDC0 | ECX manager; Clip12 by value, entity*; 005BBF05 RET10 (3) | Signed ID gate, transform-service ordering, float separation stores, attenuation branches, one-clip non-positional call |
| 005B71D0 | ECX manager; vector*; 005B7283 RET4 (3) | Panel+34/+24 and manager+74 gates, repeated slot-zero polling, manager+6C final read |
| 005BABB0 | ECX fresh 38h line; vector*, target dword, intrusive-reference value; 005BAF1F RET0C (3), EAX=this | Clip copy, target validation, text joining, first nonnegative sound ID, slot-zero probe, playback host call, logging and argument reference release |
| 007027B0 | ECX slot; 0070289F RET (1) | Native state-1 and state-2 polling, timeout, sound/auxiliary release sequence and final field reload |
| 005B9760 | ECX manager; target float, duration float, NativeString*; 005B97FB RET0C (3) | Snapshot current sound level, initialize fade fields/callback, immediate 005B8C30(minimum) host call |
| 005B7790 | ECX queue at manager+54; nullable line*; 005B77E4 RET4 (3) | Allocate 0Ch doubly linked node, append at tail, update first/last/count |

The implementation is in `include/bsp/voice_playback.hpp` and
`src/voice_playback.cpp`. These are typed C++ projections, not compatible
native memory layouts or binary entry points. Existing `NativeString` and its
allocator contract are reused. Native vector operations are represented by
`std::vector<VoiceClip>`; this does not claim allocator, iterator-debugging,
invalid-range termination or exception-ABI equivalence.

## Recovered records and ownership

| Native location | Established contents |
| --- | --- |
| Clip12 +0/+4/+8 | vtable 00CF0DD0, borrowed record pointer, copied opaque dword |
| clip record +0/+4/+8 | native string length/buffer, signed sound ID |
| slot +0/+4/+8/+14 | state, intrusive sound reference, intrusive auxiliary reference, start timestamp |
| manager +54/+58/+5C | line-list count, first node, last node |
| manager +6C/+74 | final readiness blocker / early disabled gate |
| manager +A0 | pointer to category-indexed intrusive-reference values |
| line +4 | copied vector header; input is still used for subsequent scans |
| line +14/+18/+1C | subtitle widget, chosen slot index, first playable clip index |
| line +20/+24/+28 | subtitle layout fields (zeroed when joined text empty) |
| line +2C/+34 | validated target dword / shortcut widget set by GUI helper |
| manager +D4/+D8/+DC/+E0 | fade current, target, reciprocal duration, callback string |

005BBC10 categorizes a null speaker as zero but chooses a **null** bank in that
case, not table[0]. With a speaker, a side mismatch chooses 4 unless local side
equals 2. Matching/permitted sides probe kind 8 for category 1; then 1A, 1B,
19 for category 2; then 6 for category 3; otherwise category 0. Evaluation
short-circuits in that order. Table values are proven reference-counted by
InterlockedIncrement at object+4 before constructor invocation. Their concrete
classes, table initialization and semantic category names remain unknown.

005BABB0 joins record strings with the literal `|` at 00CF100C, adding the
separator whenever the accumulated string is nonempty. Empty trailing input
therefore still adds a separator. It scans IDs independently of text. The
first ID >=0 is played even if the slot probe produces -1; there is no -1 guard
before 005B9050. Its by-value Clip12 gets vtable 00CF0DD0 and preserves the
chosen record and opaque word. The selected bank is retained around that
call, and the constructor consumes its own argument reference on exit.
No-playable-ID leaves native +1C untouched; the C++ field is optional to avoid
claiming a native initialization there.

The queue owns line/node lifetimes; their eventual removal/destructors remain
outside this packet. Allocation services return typed objects. The node
allocator promises a valid fresh node or throws, because the native would
dereference a null allocation. Clip records and category-table handles remain
borrowed; the host must keep them valid through the modeled calls.

## Slot polling and floating point

State 1 captures mission clock 00F876A4 **before** the sound's virtual +0C
completion call. If unfinished, sound+4C is passed to 00A81860 and returned+2C
supplies duration. Native arithmetic keeps `now-start` in x87 but spills
`duration+2.0` to float. Retirement requires strictly greater elapsed time;
the FCOMIP/JBE pair at 00702800..04 keeps equality and unordered values alive.
Completion bypasses the duration query. Retirement then performs:

1. Current sound virtual +8(1), reload current slot+4, intrusive release,
   then clear slot+4 and state.
2. If slot+8 is nonnull, its virtual +8(0), then 0054D510(&slot+8,0).
3. 00A7D120 on current [00F8BBD8], with zero, then reload slot state for return.

State 2 calls 00A77730 on [00F8BBCC]; false clears state. Other states return
unchanged. `VoiceSlotHost` preserves the listed callback order and rereads;
there is no assumed FMOD success or immutable snapshot of the slot.

Positional subtraction is entity minus camera on all three axes, each stored
as float before vector length 0042B2F0. Native attenuation uses x87 distance,
double 2000.0 at 00CF0DD8, and one final float store. FCOMIP(0.25,a)/JA followed
by COMISS(0,a)/JNC rejects ordered values below 0.25 or at/below zero. With
masked exceptions, NaN survives both gates. Finite 1500 distance is accepted;
the range is not a strict less-than 1500 bound. This C++ interface takes
double from the length host and uses double arithmetic before the float store;
it does not claim x87 extended-precision/exception equivalence.

Fade duration compares against double bytes `00 00 00 e0 e2 36 1a 3f`
(00D7A268), which equal the widened float 0.0001f from `17 b7 d1 38`
(00CE3C68), approximately 0.00009999999747378752. Values above that threshold
are used directly; equality, smaller values and unordered values use the
minimum. +DC receives float(1.0/divisor). The immediate update argument is
always that minimum, not the supplied duration.

## Remaining external contracts and analysis gaps

`VoiceLineHost` requires actual speaker queries, 005A2650 notification,
00645160 target validation (ECX=target, DL=1), 005B8510 subtitle creation,
005B9050 sound playback, camera/entity refresh, vector length and 005B8C30
fade update. It also requires actual intrusive lifetime and allocator services.
Their names describe callsites and are not substitute implementations. FMOD,
GUI layout and Lua fade callback execution remain external.

Read-only supporting exports: 005BA4B0 vector assignment, 005BB020 vector
append, 005B8510 GUI text setup, 005B9050 slot playback, 005B8C30 fade update.
These were inspected only to fix contracts; they are not all reconstructed.

No missing function entry was found in the claimed packet. Two existing
function bodies have flow gaps caused by `_free` being treated as nonreturning:

| Function | Inclusive undisassembled bytes | Raw bytes / interpretation |
| --- | --- | --- |
| 005BBDC0 | 005BBEF2..005BBEF4 | `83 C4 04`, ADD ESP,4; final instruction 005BBF05, length 3 |
| 005BA4B0 | 005BA5CC..005BA5CE | `83 C4 04`, ADD ESP,4; final instruction 005BA5FE, length 3; supporting helper, no repair performed |

005BABB0 reports 284 instructions and no gaps. Its pseudocode removes real
text-joining blocks as unreachable; assembly was authoritative. These issues
are reported for the integrator to repair under the Ghidra write lock.

## Validation boundary

The Win32 Release build and existing `reconstructed_math` test passed. A single
local, uncommitted contract probe also passed: timeout retirement replaces the
sound pointer during virtual stop, releases the replacement, resets the
auxiliary, then a flag-setter callback reenters and changes state to 2; readiness
returns false from that final current state. Its trace verifies callback order
and rereads, not native FMOD behavior. Results are recorded in
`reports/voice_line_playback.json`. This packet does not add a permanent test
suite. Native audio/GUI runtime, installed-game playback and binary ABI
compatibility have not been validated.

## Follow-up service integration

`VOICE_SERVICES_INTEGRATION.md` supersedes the earlier unresolved-call scope
above: `005B8510`, `005B9050` and `005B8C30` now have concrete reconstructed
bodies and are called directly. Clip `+8` is the record's resource-array index;
slot `+0C` is an owned string. The corresponding whole-routine host callbacks
were removed. `005B91E0` now reconstructs subsequent-clip advancement.

Constructor `005BABB0` reloads `[[00E198C4]+A4]` separately at `005BAE1C/21`
after subtitle work and `005BAE85/8B` after slot polling and bank retain. Its
former explicit manager argument was removed from the C++ projection. The
start callee consumes its retained bank argument, so no additional caller-side
release guard surrounds it. Build and service-fixture validation passed;
native audio, rendered subtitles and gameplay remain unvalidated.
