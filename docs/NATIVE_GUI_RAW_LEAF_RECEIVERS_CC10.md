# Raw GUI leaf receivers, CC10

Three existing leaf implementations now accept actual raw owner storage without
requiring a legacy host owner or a C++ array/tail overlay. Their typed interfaces
remain available. These are new C++ interfaces; caller admission and ownership
are separate from the native leaf stores.

| Entry | Exclusive end | Bytes | Original contract |
| --- | --- | ---: | --- |
| B7AF20 | B7AF3E | 30 | ECX ambient, stack four-word source, RET4 |
| 4B62E0 | 4B6369 | 137 | ECX light, stack four-word source, RET4 |
| B8E6C0 | B8E6EC | 44 | ECX Group, stack sphere source, RET4 |

The ambient overload writes four forward integer load/store pairs at actual18.
The shared copy helper uses raw instruction accesses, so it imposes no live
`std::array` or scalar-object overlay on the receiver's representation. It
retains forward overlap behavior for existing typed callers as well.

The diffuse overload captures actual1D8 through the original x87 spill, reads
source.x before that spill, and stores base1A4..1B0 in forward order. It then
reloads the original source through the existing four-channel multiply kernel.
The typed and raw overloads share this schedule and arithmetic. No
`NativeLightTailStorage` is fabricated. Native incoming-argument overwrite and
private product-stack aliases remain outside this source interface, as they
were for the existing typed arithmetic implementation. The eventual GUI caller
must prove its argument and scratch relationships fit that domain.

The Group overload directly reuses the existing44B kernel: clear flags138
mask30, read the supplied sphere pointer, clear byte175, then four forward
FLD/FSTP pairs into08..17. It performs no profile lookup, reference operation,
notification or metadata admission. Its caller supplies valid actual Group
storage with a live node prefix. The legacy owner wrapper keeps its existing
identity and phase checks. The GUI radius expression and outer caller's
capture/store order are unchanged and remain a separate acquisition contract.

All211 body bytes match live Ghidra and the installed PE. The leaves contain
no direct or indirect calls and require no flow repair. One ignored focused
executable executes the three complete copied bodies without relocation or
instruction replacement and compares the source sequence over whole98h,
1F0h and18Ch receiver representations. Forward overlap and signaling-NaN
inputs agree, including complete x87 status under the fixture's default masked
control word. This is raw leaf evidence, not construction or admission of a
complete owner graph. Other rounding modes, unmasked faults, private stack
aliases, binary ABI and gameplay are unclaimed.

The strict MSVC Win32 build and all three existing CTests passed. The ignored
probe uses `/MD /O2 /W4 /WX /fp:strict /link /MANIFEST:EMBED` and rejects
`NDEBUG`. An initial probe-only alignment-padding warning was corrected by
explicit padding; the failed compile log remains in the evidence. No tracked
tests were added. `reports/native_gui_raw_leaf_receivers_cc10.json` records the
source hashes, exact native extents and frozen local evidence.
