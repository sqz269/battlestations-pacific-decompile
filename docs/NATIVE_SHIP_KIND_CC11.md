# Nine actual ship type entries

The nine separate naked Win32 entries in `native_ship_kind.cpp` reconstruct
**526 complete bytes / 187 instructions**. Every complete Source COFF body and
linked entry matches its own unchanged Original body; there are zero calls,
globals, bridges or COFF relocations. This is raw predicate closure, not a
generic class resolver or a binding of ship/group construction and lifetimes.

| Actual owning profile | Target / end exclusive | Bytes / instructions | Ordered fixed words, then actual+C4 |
| --- | --- | --- | --- |
| D09678 | 006DFE50 / 006DFE86 | 54 / 19 | 6,5,4,2,1,0 |
| CFC3D0 | 006FE530 / 006FE56B | 59 / 21 | 7,6,5,4,2,1,0 |
| D0BF80 | 00853050 / 0085308B | 59 / 21 | 8,6,5,4,2,1,0 |
| D01630 | 00758510 / 0075854B | 59 / 21 | 9,6,5,4,2,1,0 |
| CFB738 | 006FB3D0 / 006FB40B | 59 / 21 | 10,6,5,4,2,1,0 |
| CFA778 | 006EB230 / 006EB26B | 59 / 21 | 11,6,5,4,2,1,0 |
| CFFA30 | 0074BC60 / 0074BC9B | 59 / 21 | 12,6,5,4,2,1,0 |
| CF90B0 | 006DFE90 / 006DFECB | 59 / 21 | 13,6,5,4,2,1,0 |
| D0C648 | 00857DC0 / 00857DFB | 59 / 21 | 14,6,5,4,2,1,0 |

All ranges are complete. Individual full-byte hashes, disassembly and exact
profile-slot addresses are in `reports/native_ship_kind_cc11.json`. The body
is `MOV EAX,[ESP+4]`, its own fixed compare run, `TEST EAX,EAX`, then exactly
one late `CMP EAX,[ECX+C4]`, with full EAX 0/1 and `RET4` on both exits.
Original ECX is this same actual receiver and the stacked query is a raw DWORD;
the Source fastcall spelling makes unused EDX explicit. No common base body,
two-value projection, census, eager dynamic-field snapshot, default, callback
or virtual dispatch wrapper replaces any most-derived target.

The ordinary caller supplies a stable nonnull actual receiver, a genuinely live
aligned DWORD exactly at `+C4`, and readable backing through `+C8`. Fixed paths
perform no receiver access, but establish no constructor/lifetime admission.
Faults, invalid placement, concurrent mutation and structural reentry are
outside the tested domain without new guards. The register/stack/result entry
protocol is reproduced; original class/table/world/private-EH binding is not.

The nine actual `profile+5C` DATA words were verified individually. The base
constructor `0081ED40` calls its base, stamps `D09678` at `0081ED80`, and writes
DWORD `+C4=6` at `0081F2AA`. The complete `006FE460` producer calls it at
`006FE468`, stamps `CFC3D0` at `006FE46D`, then writes `+C4=7` at `006FE4B3`.
These are structural witnesses only. No constructor/C4 producer is claimed
verified here for the other seven ship types. `+C4` is a class word, not depth;
the fixed ancestry skips 3. Raw profiles remain uncallable numeric DATA.

The real group type probes are `0070DA33` (current speed-record entity),
`0070E522` (leaving entity) and `0070E53B` (selected entity), each with query 6.
Conditional target attribution requires the actual owning profile. No generic
group vcall or class-admission route was installed. Whole `0070EF30` has no type
gate, so exhaustive runtime member classes cannot be inferred from its join.
Speed still needs actual `member+538 -> class+500` on true, or actual
`member+3D0 -> entity+538 -> class+188` on false, with native fresh fields/count
and x87 behavior. Detach still needs wake handoff `00815E20`, actual observer
runtime, record-copy numeric behavior, publication/ownership and class routing.
The complete existing group scalar `0070D260` is available; its actual observer,
CRT and empty-group routing/lifetime admission remains required. Base plane
`0074E400` and squadron `007EFB00` are already separate completed providers;
derived plane profiles and any other admitted classes require their own actual
targets and reached storage. They are not answered by a default or census.

One new ignored connected family passed **608 checks / zero failures**. Four
fresh TUs compile these nine leaves, current complete group storage/lookup, and
the probe. Each Source/Original setup calls the complete group constructor once
using pinned Original `CF4888` bits `4479C000`; Source setup publishes an actual
record identity/count and both real lookups identify that same record/receiver.
SAME refers to identity within each connected pass. Cross-pass group/receiver
addresses were not recorded or proven equal across the separate local lifetimes.
Every separate leaf executes its full fixed list, dynamic match, miss for 3,
and old/new probes after changing the same live C4 to a high-bit word: 98 queries
per backend, 196 observations. Full EAX, callee DWORD consumption, entire group/
receiver backing and guards are checked. All nine Original bodies are unchanged,
unrelocated RX copies. Profile tokens are DATA only; no callable table is built.
Field publication and replacement are Source fixture instrumentation, not an
observed native class constructor, join, reentry or runtime lifetime.

`local/cc11_ship_kind_leaves/build.cmd` is the strict Hostx86/x86 recipe:
`/W4 /WX /fp:strict /O2 /MD /Gy`, four fresh TUs and `/MANIFEST:EMBED`.
The safe `probe.exe` contains an asInvoker manifest. Its seal records 227 equal
pre/post inputs, 214 compiler-observed include paths (three project headers),
three actual tools and all six searched CRT/system libraries. No BSP support
library is consumed. Twenty-four fresh body/profile/producer/constant PE/live
spans matched. The first strict build and sole family execution passed; there
are no failed attempts or old-family replays. No tracked test, Ghidra/CMake/
ledger mutation or full build was made. Root owns integration and full validation.
