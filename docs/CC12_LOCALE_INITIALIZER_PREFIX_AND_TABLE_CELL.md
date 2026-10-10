# Locale initializer prefix and exact table cell

Fresh saved Ghidra bytes establish that the unowned prefix at **CC8D30** supplies
**ECX = E1768C** to accepted initializer **436230**. The exact four-byte cell at
**CE2754** contains **CC8D30**. This is a bounded call-site contract; initializer
completion, exit registration, actual table dispatch and callback lifetime
remain unproved.

## Fresh admission and immutable Native captures

Before reading bytes, typed metadata checked every byte in CC8D30..CC8D39 and
CE2754..CE2757. The code is exactly two five-byte InstructionDB units, with no
contradictory boundaries. The cell is one defined four-byte DataDB. Every
admitted byte has null exact and containing function metadata. No function was
created and the prefix remains unowned.

Each batch verified project/program identity. Typed captures additionally
verified the actual marker **C:/Users/sqz269/bsp.gpr**, program
**/battlestationspacific.exe**, x86:LE:32:default, and image base 00400000.
Function count **64730** and modification number **92** are fresh before/after
values. Four CLI response pairs are byte-identical; all fourteen typed raw
responses and identity observations are equal. Only outer capture timestamps
differ. All Native reads closed before Root could resume annotations.

| Exact site | Bytes | Saved instruction/value |
| --- | --- | --- |
| CC8D30..34 | B9 8C 76 E1 00 | MOV ECX,00E1768C |
| CC8D35..39 | E8 F6 D4 76 FF | CALL 00436230 |
| CE2754..57 | 30 8D CC 00 | little-endian uint32 00CC8D30 |

There are **10 fresh unique code bytes and 4 data bytes**. Instruction-context
queries used context0 at only the two exact starts. CC8D3A, CD9410, all callee
bodies, neighboring table cells and CRT/handler/EH bodies were excluded.
These are saved Ghidra memory/listing observations; no fresh original PE
comparison or Native execution is claimed.

## Original call-site ABI and control flow

MOV changes ECX only. With prefix-entry ESP=S, CALL writes CC8D3A at [S-4]
and enters 436230 with ESP=S-4 and ECX=E1768C. This prefix supplies no stacked
argument. Neither instruction changes flags. The signed displacement is
-8989450; CC8D3A plus that displacement equals 436230 modulo 2^32.

Both instructions have override NONE and no fallthrough override. Their saved
default/effective flows agree: MOV falls through to CC8D35; the unconditional
call targets 436230 and has fallthrough CC8D3A. Target metadata says
BSP_LocaleNodePool_Initialize, no_return=false, nonthunk. Those flags do not
prove that any particular invocation returns.

The pinned accepted lifecycle contract supplies the callee ABI: receiver in
ECX, no stacked arguments, plain RET, normal EAX=receiver, EBX/ESI/EDI preserved
and EBP untouched. Composing that inherited normal-return contract yields
ESP=S and EAX=E1768C at CC8D3A. ECX/EDX/flags after the call are unspecified
here. No callee body or runtime register behavior was revalidated.

The prefix contains no guard, status test or failure branch. Allocation failure,
exceptions, partial construction and the actual FH3 dispatch relation
EBP=S=R+0Ch remain unresolved. The accepted table-free, section-delete and
list-unlink leaves do not prove their actual invocation. No completion,
registration success or callback lifetime follows from a saved fallthrough.

The CE2754 cell is mapped saved DataDB content holding the prefix address.
Its table role, neighbors, startup phase, execution order and actual dispatch
remain separate evidence requirements.

## Frozen Source and finite physical proof

HEAD **22e67a54181d183e3c7958ce236289298fb192bd** closes all **4217** src/include/cmake/CMakeLists
paths, **4248** selected actual Git blob occurrences and **11** commit/tree
objects. The extra 31 paths are named tooling and accepted prerequisites.
Replay verifies Git hashes, exact tree traversal, complete enumerated searches
and all Source bytes. Every Source path equals the prior accepted frozen
cohort. This is finite selected closure, not whole-repository blob closure.

The existing GameNativePhysicalPoolProcess borrows its actual common head by
reference through AllocatorListDomain. GameNativeStringProcess retains the
AA8/AA4/AA0 services. These are authenticated Source reuse candidates. The
enumerated searches identify no canonical locale owner; differently named or
indirect ownership is not excluded. Original fixed-address identity, the same
published locale element, actual storage/slabs/payloads and borrower quiescence
remain unproved. A new pointer or zeroed layout would not satisfy them.

The prior metadata archive is physically nested and replayed, with its report,
Root acceptance and lifecycle/profile/unwind/cleanup dependencies pinned. Its
23,790,030 bytes retain SHA256
`bcd6104842475fdb34df620e664e5357d84e31c936dd59f2264fefa37747c74d`.
Prior Native metadata, cleanup leaves and particle Source3356 artifacts receive
no fresh byte, build, ABI or runtime credit.

Original and physically copied archives both pass ZIP-only guarded replay.
External file reads, processes, network and native-code loading are blocked;
three negative guard probes pass. The copied ZIP and three replay scripts are
distinct files with identical bytes. A postcommit finite Git delivery proof
pins the exact two immutable output blobs separately to avoid self-hash cycles.

Archive: `local/cc12_locale_initializer_prefix_and_table_cell/evidence.zip`

SHA256: `6df90fe269bcee5a56b32ab1ebf685524e7e714da91b7fbdda3507c1c757ba85` (46,713,562 bytes).

## Smallest separately gated continuation

Fresh flow metadata identifies only **CC8D3A** as the next normal-return site.
The next packet should first refresh exact typed ownership, boundary, length
and flow at that address. This proposal admits no new raw bytes. After a
separate Root admission, at most that one complete measured instruction can
be captured, with per-byte ownership checks. Further scope must follow its
fresh metadata; no arbitrary initializer hull is proposed.

No C++, CMake, build, test, probe, GPR mutation, drop-in ABI or game-validation
credit is added. Full machine-readable evidence and uncertainty are in
`reports/cc12_locale_initializer_prefix_and_table_cell.json`.
