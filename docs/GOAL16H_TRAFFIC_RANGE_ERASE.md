# GOAL16H whole traffic range erase

The complete two Native erase bodies are implemented as chosen MSVC Win32 interfaces over actual raw header, argument, output and saved-register cells. Original classes/templates/declarations and semantic owner/metadata types are not recovered.

| Entry | Coverage | Native / public Source | Physical interface |
| --- | --- | --- | --- |
| 0048E400 | complete | 68B / 28 instructions | ECX header, five actual stack DWORDs, EAX late out, RET14h |
| 00489110 | complete | 90B / 37 instructions | ECX header, five actual stack DWORDs, EAX late out, RET14h |

Source-only unused EDX binds fastcall's stack shape; Original consumes no incoming EDX. The actual stack words are out, first metadata, first position, last metadata, last position. All158 public bytes match142 fixed Native bytes plus four true E8 operands16B. Public symbols start at0 in their complete sections. The entire new object is346 executable bytes:68+90 public bodies,94 real SDK bridge and94 SDK COMDAT. This accounts all four executable sections, rather than treating158B as the whole module.

## Physical schedule and aliases

Let S be entryESP. First PUSH EBX(POD18)/EBP(DWORD) preserves a real cell; load first metadata fromS+8 into that register and TEST it BEFORE pushing ESI/EDI. MOV EDI,ECX captures the receiver and leaves TEST flags intact. Only when captured first metadata is nonzero is CURRENT S+10h compared against it. Otherwise call the real diagnostic. Neither metadata word is compared to the receiver. No header/output null guard, bounds/order/alignment repair or narrowed storage domain is added.

AFTER a returning diagnostic, load first from actualS+Ch intoESI and last fromS+14h. Equal positions skip every header+8 read/store and every copy, but still produce output. Unequal positions read current header+8 and take the complete specialization below. After child/newend publication, load CURRENT actual out fromS+4 intoEAX, POP EDI, writeout+4=capturedfirst, POP ESI, writeout+0=cachedmetadata, restore EBX/EBP, RET14h. The output-store order and interleaved POPs remain literal. Header, data, output, actual arguments and saved-register cells may overlap; no local snapshot or output object is introduced. Output stores can themselves overwrite header-end publication or future popped cells.

## POD18 specialization and real x87 dependency

Unequal positions push destination(parentfirst), sourceend(currentheaderend), sourcefirst(parentlast), call public cdecl00486850, ADD ESP,0Ch and publish actualchildEAX to header+8. There is no suffix-positive/divisibility or destination-null check and no memmove substitution.

The complete existing dispatch00486850 is86B/35instructions. It reserves two UNINITIALIZED local DWORDs, clears ONLY each low byte and forwards full private-stack preimages. It pushes SIX actual words {first,last,destination,duplicateDestination,tag0,tag1} to public0047FC30, then ADD ESP,18h. The child77B/30instructions consumes only the firstthree. Equal endpoints skip pointed access; otherwise all six words of each24B record are individually FLD/FSTP32 in exact read/store/cursor/compare order before endpoint equality ends the loop. Forward overlap, stack/value aliases, DWORDwrap, malformed-range faults/nontermination and inherited FP environment remain instruction-driven. No buffered six-word snapshot, integer/SSE copy, whole-zero tag, fake callback or three-word replacement of the six-word child call is supplied.

Dispatch computes returned end from captured destination plus wrapped24 times the Native signed /24 magic result for wrapped(last-first):2AAAAAAB signed product, high-half SAR2, sign correction, LEA multiply3/8. This need not be an assumed incidental copy cursor. Parent publishes actualEAX even when the child skips an equal source range. No FP control/status/MXCSR/stack reset or NaN policy is installed. Literal x87 conversion/status/fault effects are retained statically; POD18 parent/FP runtime is unexecuted here, with no invented results for NaNs, denormals, alternate modes or lower-stack states.

All six existing POD18 sections total390B/157instructions and match their complete Native counterparts except two real CALL operands8B:382 fixed bytes. Needed dispatch/kernel total163B/65; other227B/92 are fully retained/accounted existing functions, not newly implemented candidates.

## DWORD specialization and real SDK policy

