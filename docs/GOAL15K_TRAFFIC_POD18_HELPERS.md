# Complete raw 18h range and size helpers

Six complete Native entries are reconstructed in
`native_traffic_pod18_helpers.cpp`, totaling **390 bytes, 157 instructions,
9 branches and 36 x87 operations**. Their public naked Source bodies preserve
every instruction and stack-cell access. Only the two CALL operands bind the
real Source child entries. There is no private partial kernel, extra frame,
mutable global, callback, CRT dependency, exception guard or invented default.
No Ghidra/CMake/shared registry was changed by this worker.

The existing library/compiler names remain. Descriptive Source names identify
observed operations and are not recovered template symbols. Earlier typed
implementations and their records remain separate and unchanged.

## Whole bodies and physical interfaces

| Entry and inclusive end | Bytes | Original physical contract / Source binding |
|---|---:|---|
| 0047C4D0..0047C4EE | 31 | ECX actual header; plain RET; EAX raw signed /24 magic result or0. Source one-pointer fastcall. |
| 0047FC30..0047FC7C | 77 | Both verified callers push six DWORDs, caller cleanup18h, plain RET. First/last/destination are read; duplicate destination and two tag words are unread. Source cdecl exposes those six actual physical words. EAX advanced destination. |
| 004841A0..004841E8 | 73 | ECX first/EDX last; stack destination plus three unread actual metadata words; RET10h, EAX advanced destination. Source six-word fastcall binding. |
| 00486BE0..00486C25 | 70 | ECX destination/EDX unsigned count; stack value plus three unread actual metadata words; RET10h. Source six-word fastcall, void semantic return. |
| 0048D2F0..0048D324 | 53 | ECX metadata preimage, three actual stack words destination/count/value; RET0Ch, EAX end. Explicit Source fastcall adds only an unused EDX placeholder to bind this shape. Original EDX is not an input. |
| 00486850..004868A5 | 86 | Three actual incoming stack words first/last/destination; plain RET/EAX end. Source cdecl, with its own two partially initialized local tags. |

The consumed fields and observed caller stack words are distinguished from
semantic C++ parameter hypotheses. In particular RET10h means **16 bytes** of
callee cleanup, despite each register kernel reading only one stacked word.
There is no three-argument fastcall type cast that drops the other cells.
Unused metadata parameters have no defaults. No Source-only EDX placeholder
belongs in the Original wrapper's prototype. No public entry promises noexcept.

`size_native_traffic_pod18_0047c4d0` reads first at header+4. Null first returns0
without reading last. Otherwise it reads last+8, subtracts modulo32 and applies
the actual signed IMUL/SAR/SHR/ADD /24 sequence. It does not validate a container
or use C++ pointer subtraction/size_t arithmetic.

All three range kernels copy **six float words with x87 FLD/FSTP**. Per-word
source read then destination write order remains, including forward overlap and
fill-value aliasing. This is not integer/byte memcpy: sNaN quieting, denormal and
exception/status effects, inherited control word and lower x87 values matter.
There is no floating environment normalization in production Source.

47FC30 has no destination-null guard. Equal endpoints skip all pointed access;
otherwise it copies forward until exact endpoint equality. 4841A0 checks its
CURRENT computed destination each iteration; zero skips all six pointed reads
and stores, then all cursors still advance24 modulo32. It still reads its actual
destination stack word when endpoints are equal. 486BE0 tests count before
reading the value stack word; zero count leaves incoming EAX unchanged. Nonzero
count loads that pointer once, rereads the pointed six words each iteration,
skips pointed reads/stores when current destination is zero and still advances.
Its incidental EAX is preserved physically but has no established semantic result.
No range subtraction, safety guard, null repair or alias snapshot is added.

## All direct caller setups were checked first

The complete assemblies of 483B20,48E670,48E400 and4932E0 were read before Source
interfaces were chosen, as were both owned wrappers and the stale-index candidate
486F80. Full target-verified listings, old prototypes, live xrefs and typed body
ranges are retained in the ignored packet directory.

| Site | Target | Actual setup at that CALL |
|---|---|---|
| 004933FE | 47C4D0 | ECX actual ESI header; no pushes. |
| 00483B4E | 47FC30 | Push tag1,tag0,destination,destination,last,first; ADD ESP,18h. |
| 0048687E | 47FC30 | Same six-word sequence from this owned dispatcher. |
| 0048E68C | 4841A0 | ECX first/EDX last; stack destination,entry-ECX,destination,masked-ECX-tag. RET10h. |
| 0049342E | 4841A0 | ECX current first/EDX insertion position; stack allocation,receiver,allocation,partially masked original count word. |
| 00493454 | 4841A0 | ECX insertion position/EDX current old end; stack current output,receiver,allocation,partially masked original count word. |
| 0048D314 | 486BE0 | ECX destination/EDX count; stack value,entry-ECX,value,masked-ECX-tag. RET10h. |
| 0049343B | 48D2F0 | ECX actual ESI metadata preimage; push local18h-value address, count, actual current output. RET0Ch. |
| 0049352C | 48D2F0 | ECX actual ESI; push local value address, computed remaining count, current old end. RET0Ch. |
| 0048E42A | 486850 | Push destination, current last, source first; ADD ESP,0Ch. |

