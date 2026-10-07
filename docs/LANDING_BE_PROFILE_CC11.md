# Landing BE profile and deferred receipt (CC11)

Packet: `cc11_land_be_profile`. Status: **conditional source reconstruction, unbound**.
Target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 little-endian,
base00400000; verified by the `bsp.py` client before each live batch. Worker analysis was
read-only. The primary defined/saved/refreshed the previously missing profile entries.
Descriptive names are hypotheses; the existing scalar deleting destructor name is retained.

This packet adds a real BE source profile, its extended-header codec and the bounded receive
factory fragment. The BE receipt projection now calls the required reindex provider after
rotation. It does not connect the retired-task hook, delivery queue or GameUnitsHost.

| Native range, end exclusive | Coverage | Original ABI | Source |
| --- | --- | --- | --- |
| 0075AD60..0075AD88 | complete | ECX=message, RET | zero constructor |
| 0075ADA0..0075ADBD | complete | ECX=message, stack DWORD type, RET4 | predicate |
| 0075ADC0..0075ADDF | complete | ECX=message, stack flags, RET4 | scalar deleting destructor |
| 007EF6F0..007EF710 | complete | ECX=message, stack cursor, RET4 | writer |
| 007EF710..007EF731 | complete | ECX=message, stack read wrapper, RET4 | reader |
| 007ED580..007ED58B | complete, existing provider reused | ECX=squadron; MOV ECX,[ECX+348h], tail JMP0071BE40 | actual singleton-token getter |
| 00769AAB..00769ACB; 0076A27B..0076A298 | partial00768530: BE allocation/constructor arm and common reader join only | parent ECX=read wrapper, RET; private SEH outside projection | BE factory fragment |
| 007ED610..007ED64F | complete conditional control flow | ECX=squadron, stack index, RET4 | rotation helper plus required reindex call |
| 007ED260..007ED375 | complete native body reviewed; existing value algorithm reused | ECX=squadron, RET | required live-field publication contract |
| 007F0077..007F008C inside007F0030..007F01E6 | partial: BE arm only | ECX=squadron, stack message, RET4; AL=1 | BE receipt projection |

Other dispatcher arms (4B,4C,7F,80,BC and default), other factory selectors and selector
peek/rewind/private exception handling are outside the source projections. The source factory
caller has already selected BE and rewound the type byte. It is not the complete00768530.

## Profile and wire layout

The five DWORDs at00D02E5C are0075ADC0,007EF6F0,007EF710,0075ADA0,004499C0.
The source table uses x86 fastcall bridges for thiscall-shaped slots. All five methods have
recovered behavior; no context/default adapter is appended. The public storage has Win32 size20h:
base00h..17h, sender WORD18h, relay BYTE1Ah, retained padding1Bh, signed member index DWORD1Ch.
Base padding11h..13h is also retained. Predicate acceptance is exactly BEh or46h; acceptance of
46h does not establish a46h payload or receipt semantic.

0075AD60 writes +08h/+0Ch/+14h/type+10h/sender+18h to zero, delivery+04h=1,
relay+1Ah=0, the BE profile, then index+1Ch=0, retaining padding. This is the receive constructor.
It neither invokes0075B430 nor captures a current game/selected owner. Emission at007EEEDE
uses0075B430 with typeBEh, captures its real selected owner and overwrites the derived fields;
the earlier hook's required route must still supply that different construction contract.

0075ADC0 stamps root profile00CE4974, frees only when flags bit0 is set, and returns the same
pointer. It has no payload teardown. The source reuses the existing root-delete/free provider.

| Native site | Callee | Recovered contract |
| --- | --- | --- |
| 007EF6F9 | 0075B480 | write type8, low sender12, relay1 |
| 007EF706 | 00429090 | write low3 bits of index DWORD |
| 007EF719 | 0075B4C0 | read the same extended header into message offsets |
| 007EF727 | 00428D30 | read/sign-extend3 bits into +1Ch, ECX=wrapper+4h |
| 0075ADD1 | 00BF65AC (`_free`) | scalar-delete flag1 frees stamped object |
| 007F007B | 007ED610 | BE receipt loads +1Ch; AL=1 at007F0083 even if index rejected |
| 007ED645 | 007ED260 | reindex after prefix rotation, never on rejected index |
| 007ED586 | 0071BE40 (tail JMP) | mode1 reads +54h, mode2 reads +188h, other modes return0 |

The native signed reader is deliberate here: three-bit wire values4..7 become -4..-1. Thus
source index4 traversing this particular codec becomes -4 and is rejected on receipt. The
writer/reader listing and00428D30 body establish that transformation; this is not a claim about
observed gameplay or an authorization to change the width/reader. Positive indices1..3 round-trip.