For unequal positions, compute wrapped currentend-last then arithmetic SAR2. TEST signedcount; LEA bytes=count*4 retains its flags. PUSH EBX, capture wrapped newend=first+bytes inEBX, JLE skips copy for count<=0. Negative count still publishes that wrapped end; unequal count0 publishesfirst. Only equal first/last avoids publication.

Positive count pushes bytes,last,bytes,first, calls a real local cdecl SDK forwarder with {destination,capacity,source,count}, ADD ESP,10h. Ignore integer error return and publish capturedEBX after the call even on an SDK error return. Read actualCURRENT out only afterward. Existing P's SDK bridge is internal and is not called across translation units.

Actual selected SDK corecrt_memcpy_s.h:69..87 defines the four-argument memmove_s API: count0 returns0; validate destination/source/capacity, set actualcurrenterrno and invoke actualreturning _invalid_parameter_noinfo on errors, otherwise actualmemmove. The94B local bridge and94B SDK COMDAT have five genuine relocations each and complete code equal to current P's SDK implementation. Every selected IAT operand is checked against actual PE imports; memmove's E8 target is a real FF25 import thunk to VCRUNTIME140.dll. errno, noinfo and thread-local-handler APIs are actual current UCRT imports. No redeclaration, fake service, provider event or class/global invention is used.

Native00BF67A7's indexed Host abstraction is NOT a physical provider. Its whole91B/43instructions and actual private errno/noinfo/memmove callsites are retained only as Native evidence. Original BF66EF/BFFB8B/BF87E0 private CRT, handler/debugger/Watson, FH3/RTTI, fault/private-frame and unwind identity remain separate from selected current SDK policy.

## Actual calls and callers

| Actual Native CALL | Actual Native target | Genuine chosen Source binding |
| --- | --- | --- |
| 0048E413 | 00BF6713 | `?invoke_native_invalid_parameter_00bf6713@bsp@@YAXXZ` |
| 0048E42A | 00486850 | `?copy_native_traffic_pod18_dispatch_00486850@bsp@@YAPAXPBX0PAX@Z` |
| 00489123 | 00BF6713 | `?invoke_native_invalid_parameter_00bf6713@bsp@@YAXXZ` |
| 0048914F | 00BF67A7 | `?move_traffic_erase_words@?A0xf5f448ad@bsp@@YAHPAXIPBXI@Z` |

The complete existing public diagnostic is17B/9instructions versus Native16B/9: XOR, five PUSH0 padding words, actualSDK zero-argument returning _invalid_parameter_noinfo, ADD ESP,14h, RET. Five physical words are not five API arguments; no noreturn/noexcept annotation or exception conversion is supplied.

Actual caller00495BA0 has216B/91instructions and its only H call is00495C6C to0048E400. It captures OLDendEDI before diagnostics, reloads begin later, writes begin into incoming by-value secondwordR+Ch, and passes out=&actual first by-value wordR+8. The H output therefore overwrites two actual incoming value words. CallerRET1Ch consumes count plus six by-value POD18 words. Its growth target4932E0 (694B/261 overtwo typedranges) remains OPEN on47EA00/4902B0 and legacy EH qualification; T's analogous addresses are different bindings.

Actual caller00495C80 has179B/78instructions and its only H call is00495D24 to00489110. It reserves UNINITIALIZED8B temporaryR-8/R-4, captures OLDendEBP before diagnostics, seeds only secondtempword with currentbegin, then passes out=R-8. HRET14h is followed by restores/ADD ESP8/RET8. Its growth target4935B0 remains OPEN on47EA60/486900/486920/48D690/48E4D0/490320; X493120/R's copy/fill children are different entries. Neither full parent is completed by H. LegacyLeft8bHost abstract resize methods, larger physical parents, mutable globals/layout/header producers and game adoption remain OPEN.

## Complete object/archive/linked evidence

Strict C++17 `/MD /O2 /Gy /Oy- /W4 /WX /fp:strict /EHsc` passes first attempt and emits entire346B, identical in executable bytes and relocations to the actual registered object. Three old supporting objects685B (POD18 390, diagnostic17, P278 comparison) remain whole-object byte-identical to their pre-H actual objects. Only POD18 and diagnostic are direct external H modules; P is a baseline SDK comparison, not an invented H edge.

