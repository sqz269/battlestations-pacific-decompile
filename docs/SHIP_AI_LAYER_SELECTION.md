# Ship AI navigation layer selection

`ship_ai_select_navigation_layer_009eca20` reconstructs the complete normal
control/store/call schedule of 009ECA20 through explicit borrowed navigation
state and actual external services. It produces the travel layer at nav+30Ch;
the planner's existing layer is a consumer, not this field's producer.
Class+570h/nav+168h is a different scalar and cannot substitute for it.

Names are hypotheses. These are new C++ interfaces, not binary or native
exception ABI replacements. Win32/CTest and a bounded original-byte fixture
passed; this packet does not claim a mission or gameplay run.

| Entry | Inclusive body | Original input / cleanup | Coverage |
| --- | --- | --- | --- |
| 009ECA20 | 009ECA20..009ED3D9 | ECX=nav, seconds; RET4 at009ED3D7 | Complete through required host |
| 006DFD80 | 006DFD80..006DFD8C | ECX=unit, EAX dword; RET | Complete class560 getter |
| 00852FD0 | 00852FD0..00852FE3 | ECX=submarine, EAX dword; RET | Complete depth-indexed getter |
| 00412170 | 00412170..0041217F | ECX=manager, EAX key; RET | Complete first key |
| 00412180 | 00412180..00412191 | ECX=manager, EAX key; RET | Complete last key |
| 004121B0 | 004121B0..004121E7 | ECX=manager, signed key; RET4 | Complete next key or last |
| 00417D60 | 00417D60..00417DAA | ECX=manager, signed key; RET4 | Complete previous key or first |
| 004121A0 | 004121A0..004121A2 | ignored receiver/key; RET4 | Complete actual no-op |
| 00417B10 | 00417B10..00417BF9 | ECX=group; out,point,push,byte; EAX=out; RET10h | Complete ordered offset walk |

Live flow inspection found **641 instructions / zero gaps** in 009ECA20 and
**86 instructions / zero gaps** in 00417B10. The getters were initially missing
Ghidra functions: 006DFD80 is 13 bytes, last instruction RET at006DFD8C length1;
00852FD0 is 20 bytes, last instruction RET at00852FE3 length1. The integrator
defined their exact bodies under the write lock and returned ownership before
this packet's annotation proposals. All nine owned bodies now have function
definitions. No Ghidra writes were performed here. The external LandingShip
virtual+10Ch target 0042BB40..0042BB42 (XOR AL,AL; RET, final instruction length1)
is reported separately for integrator definition; it is outside this packet's
ownership and remains an explicit dispatch service.

## Producer chain and storage

Controller009F50E0 calls pre-step009E0270 at009F5156, this function at009F51C6,
then controls009ED6B0 at009F5209. The nav pointer is brain+60h, kept in EDI.
009ED3E0 passes nav+30Ch to planner009E3780 at009ED523,009ED588,009ED5FD,
009ED692. The associated owner+9C8 argument is pushed but not read by009E3780;
it does not supply a missing layer.

Eight live base/surface-ship vtables resolve virtual+214h to006DFD80, whose
two MOVs return `[[unit+538h]+560h]`. Submarine vtable00D0BF80+214h instead
contains00852FD0, which reads `[unit+1268h]` then
`[class+560h+4*depthLevel]`. The already reconstructed `ShipLeafTuning.array`
owns those four values; `scalar` owns class570h. Surface leaves replicate the
second Lua integer into four slots. Submarine uses entries2..5. Existing
`kShipLeafTuningSources` and `kSubmarineTuningArraySources` establish this map.

Installed Single navigation values are11 for surface ships except TBoat and
both LandingShip variants5. Multi surface values are11. Submarine arrays are
Single[11,26,46,86], Multi[11,46,46,86]. These are source observations, not
constants introduced into this implementation. `SUBMARINE_MODEL.md` establishes
the actual depthLevel producer and command; an unavailable level is not zero.

The borrowed view binds existing fields instead of creating another nav block.
Their initializer chain is009E43A4 (nav+1C4),009DFCD4 (+60),009DFCF3→009DBDF0
with base=nav+224. Stores009DBE50/+E4,009DBE4A/+E8,009DBE56/+EC and009DBE28/+F0
initialize nav308,30C,310,314 to zero. 009DBE6E/+FC and009DBE76/+F8 initialize
nav320 and31C from00D217E8, bits4E6E6B28 (**1e9f**). This differs from the
**1e10f** reset at009ECEAA/B2, source00CE4970 bits501502F9.

009E4330 initializes flag160/value164/timer170 at009E43AA/43B0/4391. It first
zeros timer148 then replaces it at009E4669 with a negated00BD2F10 draw using
**stream0** (XOR ECX,ECX009E4644). This function's three reseeds use **stream1**.
The goal setter009DE050 can reset314; bind its existing `crossing_314` field.
Escape direction150/154 is written when flag160 is raised; the native false
flag path does not initialize or consume that direction. Hull174..188 is the
existing009DE2F0 output, and goal1DC/1E0 is the existing009DE050 output.