At48D2F0 the incoming ECX is copied to a local, ONLY its low byte is cleared,
and the original ECX and modified tag are forwarded separately. The local is
popped back to ECX, so returned ECX is masked, not the original unmodified word.
The full four-word call to genuine Source486BE0 remains. EAX end is computed with
wrapped24 multiplication even for count0.

486850 reserves two UNINITIALIZED local DWORDs. It clears only their low bytes,
loads the full words and passes them as the final two physical arguments to
genuine Source47FC30. Their upper bytes are actual private-stack preimages; no
whole-word zeroing, compiler-materialized semantic tag object or copied frame is
substituted. The destination is pushed twice exactly as recovered. Returned EAX
is recomputed using Native signed /24 magic and wrapped multiplication by24.

The cached call index incorrectly lists486F80 as a caller of4841A0. Its complete
current assembly only calls BF67A7, while live xrefs identify48E670. That cached
edge is not used as evidence. Likewise49352C belongs to ordinary4932E0 continuation:
typed metadata gives4932E0 two ranges and Catch_All4934B4 only the21-byte range
4934B4..4934C8. No caller/control-flow/Ghidra mutation was made here.

## Verification and limits

Existing project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`,
x86 base00400000 was verified through `bsp.py` before analysis/export batches.
All390 captured live-analysis bytes equal the installed PE. Full Source public
bodies compile with MSVC Win32 `/MD /O2 /W4 /WX /EHsc /fp:strict /permissive-`.
The COFF checker compares every instruction byte: **382 fixed bytes and two
zero-addend REL32 CALL operands** to actual Source children. Source spans equal
all six Native spans, including alignment instructions, all9 branches and all36
x87 operations. There is no additional Source frame/EH code or external binding.
Current Source/header/object hashes are recorded in the report.
The report checker passed all12 transfer rows (10 unique sites; the two owned
calls are recorded both under their routines and the complete caller inventory).

One ignored combined fixture passed. It replays all six exact captured Native
bodies; only the two wrapper CALL operands are rebound to actual Source children.
An explicit assembly caller supplies every physical argument and ECX/EDX/EAX,
using a backed window in the real caller's thread-stack array. The bounded cases
cover per-word forward overlap with sNaN quieting/FP status, a retained lower x87
value, actual RET10h/RET0Ch/cdecl cleanup, zero-count EAX preservation, computed
null-destination skip, the masked ECX preimage and both nonzero-upper-byte local
tags. The fixture inspects actual leftover argument cells; no recording callback
replaces a child. Its FNINIT/FLD1 and seeded stack window are fixture-owned input
state, not production repairs or recovered Native frame addresses.

The standard `scripts/build.ps1` run also passed the two existing CTests. The
new module is unregistered in this worker, so that build does not cover it;
strict object/COFF/fixture proof is separate. Primary owns registration, final
archive-only linkage, annotations and export refresh. Reproduction tools under
ignored `local/goal15k_traffic_pod18_helpers/` are:

* `verify_coff.py [registered-object]`, writing `coff_receipt.json`;
* `probe.cmd [registered-bsp_core.lib]`, where the archive argument REPLACES the
  standalone object, for a final archive-only probe;
* `manifest.json`, six `.bin`/`.bytes.txt`/`.asm.txt` captures, whole caller
  listings, `typed_flow.json`, `object.dump.txt`, `probe_result.txt` and build log.

These hardware/library bodies have no CRT/exception child. Their local physical
ABI and complete instruction composition are established within the stated
tests; original C++ template identity, installed replacement, actual Native
private stack addresses, production owner composition and gameplay are unproved.
The full49E560 traffic parent, layout, resize/insert/erase/settings and mutable
world/axis producers remain explicitly incomplete. These six leaves do not close
that graph or adopt typed host callbacks as raw providers.

Primary annotation note: five conventional physical prototypes were applied
under the Ghidra write lock with old values retained. The `48D2F0` custom formal
signature remains unresolved. Its established inputs are a raw32-bit ECX
metadata preimage and three actual stack words at +4/+8/+C, EAX end and RET0Ch;
there is no Original EDX input or recovered C++ class. The configured bridge
disables script execution, and its storage tool only returns manual guidance.
The displayed undefined cdecl signature is an analysis placeholder, not evidence
of that ABI. Exact custom storage remains an annotation gate; Source body and
physical caller evidence are unaffected.