Four entire actual Core archive member payloads match the complete objects uniquely. Both MS archive linker dictionaries independently bind ten direct public symbols to the exact expected member offsets. The linked fixture selects three H sections252B, two POD18 sections163B and diagnostic17B plus actual fixture helpers; all12 selected code contributions1471B and all45 selected code relocations equal actual PE bytes/operands. Five complete DATA sections331B and two BSS sections12B also match with zero data relocations. All16 unselected code sections721B, all non-code metadata/data/symbol/aux/string/relocation inputs and the unused SDK COMDAT are retained/accounted. No claim that unselected code executed. All158 owned Native bytes and all provider/caller bodies are freshly target-verified live=installed:14 complete functions1176B. Ghidra names/prototypes/comments saved; no author Ghidra writes or Main merge.

## Build, exactly one focused case and correction

One normal warm `./scripts/build.ps1` invocation passes in17.8806058s (2026-10-16T07:08:14.8863133Z..2026-10-16T07:08:32.7696921Z); existing reconstructed_math0.21s and tool_tests7.35s pass. Native reference seeds are not enabled in this worker; Root's three-check build is separate. No registered tests or broad suite added.

One concrete DWORD signed-negative/header-output-argument alias case uses actual Core providers and a real SDK thread-local returning handler, installed/restored through the real API. LetQ be ESP before the five pushes, S=Q-24. Header=S-4; header.end=S+4 IS the actual out-argument cell. Cached metadata starts0. Handler observes real preimages out11223344/firstmeta0/first11111111/secondmeta13579BDF/last22222222, then writes actual metadata cellsAABBCCDD, first=S+8 and last11223348. Currentend11223344-last yieldsSAR-1/bytesFFFFFFFC; capturednewend=first-4=S+4. Publishing rewrites actualoutarg to its own address, which the late load returns inEAX. Output+4 writes first=S+8 to actual firstmetadata argument; output+0 writes cached0 to actualoutarg/header.end. Final header.end is therefore0 AFTER the output alias, not the intermediate publishedpointer. Both Source and full capturedNative90 (only two CALL operands rebound to the actual public diagnostic/internal SDK bridge targets read from Source E8s) show EAX=Q-20, first=Q-16, ECXFFFFFFFC, EDX11223348, ESPdelta0/RET14h, flags286. POD18's real code/children are statically linked by a nonexecuting address anchor only.

The first fixture attempt failed because its observer PUSHFD overwrote retired lastargumentQ-4 before recording it. Full first probe Source/object/EXE/map/output/provider evidence is retained. Moving only MOV observations before PUSHFD fixes this unchanged one case, which then passes. There are two probe compile/run attempts, one Source strict compile and one registered build; no Source change/build rerun. No fake provider/handler-event replacement. POD18/x87, equal/zero suffix, positive/error SDK copies, other malformed/FP/fault/EH paths and actual whole callers remain unexecuted. Current C++ exception propagation through either naked H entry is unproved.

Even with exact owned instruction bytes, Source code/return/fault addresses, private child/SDK frames and saved x87FIP follow Source placement. Actual stack/header/data aliases may observe those differences; full Native ABI, private CRT/EH and game identity are not established.

## Preservation and clocks

Warm base a123c8557d373cf743312a8d876de04821dc0723 retained; before reuse all391 F receipt artifacts/all395 F files and six actual prior build inputs were copied and verified. F/Z originals remain unchanged. H readiness79-artifact corpus is frozen with HANDOFF dbd81739e6919664b5e8a68f4e6182ef432cc7f1c4f35f7866ca7b095b397395, readiness08b9d5bb0ad57f175b5d4d26c9fb93d68d53c11f008ea919e8cf88b6d0e00807 and receipt75786a1c5d5d55fdcb5059e069aa1a160abd22c1ec1aa4785c030b45ff9f0151.

Independent readiness sealed interval is06:34:55..06:47:54.880554Z =779.880554s; Root reports792s including its terminal overhead. These are recorded separately from implementation, which starts07:02:09Z beforebrief/capture and ends in the final ignored receipt after commit/release/sealing, including all setup, one observer rejection/correction and checks. Readiness overlapped prior F work; intervals are not summed into serial hourly throughput. Final HANDOFF/receipt carry exact pin/full hash manifest and inclusive implementation wall time.

Only two owned addresses, four new Source/header/doc/report paths, additive48-band name/reconstruction records and one CMake append are changed: seven tracked files. Existing names/tags/comments preserved; Root owns independent review, annotation and integration.
