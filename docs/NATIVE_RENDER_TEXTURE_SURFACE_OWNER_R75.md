# Native render texture and surface owner

Addresses: 00b3d640, 00b4e020, 00b4e140, 00b4e410

## Result and boundaries

R75 implements the complete 18h D61EB8 texture/surface owner constructor,
ordinary destructor and deleting destructor using the R73 texture factory,
R74 level-surface getter and existing registered render-target factory. The
owner supports a texture surface, a separate render target, or a retained
external surface. It serves shadow/resource and other post-effect callers;
the descriptive names do not assert an original class symbol.

The missing texture+54h target is the genuine three-byte `RET 4` at B3D640.
It neither reads its receiver/argument nor initializes pixels. Its implementation
preserves those original bytes and effects. No texture fill or clear is inferred
from the constructor passing a pointer to four zero words.

| Routine | Inclusive range / bytes | Original ABI | Coverage |
| --- | --- | --- | --- |
| B3D640 | B3D640..B3D642 / 3 | Unused ECX and one unused stack word; RET4 | Exact machine leaf, compiled bytes checked |
| B4E020 | B4E020..B4E13A / 283 | ECX actual18h owner; width,height,format,multisample,low mode byte,external surface; EAX owner; RET18h | Complete source control flow and base-only C++ cleanup projection |
| B4E140 | B4E140..B4E1E7 / 168 | ECX owner; RET | Complete source destruction and base-only C++ cleanup projection |
| B4E410 | B4E410..B4E42D / 30 | ECX owner; flags stack word; EAX original pointer; RET4 | Complete source scalar deletion, including post-free tail |

New source contexts do not claim the original constructor/destructor ABI or
native FH3/SEH. This packet supplies dependencies; the complete resource-service
initializer and application/gameplay integration remain open.

## Layout, inputs and construction

The B107F0 caller allocates 18h at B109BC/B109C2 and supplies all six constructor
arguments at B109E5..B109EE. The constructor writes profile CEB130 and actual
intrusive count+4=1, reads low mode byte, captures format then height, stores
mode at +14, reads initial width, installs D61EB8 and clears +8/+C/+10. Bytes
+15..17 remain untouched.

| Offset | Producer-established value |
| --- | --- |
| +0 | D61EB8 profile; base destructor later stamps CEB130 |
| +4 | Actual intrusive owner count |
| +8 | Creator-owned runtime 2D texture |
| +C | Retained level surface, separately created target, or retained external surface |
| +10 | Retained texture level surface only when current mode byte is nonzero |
| +14 | Original low mode byte |

Construction reloads the actual F8D394 renderer publication and calls its
current D5F0A8+88 -> B2A070 with width,height,one mip,format,flags10h. It captures
current mode before publishing returned texture+8, then resolves the texture's
current D61948+30 -> B3FD80 and requests level0 with the unused argument zero.

Mode zero publishes the level owner at +C. Any nonzero mode publishes it at
+10. A nonnull external input is compared with captured old+C; if different,
it is published before retaining it through fresh CE221C. A nonnull old owner
is then decremented through fresh CE2220 and dispatched at zero. With a null
external input, current multisample and width are read again, the renderer
publication is reloaded, and full B2A7C0 creates a separate registered target
using the captured height/format. Its creator reference becomes +C.

Finally the routine reloads current texture+8, writes the four zero words and
dispatches current D61948+54 -> B3D640. That recovered operation is empty.
Source contexts share the actual renderer publication, pools, strings,
support manager, counters, synchronization and immutable native table views.
Admitted resource profiles are D61948 textures and D619A0 surfaces. The source
does not invent a provider for arbitrary native profiles.

## Lifetime and unwind

The ordinary destructor stamps D61EB8, captures +C and then the CE2220 target.
It uses that same captured target while visiting current +C, +10, +8 in order.
Each nonnull slot is cleared only after its decrement and possible zero
dispatch finish. Current virtual0 must be the real BD30E0 invoker, which reloads
the current deleting slot: B3F5B0 for the actual surface or B3F590 for the
actual texture. Those existing complete lifetimes release COM, cache ownership,
registration, strings and their canonical pool slots.

Normal exit disarms cleanup before full BD30F0 stamps CEB130. The deleting
destructor then calls actual allocation free iff flags bit0 is set and returns
the original address, including the freed-address return path.