## Bounded factory and deferred delivery

Factory switch entry0076A58C contains00769AAB for(BEh-1). The arm allocates20h at00769AAD,
calls0075AD60 at00769ABF and joins0076A27B. The join invokes profile+8 at0076A283 with
the same wrapper, then returns the message. The source fragment uses the existing throwing
allocation provider and actually invokes the source profile's reader slot.

The current Ghidra parent has a noncontiguous body:0076A283 belongs to00768530, but00769ABF
has no containing function. The arm's live bytes and disk disassembly were reviewed without
repairing that body. These two arm calls therefore remain raw-fragment receipts; the automatic
call verifier cannot establish their containing function. Actual failed allocation/CRT handler
identity and private SEH/fault behavior remain outside the source domain.

The existing route is still required:007EEE50 calls0077C2A0 with sender=squadron, flags5,
status=null; readiness game+5D4h gates routing;0077C44D calls0076E520. The latter serializes,
rebuilds at0076E5A4, copies the DISTINCT sender object's WORD+174h into copy+18h at0076E5B9,
copies original selected-owner+14h at0076E5CE, then enqueues for later receive dispatch.
The new factory returns owner=null until that real copy step. Neither this packet nor its probe
implements the route, readiness, sender resolution, enqueue or drain timing.

## Receipt and formation contract

007ED610 rejects index<=0 or index>=live_count without mutation. Otherwise it moves that
member to slot0, shifts the preceding prefix right and then calls007ED260. The older typed
helper covers rotation only; the new wrapper calls the required provider immediately afterward.
BE receipt acknowledges even a rejected index. The caller supplies an already selected BE
message at actual deferred receipt; other message types are excluded, not generic-dispatched.

007ED260's whole listing confirms a five-entry table initially zeroed. In the first walk it
writes each current member's plane+9D8h=array slot, then marks its OLD plane+9D0h with1.
The second walk sets leader+9D0h=0 and table0=-1. Later slots scan odd candidates from1 and
even candidates from2, skipping entries already assigned(-1). When odd=even-1 and the old
formation index is positive even, odd advances by2, preserving the side. The smaller candidate
wins and is marked-1. Native scans/table indexing are unchecked; fault/invalid-index domains
are excluded. The existing `plane_formation_assign_indices_007ed260` adds count/index/scan
guards, so its value agreement is confined to valid authored formation data.

PlaneSquadronEntity lacks live +9D0h/+9D8h field bindings. The required pure-virtual provider
must publish the first-pass +9D8h writes before second-pass +9D0h writes in current member
order. Detached arrays alone cannot supply those native publications or timing. There is no
default, no-op provider, synchronous request adapter or GameUnitsHost implementation.

## Validation and unfinished binding

MSVC19.51.36244.0 x86 (toolset14.51.36231), `/std:c++17 /MD /O2 /DNDEBUG`,
compiled both changed .cpp files successfully.
One ignored source fixture was compiled/linked against those objects and the existing main
Win32 bsp_core.lib with `/MANIFEST:EMBED`. It passed the actual five-slot source profile,
zero constructor/padding, BE/46 predicate, bytes `BE BC 3A` for24 bits, virtual factory reader,
deferred receipt then rotation/reindex, positive-even preservation and wire4 -> -4 rejection.
Its plane fields/publication provider are fixture storage. This is **source fixture evidence**,
not original ABI, native runtime, queue, network, lifetime or gameplay validation.
The existing live call verifier passed8 direct/tail rows; one virtual reader row is explicitly
indirect and the two raw factory-arm calls retain the membership limitation above.
`git diff --check` passed. The primary owns serialized full CMake/MSVC integration.

The old task's cached squadron at task+404h remains different from the plane's CURRENT +9D4h.
Native00999EE0 calls the old task+58 hook before its scalar destructor; retirement0099A4C0
does not drain immediately. The hook/request must retain that old identity while its queued
copy is later delivered. This packet neither creates task ownership nor proves death observer
invalidation. Direct dequeue at Boolean clears or synchronous rotation would change the schedule.

Next binding requires concrete retired-task ownership/FIFO and native drain schedule, valid
cached-squadron/plane lifetime through hook and delivery, real emission base/selected-owner
context, readiness and sender/owner copy, and a live reindex provider. A bounded routing packet
can recover0076E520's queue allocation/tail and its receive consumer before connecting this
profile/factory to a host. A primary analysis-repair packet can add only the reviewed BE switch
arm to00768530's body and revalidate raw call membership. Original destructor/observer death
ordering, allocator identity, exceptions/faults, full ABI and game behavior remain unproved.
