# Raw texture-atlas loader, matcher and descriptor admission

The proposal contains whole ordinary **normal** Source bodies for AF0060,
AEF3C0 and AEF280. It replaces no existing tracked source, semantic atlas,
registry, texture cache, VFS, renderer, startup hook or host behavior. Names are
hypotheses. Native graph creation and actual activation remain caller work.

| Entry | Native interface | Complete original listing |
| --- | --- | --- |
| AF0060 | ECX actual1C manager; requested-header pointer stack; RET4 | 304 instructions, 935 bytes |
| AEF3C0 | ECX requested raw8; EDX candidate raw8; AL result; RET | 516 instructions, 1633 bytes |
| AEF280 | entry ECX unused; descriptor-header pointer stack; RET4 | 92 instructions, 305 bytes |

All complete original assembly ranges, including AEF3C0's final suffix/concat
tail, were inspected through `bsp.py show --asm`. The new normal behavior uses
the actual current raw fields and the instruction-defined capture epochs.
Complete live byte captures and installed-PE equality are retained in evidence.
Live program/project defaults were checked; no Ghidra mutation was performed.

AF0060 captures the requested-header HOME before the first diagnostic data read.
It clones to an actual raw8, lowercases it, then performs real469840 and the
current CRT case-insensitive suffix comparison. It creates raw backslash and
slash headers, uses both real467CF0 reverse searches, and chooses the signed
maximum. The directory includes the final separator. The directoryless arm
uses the actual immutable D5D7D0 `"./"` bytes, not a generated path default.
The extension is the actual immutable D5D7CC `"ats"` string.

Enumeration directly reuses the existing whole raw886280 implementation,
`enumerate_native_language_resources_00886280`, over the **same** actual VFS
manager publication, actual intrusive0C list, raw pool and actual BDD990
enumeration services. Its concrete Calls object is embedded privately, not a
caller-supplied callback. Real557A90 pops each raw8 candidate; real4D05E0 clears
the list before the current sentinel is freed and nulled. No memory stream,
standard-string sequence, alternate enumeration, name projection or empty
success route is supplied.

Each popped candidate gets distinct retained matcher and descriptor metadata
before the pop's effects. AEF3C0 receives the **original current requested
header**, not the lowercase working copy. On each matched candidate, AF0060
starts a fresh scan of its captured manager: signed current count at+14,
current pointer array at+10, current texture pointer and actual B33E40 name
address. It reads stored length then original request length. Nonempty equality
reads request data before stored name data, then invokes current CRT stricmp.
The comparison is against the **requested descriptor**, never the candidate.
The scan is repeated after each prior load and cleanup; neither registry
count, array nor names are snapshotted across callbacks. A match skips admission;
reaching the current count without a match invokes actual AEF280.

AEF3C0 uses actual six raw8 local regions with the original local reuse. It
deep-copies and lowercases each input, then normalizes backslashes by whole
raw4CAD40. Initial slash/backslash return operations use the original captured
data and length registers, even if later calls changed those headers. It tries
exact equality first, then the unsigned candidate-length gate. Both real467CF0
slash searches must produce the same signed index. The accepted established
467CF0 helper deliberately does not test position zero. If a directory exists,
the candidate prefix is constructed before the requested prefix, compared
using449AF0, returned in native order, and the two names are stripped using
real469840 plus exact425F40. Both `.ats` suffix checks remain separate. The final
request stem plus underscore is constructed through exact4261A0; the candidate
prefix length is **requested length minus three**, not the candidate's stem.
All return paths preserve their own native cleanup order, including the early
length/slash reject's captured lengths and the final current-header footer.

AEF280 does not consume entry ECX. It first performs real BF681B(1C) and AF5600;
the descriptor argument HOME is read only after the constructor returns. Its
raw8 temporary name is copied with exact resize/copy, then actual BDF4C0 is
called using a distinct retained `NativeVfsNameResolutionAcquired`. The original
descriptor's **current data pointer** is read unconditionally after resolution.
Failure selects the actual F8C270 empty-name role when needed and reaches the
actual4254B0 bare RET; no added logging callback changes effects.

On admission success AEF280 calls the real raw AF5850 provider with a second
distinct retained name-resolution acquisition and the same actual VFS services.
Its returned count is deliberately ignored. It then reloads **current F8C26C at
AEF33E**, initializes a separate parser buffer DWORD HOME, and directly invokes
the sealed whole raw AEEAF0 parser. It does not substitute the entry manager,
reuse admission's resolution, skip parsing after a zero read, or manufacture
an empty atlas. Normal cleanup runs actual AF5620 and frees the captured text
register, then returns current copied-name storage. Original allocation-null
behavior remains unsafe on the reached success arm, with no recovery branch.

The cold Context borrows the genuine RO service, same raw pool and canonical
live feature word, a retained `GameNativeVfsRawServices` view, actual F8C26C cell,
actual F8C270 empty-name storage and the genuine parser Context. It checks the
VFS string wrapper and enumeration wrapper share the raw cells. The parser's
same RO/pool/feature and Source-created renderer/cache custody are explicit
caller obligations; this interface cannot mint that provenance. There is no
second renderer/cache observation, fake publication, factory duplication,
default atlas, semantic registry or numeric engine-function dispatch.

The established raw886280/557A90 actual string-wrapper/CRT/exception contracts,
raw-pool4D05E0 contract and actual BDF4C0/AF5850 boundaries remain qualified. In
particular AF5850 retains its previous Source/private argument-slot alias and
current CRT copy qualifications; this packet does not silently expand that
callee's domain or claim a new original ABI. The exact469840/425F40/4CAD40 closure
and sealed exact426060/4261A0 are direct dependencies of the new bodies. Full
parser/canonicalizer/item/renderer/registry custody and lifetime are external.

Fresh nonmoving frames retain native homes, allocation entry/return history,
both VFS acquisitions, the one existing parser/cache acquisition per descriptor
and child helper states. Source metadata lists are operation histories, never
actual atlas arrays. Preserve complete partial graph debt after failure. C++
exceptions record and bare-throw without invented cleanup/retry/rollback.
Existing failed acquired-frame destructors may terminate, so destroying a
failed enclosing frame is not a cleanup permission. Selected native FH3/SEH,
register/private-stack/fault-PC/NoThrow/Origin and binary ABI remain outside the
normal-flow interface.

This packet provides no actual common.ats startup, CFEAE8 singleton arm,
registry birth/publication, manager deletion, GUI/Page activation or game proof.
The known original startup call73DE42 to AF0060 is before GUI startup and must
still be connected to genuine Source roles by Root. A valid empty registry is
not that populated pre-GUI graph. Existing nine gates remain held.

No optional tests or build/runtime/game/install checks were run. No tracked
source/CMake/ledger/Ghidra/commit/push action occurred. Root must review the
sealed candidates, join the exact siblings and baseline, register them in the
build and metadata, compile the joined tree, and qualify actual runtime later.