Constructor handler CBFAB8 selects FuncInfo DF862C/map DF8624; destructor
handler CBFAD8 selects FuncInfo DF8658/map DF8650. Each has one state 0 -> -1,
through CBFAB0 or CBFAD0 respectively, both loading saved owner then tail-jumping
to BD30F0. Source RAII preserves base-only cleanup and initial C++ search.
Neither native map retries field releases nor rolls back acquired texture or
surface references. Persistent construction frames retain nested acquisition
evidence after failure. Native failure/unwind execution remains unproved.

## Saved Ghidra changes and evidence

B3D640 was missing as a function. Its exact three bytes were matched to the
original PE, explicitly defined without flow discovery, and saved. B4E410 had
a CALL_RETURN override on `_free`; clearing only that call-site override and
disassembling B4E425..B4E427 restored `ADD ESP,4` before `MOV EAX,ESI` and RET4.
The library function's no-return annotation was not changed. The definition
and flow-repair records are included alongside the main report.

All four packet bodies total 484 bytes. The probe additionally executes the
existing complete 14-byte BD30E0 invoker. Live Ghidra/PE comparisons cover 921
bytes including those bodies, both unwind/map sets, current profiles and the
18h caller allocation/argument setup. Descriptive names and evidence are saved
through the locked annotation tool, preserving prior comments.

## Verification

Strict MSVC Win32 `/MD /W4 /WX /fp:strict` build and all three existing CTests
pass. One ignored focused probe uses real Direct3D9 and runs original/source
pairs for the following modes:

| Mode byte | Primary/alternate construction | Terminal tested |
| --- | --- | --- |
| 0 | Texture level0 at+C, +10 null | Deleting destructor with flags1 |
| 1 | Separate registered render target at+C, texture level0 at+10 | Deleting destructor with flags0 |
| 7 | Retained external target at+C, texture level0 at+10 | Ordinary destructor |

All six lanes pass. High mode-word bits are nonzero and ignored; +15..17 retain
their explicit CCh preimage. Whole18h holder bytes match after normalizing only
the three resource identities. Resource profiles, dimensions, actual counts,
one texture cache reference, registered arrays, texture/surface tracking and
8192-byte factory accounting are checked. External caller ownership survives
holder destruction, then its independent release drains the final reference.
Ordinary/scalar0 storage has base profile and cleared payloads; scalar1 returns
the original pointer after freeing it. Raw singleton shutdown clears the two
actual pool/support registrations. Final device/API references are zero.

The source B3D640 entry begins with the exact original `C2 04 00` bytes. The
original holder constructor/destructor/scalar bodies and BD30E0 execute copied
native bytes. Actual native profile values remain unchanged; isolated child
code bands map admitted original virtual target addresses to x86 ABI adapters
for the full existing texture factory/getter and concrete terminal providers.
The no-op and BD30E0 entries contain their genuine original bytes. COM calls
reach real interfaces and atomics resolve real Win32 exports. Original holder
EH targets are unreached fail-fast traps. The source and original therefore
share full dependency implementations; this is not an original whole-game run.

The renderer remains an explicit zeroed 1D94h fixture, with actual device and
empty resource containers. Canonical strings, static texture/surface pools,
scalar domains and raw support manager are genuine existing providers. Native
renderer construction, reset/retry, application initialization and gameplay are
not exercised. No new repository tests were added.

Two initial probe attempts stopped before holder execution because a private
heap reservation occupied B20000. A bounded VirtualQuery recorded allocation
A40000/error487. Reserving the three required code bands in the controlled,
suspended child before heap initialization fixed the fixture; only those
parent-owned reservations are committed and released. Historical logs remain
in the sealed local evidence. Production code did not change for this issue.

Failure paths, native FH3/SEH, old-primary replacement induced by reentrancy,
IAT/publication changes during calls, nonzero multisample, arbitrary resource
profiles, full resource-service construction and gameplay remain untested.

## Next work

The B4E020 owner now has concrete texture, level-surface, optional render-target
and lifetime dependencies. The resource-service initializer B107F0 still needs
its complete construction graph and subordinate providers before application
binding. Its field producer map from R73 is a locator for bounded follow-up;
do not replace missing producers or failure cleanup with empty objects.
