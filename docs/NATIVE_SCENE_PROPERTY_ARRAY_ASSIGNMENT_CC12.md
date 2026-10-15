# Whole raw scene-property array assignment (CC12)

Four complete Original entries now have independent MSVC Win32 naked Source
bodies. This worker authored and strictly compiled them, inspected the complete
actual COFF and ran the required existing repository build. Source admission
credit remains **zero** pending Root review, registration and admission.

| Entry | Original half-open interval | Bytes/instructions | Literal bytes | Whole Original SHA256 |
| --- | --- | --- | --- | --- |
| 008F0460 | [008F0460,008F04B6) | 86/33 | 74 | `1aa4e7ed871e8547fdc979337d00ec3017e793b3c63be3ea630a672c81151ea0` |
| 008F04C0 | [008F04C0,008F051A) | 90/35 | 78 | `07c8d0cca1c813e7496447bc2834fcff05897b6f1d5f7886cf17e96b336026bb` |
| 008F0520 | [008F0520,008F057A) | 90/35 | 78 | `0c17b9c96e87c2b67b337c81a3abf3856ae7787f0ffc2edff04546a879d2ad37` |
| 008F0580 | [008F0580,008F05DD) | 93/36 | 81 | `3dc15a9fde6100ce0c7362250fa4af5882e65112edde826da325d9fd94d80aed` |

The four bodies total 359 bytes and 139 instructions. The strict actual object retains
all 311 non-CALL-operand bytes and every branch, read, store and both RET0C
instructions. Exactly 48 bytes, twelve operand bytes per body, are symbolic
current-provider relocations. Names are hypotheses, not recovered symbols.

All use ECX=actual record, incoming EDX unused, EAX=actual record and RET0C.
The physical entry must retain an unused EDX formal so that all three actual
arguments remain stack DWORDs. Save/restore ESI and EDI as the whole body does.
Use a uint32_t flag word, not C++ bool: exactly its low byte controls the branch;
all 256 low-byte patterns and arbitrary high 24 bits are admitted. No bool
narrowing, guard, added flag store or change to byte-read timing is allowed.

The only record writes are full pointer+20 and full byte-size DWORD+24, eight
bytes total. Other 48 bytes of the complete 56-byte record retain their actual
prior values. No root construction, tag, phase, name, defaults, owner, marker,
lookup, context binding, startup, publication or rollback is part of these
entries. Constructors cannot replace them: constructors store phase/tag and
defaults with different schedules. This Source uses four independent
MSVC Win32 naked bodies and only rebinds the three CALL operands in each body.
The standalone strict COFF gate below verified those bytes; integrated Main build remains separate.

The optional free occurs first: capture old+20, free if nonnull, and clear +20
only after free returns. 0460 loads its byte-count argument after that free;
04C0 and 0520 load count and double it twice before free. 0580 loads count
before saving ESI/EDI, computes 3*count with LEA and doubles twice before free.
Arithmetic in the original is modulo DWORD; this proposed admission requires
positive nonwrapping sizes/ranges rather than changing arithmetic or adding
guards. Permit count in [1,3FFFFFFF] for stride4 and [1,15555555] for stride12.
Raw byte count is positive uint32_t subject to successful allocatable ranges.

After optional free/clear, every entry compares the low flag byte BEFORE
storing size+24. 0460 loads count between that comparison and the size store.
The conditional branch follows the size store. On nonzero low byte, allocate
the exact positive byte size, then load the input pointer after allocation,
push size/input/actual allocation, store the full actual allocation at +20
BEFORE calling memcpy, and return the actual record. No-copy loads and stores
the exact supplied full pointer at +20 without allocation, then returns.
There is no fallback, RAII cleanup, transactional update or rollback guarantee.

The three direct calls in every entry are free 00BF6989->00BF65AC (then the
private CRT implementation), operator-new 00BF55BE->00BF681B, and memcpy 00BF7680.
All required successful current Source providers already exist: unchanged
canonical singleton_lifetime_free uses std::free; unchanged canonical
singleton_lifetime_allocate implements malloc/_callnewh retry/bad_alloc with
actual_bytes==native_bytes==host_bytes; a new private genuine CDECL size bridge
may bind that allocator exactly as the accepted type8/9/10/11 raw constructors
do, receiving no native-body credit. Call real current memcpy and retain
#pragma function(memcpy). There is no missing direct provider. Historical CRT
free/new-handler/exception/vtable/global identity and backward-overlap memcpy
behavior are not imported by naming these current providers.

