# CC12 physical reference payload copy (008F0340)

The worker implementation and focused fixture passed. **Source 0 / ready 0**
remain pending Root's independent fresh fixture, full main build and publication.
This packet implements the actual eight-byte payload operation. It does not
close the native scene-property class, constructor, destructor or game caller.

## Native body and interface

`copy_native_reference_payload_008f0340(void* destination, void* unused_edx,
const void* source)` is a Win32 MSVC padded `__fastcall` interface returning
`char*`. ECX is the actual destination payload, EDX is unused, and the sole
stack DWORD points at the actual source payload. The complete native range is
`[008F0340,008F0377)`: **55 bytes / 22 instructions**, SHA-256
`40611ad298e15a07908bead1b10f73434e3574529a027779139398e0f3edd3c6`.
Names are descriptive hypotheses, not recovered source symbols.

The body snapshots source +4 borrowed text in EBX and source +0 scalar bits in
EDI before calling free. A nonnull destination +4 is passed to the actual
current CDECL canonical free, followed by ADD ESP,4 and clearing destination +4.
It then passes the borrowed text in ECX to the admitted physical 00438E40
duplicate. On successful return it writes the snapshotted scalar and actual
new allocation into destination +0/+4. **Full EAX is the new allocation or null**;
the destination pointer is not the returned value. RET 4 consumes the source
argument. EBX, ESI, EDI and EBP survive normal return.

The source body retains 47 literal bytes and changes only the two REL32 operands:
`[23,27)` binds the actual freshly compiled canonical free; `[40,44)` binds the
actual freshly compiled admitted physical duplicate57. Both the relocated native
body and Source call these identical providers. No original private free, legacy
CDECL string duplicate, fake object, vtable, callback substitute or forced output
is involved. No other payload/record bytes or phase fields are written.

Root repaired the saved CALL_RETURN override at 008F0356 once, observed its old
value and replacement NONE, and decoded the genuine ten bytes 008F035B..008F0365.
Root's repair manifest is
`0b689c4d55a5231645e134a032aac5170af49271318e311df74d764f7b670bb8`.
The current actual saved listing has 22 instructions. The cached signature's
20-instruction count is historical. This worker performed no Ghidra mutation,
flow-repair replay or type-5 packet query.

## Storage and ownership contract

Both payloads are actual readable eight-byte storage; destination is writable.
Destination +4 must be null or the sole-owned actual result of the matching
current duplicate/provider. The source payload and its borrowed NUL text remain
live throughout the call. Text length plus NUL must be representable without
address wrap. Payloads, borrowed text, old allocation and active call frames must
be disjoint. Current CRT execution requires DF clear.

Snapshotting the source pointer does not retain its allocation. A nonnull
self-copy or shared source text pointing into the old destination allocation
would free that text before duplication and is outside this contract.

The consumer frees a real old allocation exactly once. The caller frees each
nonnull new returned/stored allocation exactly once with the matching canonical
free before reuse or disposal of the destination payload. Borrowed text is not
freed here. An allocator may reuse the old address: the fixture observed this
for both native and Source nonnull replacement calls. Address inequality is
neither required nor promised, and old allocation bytes are never read after
the target returns. The function has no `noexcept` promise; OOM/EH behavior and
native class exception cleanup remain unadmitted.

## Nested stack and arithmetic flags

Let T be the actual source argument-slot ESP just before the outer target CALL.
Outer entry is T-4. Its saved EBX/ESI/EDI occupy T-8/T-12/T-16. Optional old free
uses argument T-20, entry T-24, return T-20, then ADD4 restores T-16.
The duplicate enters at T-20, saves EBX/ESI/EDI at T-24/T-28/T-32, and places its
size argument at T-36. Its memcpy destination/source/count slots are
T-48/T-44/T-40, memcpy entry T-52 and return T-48. The final ADD16 yields T-32;
child RET0 returns T-16, and outer RET4 yields T+4.