## Full selection schedule

An absent owner returns before every other input. Otherwise00778890 selects
the owner virtual214 or0070E450 group maximum; store308 occurs at009ECA6E.
The latter aggregates actual controller members that satisfy virtual5C(6).
Kind5C(0Ch) and virtual10Ch gate floor16C: true/true stores0, otherwise the
actual class560 word is copied. Actual LandingShip table00CFFA30+10Ch points
to0042BB40, whose bytes32 C0 C3 return AL=0; the interface retains real dispatch.

Two separate manager lookups supply first and last keys, then signed compares
clamp308. Timers148,170,314 are decremented in that store order. An expired148
reseeds from settings1FC/200, scales its period by speed/reference through the
existing00419010, and probes along the bow or stern from the actual hull pose.
When its squared movement exceeds1,164 is clamped and walked down through
containing layers or up through clear layers. The upward loop's initial308
bound is **not retested** after each advance. A changed164 forces travel refresh.

Otherwise an expired314 handles modes2/3: reseed from settings1F4/1F8, compare
goal movement, walk310 downward through containing groups, then apply floor16C.
The changed flag is computed **before** that floor, so a floor-only change does
not force refresh. Other modes copy164 into310 and reset cached goal coordinates.
If no change forced refresh,170 must be ordered-negative; zero and unordered
return. Settings204/208 then reseed170, clear160, and copy308 into30C.

The travel pass can lower30C to the goal layer when near enough, or step down
after an offset goal falls within the lookahead distance. It then walks upward
from164 against the actual hull pose. A containing group raises160, derives
distance14C and direction150/154, and applies the native limit3C8 rule. The final
30C is the signed minimum of164 and the requested layer's corresponding group
ceiling, with the next key used when the selected group lies strictly below it.
Every004121A0 call is retained as an actual no-op after its manager lookup.

## Dependencies, arithmetic, and boundaries

All singleton accesses occur separately at the native sites. In particular,
the first settings view remains borrowed across the second lookup, and every
live layer word is loaded **after** its manager lookup. Explicit helper sequencing
avoids C++'s unspecified function-argument evaluation order. The report records
every native call address, target and containing function, including all ten
00417B10 caller sites (seven outside this body) and their argument instructions.

The small manager leaves reuse `AvoidZoneTable` in existing slot order. Empty
tables return0 for first/last/next/previous. Group selection and containment
reuse004120D0/004178F0. 00417B10 copies the input through x87, then supplies each
zone with the **updated** point from the previous zone. It preserves the optional
half-open bounds/containment gate and unchanged push argument, reusing the
existing00416B50/00416F30 composition. Native and semantic groups must match;
`AvoidZoneClearanceGroupView` supplies the actual native pointer order.

Arithmetic retains x87 operation/spill boundaries, native x87 comparisons and
the two SSE timer comparisons. The interpolation endpoints come from verified
image bytes: timer(0.1,2,0.6,1), forward(0.1,0,0.5,1.5), reverse(0.1,0,0.4,1).
Ratio magnitude uses the original sign-bit mask. RNG00BD2E60 and reference
speed0080FC30 float-spill before returning, so their host results are floats.
00414C60 remains a required length callback with its actual cutoff/CRT contract;
no new CRT or RNG implementation is introduced.

The C++ boundary rejects a missing group at a native dereference, mismatched
native/semantic storage, invalid submarine index, and the existing offset
primitive's undefined no-candidate case. It invents no clear result, group,
loop cap, depth, timer, or random state. Valid ordered native storage, callback
lifetimes, coherent geometry, and progress of the final upward walk are caller
preconditions. Concurrent storage mutation, unmasked hardware-fault sequencing,
native EH ABI and arbitrary invalid-object reads are not claimed.

## Verification

`./scripts/build.ps1` passed Win32 and the two existing CTest checks after seed
verification. No tracked tests were added. One ignored `/MANIFEST:EMBED` probe
maps original executable bytes without modifying the installation, relocates
13 decoded absolute memory operands in the executed bodies, and supplies
explicit fixture inputs at external dependency boundaries.

The fixture executes original009ECA20, both214 getters, the five small manager
leaves,00417B10,004178F0,004120D0 and00419010. It compares every mutable field at
each host callback and final output against this implementation: **3,072 cases,
six x87 control words, zero mismatches**, observing all40 external callback sites.
Another **68 cases** in that same executable check signed/empty manager edges
and ordered two-zone offsets, including in-place output, containment bytes0/1/80h,
and positive/zero/negative push. Geometry primitives, length, group aggregate,
RNG and unit/settings services are explicit fixture boundaries, not independent
validations of those systems. Numeric controls are varied with masked exceptions;
this does not establish native fault/exception sequencing or gameplay parity.
The report lists exact fixture sources, input generation, original-body hashes,
relocation offsets, compile command, build log and call verification.