Require actual complete live raw56 records whose storage provenance is an
accepted whole type8/9/10/11 producer, preserving its real untouched bytes.
The accepted producers construct RAW storage, not C++ class lifetimes. For a
record with an existing nonnull +20, that allocation must be a uniquely owned
live allocation of the same current heap and removable exactly once. The
constructor's copied branch supplies such a child; a retained pointer does
not become owned just because +2C is one. The old-null branch can follow the
accepted whole array-block release on the actual +20/+24 interior, preserving
the rest of the same record. A tag number or synthetic buffer is not provenance.

Copying input must be stable/readable for the exact size and disjoint from the
complete record, its active frame, the entire old allocation freed first and
the new allocation. No-copy in this Source domain requires an actual separately
allocated live uniquely owned same-current-heap incoming payload of the stated
positive size. Its owner transfers sole retirement responsibility to the record
when the native pointer store occurs. It must be disjoint from old allocation,
record and frame, and have no competing owner/retirement. This is a caller
precondition, not an invented ownership action in the leaf body. The cached
word "borrow" only describes storing a supplied pointer. It cannot establish
safe class ownership. The accepted release 008F0640 for tags 8..11 calls whole
array release 008F03F0, which frees every nonnull +20 irrespective of +2C.
No caller-managed borrowed interval is admitted to later retirement here.
Retire each new live child exactly once through the accepted current release
before discarding the raw record or handing off to an accepted lifetime path.

If composed into bags, all existing complete producer/map/node/key/context
preconditions remain mandatory: same actual bag, canonical process/pool/gate
cells and initialized pool generation, unique slots/keys, coherent ownership,
finite unshared acyclic graph, owners outliving every borrower, and all
retirement before actual pool teardown. This leaf introduces no context or
startup substitute. Raw assignment alone does not prove that composition.

Use valid nonwrapping writable record/stack ranges, valid flat ES, DF clear,
successful returning current providers, sufficient resources, no faults,
throws, callback/new-handler reentry, concurrent mutation, trim, teardown,
unwind or aliasing. Providers may clobber volatile registers. EAX returns root;
no-copy EDX is the supplied pointer. No-copy ECX remains root only when no old
free occurred; otherwise it is the real free provider's residual. Copy ECX/EDX
are real provider residuals. No-copy final CMP defines six arithmetic flags
(mask8D5=44, AF=0); copy final ADD ESP,10h defines arithmetic flags from
entry T: (T-24)+16=T-8. EBX/EBP depend on genuine provider normal ABI. No blanket
DF/ES/FPU/MXCSR preservation or exceptional naked-frame unwind is asserted.

Zero count/size is explicitly outside both branches of this positive domain.
Neither current zero-size allocation success nor null memcpy arguments are
assumed. A later expansion would need a separate exact allocation/memcpy and
pointer-lifetime contract; this Source inserts no special zero-size path.


The prior sealed readiness is the admission basis, not this worker's current
execution claim. Root published it in
`reports/cc12_property_array_assignment_readiness.json` at 9386E45E7. This
worker's standalone four-entry Source contains no parser, loader, phase/tag or
context substitutes. Native parser 008F5A00 has a call at 008F6684 into 008F0580;
the existing host SceneLexer/tree parser remains a different interface. Those
outer paths are not missing direct providers and remain unexpanded.

## Strict compiler and complete object evidence