For null text, full EAX and ECX are zero, and arithmetic mask `0x8C5` expects
`0x44`; XOR's undefined AF is excluded. Incoming EDX is preserved only with a
null old pointer. A real old free may clobber EDX even when the new text is null.
For nonnull text, mask `0x8D5` is calculated from `ADD32(T-48,16)`; ECX/EDX are
volatile. No blanket EFLAGS, floating-point, MXCSR or segment-state claim is made.

## Fresh focused fixture and gates

Exactly four fresh translation units compiled: this consumer, the unchanged
admitted duplicate and adapter, unchanged canonical allocation/free, and a new
probe. No BSP archive, old object or old process was used. Win32 MSVC /MD /O2
/W4 /WX /fp:strict /permissive- /EHsc /Gy /GL- builds used /Oi- for the duplicate,
/OPT:NOICF and an embedded asInvoker manifest. One accepted process ran once.

All 38 complete code spans were resolved from current COFF/link/map evidence and
checked before zero consumer, old-setup duplicate or target-related free entries,
then checked again afterward. Coverage includes consumer55/22, duplicate57/30,
adapter61/17, allocator90/34, free6/1, ordinary caller18/5, raw caller140/52,
**entire main5183/1288**, all retained probe helpers, actual import thunks, the
14-byte/four-instruction security-cookie helper, and the complete
43-byte/19-instruction stack probe. The large actual main frame is covered;
the gate does not compare only a prefix or omit its stack helper.

Before target setup and afterward, actual IATs for UCRT malloc/free/_callnewh
and VCRUNTIME memcpy were matched to actual physical I386 modules and exports.
Checks covered MEM_IMAGE, export RVA, mapped/physical NT paths, volume/file ID,
full file size/SHA-256 and ASLR-relocated physical export bytes. memcpy resolves
to the distinct physical `VCRUNTIME140.dll`, not the UCRT allocation module.
Provider-observer activity is separate from the explicit target counters.

The seven entries were three native/Source raw pairs (old-null/text-null,
real-old/text-null, real-old/nonempty-text), plus an ordinary Source call with
old-null/nonempty-text. Four actual admitted duplicate calls set up old owned
strings after all gates; four actual in-consumer frees completed. Three actual
new copies were observed live through the complete seven bytes `c78135e2694d00`
including NUL, then caller-freed once. Total canonical frees: seven.

All actual source and destination payloads had full 48-byte storage with 16-byte
prefix and 24-byte suffix guards. All seven before/after destination and source
images, six raw register/flags captures, actual argument slots and pointers,
full borrowed text preservation and matching raw caller frames were checked.
Observed flags were 582, 582, 518, 582, 582, 518; only justified masks are asserted.
No requirement compared new allocation addresses across the native/Source pair.

## Provenance and integration boundary

The family is `local/cc12_reference_payload_copy_worker20261008a/` in the worker
worktree. `runtime.json`, `static_gate.json`, `post.json` and the structured
tracked report carry exact identities and receipts. The accepted core seal is
`386ab38eafbe57a17f4d5e47c28b227c121b133ea4b29c8894b98625af1226bf`
and covers 103 artifacts. A final manifest adds all subsequent metadata and
handoff files, excludes only itself, and is verified against exact inventory.

Post verified all 1,100 prior artifacts plus all actually consumed sources,
headers, libraries, tools, PE/providers, objects and executable unchanged.
Mutable unconsumed dispatch/CMake metadata was snapshotted separately; historical
hash associations were retained. The original native 55 bytes matched before
and after. The old recipe/probe were read as text only to author a fresh fixture.

Two setup issues remain recorded: preflight corrected a stale `len(rows)` name
before any accepted stage, retaining the draft; the first static gate's substring
lookup confused `_printf` with `___local_stdio_printf_options`. A separate exact
symbol metadata helper consumed the four unchanged COFF reports and completed
all 38 gates. The build and process were never rerun. The accepted recipe and
failure records remain immutable; a Root fresh recipe must incorporate that
exact-symbol lookup correction before its own first static gate.

Root still owns the independent fresh fixture, full main build, shared metadata,
Source admission and publication. Worker changes are restricted to the new
header/source and this doc/report. No current result admits native private-heap
compatibility, class/type-5 producer/destructor closure, World integration,
OOM/EH, full caller behavior, startup or gameplay.