Exactly one command used `/std:c++17 /MD /O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL-
/fp:strict`, `/sourceDependencies` and the actual x86 compiler 19.51.36244.
It returned naturally with exit0. Before that invocation, this worker freshly
hashed and copied 151 candidate compiler inputs, 67 tool files including both
clui DLLs, 28 current provider/context files and the build configuration.
Candidate paths came from the 119 prospective / 137 actual holder and 82 actual
setter inventories; project src/include paths were mapped to this worktree,
installed paths remained literal. The compiler declared 81 actual Source/include
inputs. Every actual input matched its own literal precompile pin; no missing
input or after-compile first-observation qualification was needed. Actual
dependency readback occurred after the normal build had started. This is not a
hermetic OS, DLL-loader or SDK proof; normal-build dependencies are not claimed
as that strict TU's input closure.

The actual 2702-byte I386 COFF SHA256 is
`a3230b09f46bf3671db1407c51c0e56cf851f9f6a102d749899ab05756490c6d`.
All nine section headers and contents, every relocation,31 primary/auxiliary
symbol slots, the complete string table and every file byte were accounted
for. Five complete executable sections contain 417 bytes and 159 instructions with 15
relocations. No file bytes or instruction islands are unaccounted for. The
four entry sections contain 359 bytes and 139 instructions and twelve exact REL32 CALL bindings: four
canonical frees, four private size-adapter calls and four real `_memcpy`
references. This unlinked object proves symbolic references; it does not prove
a loaded runtime import address or physical DLL binding.

The separate genuine CDECL adapter is 58 bytes and 20 instructions with three relocations.
Its argument is loaded from EBP+8, copied to request native_bytes and host_bytes
at EBP-C and EBP-8, with kind object=3 at EBP-10. It passes the actual request
address to unchanged canonical singleton_lifetime_allocate, then performs the
normal compiler GS-cookie check and returns the allocator's full EAX. The
cookie load and check are genuine compiler/runtime boundaries, not Native-body
reconstruction credit. The adapter receives zero Native credit. No native
entry has a cookie/prologue wrapper or additional call.

Noncode metadata was also reviewed: `.drectve` 305 bytes requests MD release
msvcprt/MSVCRT/OLDNAMES and standard mismatch/weak-Avx2 aliases; `.debug$S` 248 bytes
contains C13 S_OBJNAME and S_COMPILE3 records with this actual object path,
x86 machine 7 and frontend/backend 19.51.36244; `.bss` 4 bytes is the weak
Avx2WmemEnabled value with no raw file data or initializer; `.chks64` 72 bytes retains
all nine opaque checksum words. Section auxiliary sizes/relocation counts,
COMDAT selections, absolute compiler/feature symbols and complete long names
were inspected. No metadata relocations or static initialization sections
exist. The only undefined symbols are canonical allocator/free, `_memcpy`,
security cookie and its check. No code was executed from this object.

## Existing repository build and remaining qualification

Exactly one normal `scripts/build.ps1` invocation completed naturally with
exit0. Its actual configured CTest checks were reconstructed_math and tool_tests,
both passed (2/2). Native math differential was absent because
`local/seed_reference.hpp` did not exist; cc13 leaf differential was likewise
absent without its conditional header. No seeds were generated and no build
was replayed. The new TU is unregistered and absent from the generated
bsp_core project, so this normal build checks the existing repository and
does not qualify the new TU as an integrated production build. Generated
project/cache/CTest/log copies were first observed after the build.

The evidence family is
`local/WholeScenePropertyArrayAssignmentSource01/`. It preserves a literal
copy of the complete 136-file readiness family, fresh precompile inputs/tools,
all actual compiler inputs, strict and normal logs, full object/file review,
own UUID original raw tool records including initial/poll/natural terminal
outputs, and four staged Git blobs with explicit literal/EOL qualification.
Its root seal includes every nested file, including nested root manifests;
only its exact own root manifest is excluded from its payload hash set.
The original 136-file readiness,448-file reference/subbag review and 462-file
holder review remain untouched and read-only.

No new tests, probes, helper fixtures, linked Native-entry execution, selected
populated graph, startup or game validation was performed. No CMake/registry,
ledger or Ghidra mutation was made. Root owns independent peer/Primary review,
Main registration/build, whole current helper/code/provider qualification,
saved analysis publication and the final Source-admission decision. Private
Original CRT/EH, full binary ABI, zero-size/failure/alias/reentry/unwind and
outer parser/runtime/game behavior remain outside this bounded Source work.
