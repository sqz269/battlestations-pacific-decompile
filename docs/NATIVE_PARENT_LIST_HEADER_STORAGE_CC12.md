# Native parent list header storage (CC12)

Implemented whole `[004B 7EC0, 004B 7ECD)` as the literal **13-byte / 6-instruction**
MSVC Win32 naked function `initialize_native_parent_list_header_storage_004b7ec0`.
The name is a hypothesis. Root Source admission credit is **1** for the fresh,
unowned writable 12-byte interface qualified below. Worker credit remains 0.

Native SHA-256: `1e09d34a9de7a48bd061703ef73f341df8596595468b282c5f19c36b6bf0b98d`.
Whole hex: `8B C1 33 C9 89 08 89 48 04 89 48 08 C3`.
Whole Original, Source COFF, and unique linked executable body match exactly;
there are zero CALLs and zero relocations. No provider, phase dispatch, node,
sentinel, owning parent, or World is invented by the production body.

## Physical interface and storage

```cpp
void* __fastcall initialize_native_parent_list_header_storage_004b7ec0(
    void* actual_storage_root, std::uint32_t unused_edx) noexcept;
```

ECX is an actual fresh, unowned writable 12-byte storage region. The explicit
unused EDX formal occupies the second register; there are no stacked arguments.
MOV EAX,ECX retains the actual full root, XOR ECX,ECX produces zero, and three
DWORD stores zero `+0`, `+4`, `+8` in that order. EAX returns that root; ECX is
zero; EDX, EBX, EBP, ESI, and EDI are preserved. Plain RET at `004B 7ECC` restores
caller ESP without argument cleanup. All 12 bytes are written; none within the
requested region are preserved. There are no guards or allocation calls inside
the native/Source leaf.

The sole flag writer is XOR: CF0/PF1/ZF1/SF0/OF0, defined mask `0x8C5=0x44`.
AF is undefined and excluded. DF remains unchanged; this call-free leaf permits
raw DF1. The probe restores DF0 before current CRT operations. ES is untouched.
No numerical floating-point behavior is claimed. A populated owning root must
not be reset: the zero stores provide no node disposal or owning lifetime.

The frozen readiness audit establishes this as the actual constructor callback
in the genuine owner descriptor and documents one real iterator invocation.
The owning iterator frame, exception cleanup, sentinel/private heap, class
destructor, and parent/subject relationship remain outside this raw interface.

## Current complete-helper qualification

Root's fresh `local/h13p4` contains exactly **610 sealed files**. Its seal SHA256
is `cfac97ba8a6c7d587995ceb77320d5369f5104658f70155d908a80b6c5985a49`. Current Main's entire leaf/header and
canonical allocation TUs match the fresh compiled inputs. Root and an external
reader checked all **41 complete linked spans / 6,697 bytes / 2,045 instructions**,
including the complete **2,615-byte / 719-instruction Main**, raw 173B63 and
ordinary 6B2 callers, actual cookie14/chkstk43, nine normal import thunks, and
the cold standard/delete16-to-5-to-free6 chain. GS failure is the only retained
unexpanded frontier; its execution remains outside this qualification.

The sole new process passed raw Source DF1, raw Original DF0 and ordinary Source
DF0: **three calls, three allocations and three frees**. Root directly decoded
all three full CaptureBox96 records and full live 44-byte buffers. Each fresh
12-byte header was zeroed with both surrounding 16-byte regions preserved.
Defined XOR flags, unused EDX, nonvolatile registers, ESP/EBP, ES and DF passed;
undefined AF is excluded. DF0 was restored before current CRT operations.

The actual external recorded reader passed without a new target or provider
query. Recorded UCRT malloc/free/new-handler IAT/export bindings, physical file
identity/path and loaded-base-adjusted 32-byte prefixes agree with frozen
evidence. Its IAT column is a resolved pointer; module base is derived, and
MEM_IMAGE was checked by the gated probe but not separately serialized.
Native13, six instruction starts, project/program, installed game PE, physical
provider and selected-input post bookends passed. **21,630 prior pins** and
the terminal R03 **540-file family** remain unchanged. Earlier inspector stops
and corrected own utilities are retained in the new seal; no old stage ran.

The current combined Main build at `0b2eede1007b74f45a1a85622f359a288a4fbd89`
passed all three existing checks. No production code or tracked tests changed
for this qualification. Saved-analysis publication is recorded separately.
The historical sections below retain their original fixture and coverage hold;
their pending statements describe those older attempts.

This admits only fresh unowned writable12 supplied storage. Owning parent/World,
sentinel/private heap, iterator/EH/class lifetime, allocation failure, general
binary replacement, startup and gameplay remain unvalidated. Consumer families
retain their separate evidence and qualifications.

## Historical worker connected validation

Baseline `1c027b594dbad3b4ae7cc5cba03281320e7a8f40`. Accepted ignored family:
`local/h13/r02`. It compiled exactly three new TUs: this leaf, unchanged current
`singleton_lifetime.cpp`, and its new probe. BSP archives linked: **0**; old objects:
**0**; new tracked tests: **0**. Actual MSVC 14.51.36231/SDK 10.0.26100.0 tools,
compiler backends, **184 consumed headers**, and **seven system libraries** were
pinned before/after and frozen. Flags included `/MD /O2 /W4 /WX /fp:strict
/permissive-`; the I386 executable embeds an `asInvoker` manifest.

Before the sole process, all **25 complete linked application/canonical/helper
spans** were resolved against COFF relocation targets and map ownership. They
include canonical allocation 90B34/free 6B1, all linked probe/exception helpers,
complete main 2454B681, and the actual security cookie helper 14B4. Its named
external failure tail is qualified without expanding generic GS failure.

The ordinary compiler caller is complete 6B2: CALL the actual Source leaf then
plain RET, preserving receiver/unused-register inputs and leaving no stacked
argument or flag-changing epilogue. The complete raw capture is 172B62 with one
indirect CALL `[EBP+8]`. Manual review bound the actual caller frame, snapshot
offsets, two stack canaries, ESP restoration, DF handling, and exact executable,
map, gate magic, and gate-key layout before launch.

The process executed exactly **three leaf entries**: raw Source DF0, unmodified
Original13 RX DF1, and ordinary compiled Source DF0. Original needs no callee
patch. It obtained **three actual 44-byte allocations** through the canonical
current allocator. Each contains a fresh 12-byte header at offset 16, with
16-byte prefix and suffix canaries **inside the writable requested allocation**.
These are not adjacent heap metadata/red zones or fabricated parent objects.

Every live 44-byte buffer, all 12 initialized header bytes, other live buffers,
full EAX root, ECX, EDX poison, all nonvolatiles, ESP, defined flags, DF, ES,
capture canaries, and raw stack canaries passed. Full observations are preserved
in the report and immutable process output. Each actual allocation base was
explicitly canonical-freed exactly once after live checks; no interior header
was freed and no freed buffer was read. No nodes or populated roots were created.

Before zero canonical allocations/target entries and after completion, the
probe checked malloc/free/_callnewh IAT targets, physical exports, loaded I386
UCRT image metadata, mapped NT path, actual file ID, full file SHA-256, and
ASLR-normalized physical export prefixes. The observed mapped/file paths both
resolve to `\Device\HarddiskVolume3\Windows\SysWOW64\ucrtbase.dll`, full SHA-256
`60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`. All linked code spans and unmodified Original
bytes were checked before and after. Canonical allocation failure/new-handler
and exception paths were not dynamically exercised.

Live native before/after reads used the supported CLI and verified
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and installed PE
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` each batch.
Workers made no Ghidra or shared metadata/CMake mutation. No original game write,
startup, or gameplay execution occurred.

## Immutable evidence and qualifications

The stopped `r01` contains 96 unchanged files and had zero canonical allocations
or Source/Original entries. Its static parser's `_printf` substring also selected
`___local_stdio_printf_options`; the new `r02` prioritizes exact symbol ownership.
No Source/API change or old successful phase/process replay occurred.

All **9,852 prior pins** remain unchanged: readiness/older history 9,756 plus the
96 stopped files. The 135-file callback audit and its 9,621 older pins retain
their original frozen Source/path/hash associations. Generation dispatch
hashes and exact bytes are frozen via a post-only Git blob recovery, alongside
the fixed baseline registered packet. Changed current-main metadata is distinct
unconsumed context. CMake and BSP archive snapshots are unconsumed; no old
receipt is repinned to a newer Core and no live shared CMake hash is required.

The accepted `r02` seal covers **336 listed artifacts plus
two seals, 338 actual files**, with exact-root
exclusions only for receipt/manifest. Frozen nested metadata, Source, tools,
consumed headers/libraries, and post-only context recovery are included.
Postprocessing consoles are outside the sealed family root.

| File | SHA-256 |
| --- | --- |
| Source CPP | `e8adeb28a7deeb4b3fbe14c483fc6e0207de3c8347960787eb696a7611c2677d` |
| Source HPP | `f462efb6557acb2d52dbc4bd6c4e439aace97613652db6e82881ea4fd8c40aca` |
| `receipt.json` | `119fda3fd798fc81f3ffddab415993f0c91981383273e453a7fe9fdb87233e4e` |
| `artifact_manifest.json` | `07bf0b77c608f3eacfae41743d844279a0de8b72ec25e96bab69cdb3fdd31b6c` |

This is a current-canonical supplied-storage fixture and physical ordinary ABI
qualification. It does not admit native owning parent/World, private heap,
sentinel, array iterator/EH/class teardown, observers, registration, or gameplay.
Root's separate complementary family and full combined build remain required.

## Historical primary integration

Whole supplied-storage header initializer[004B7EC0,004B7ECD)13B6 literal MOV EAX,ECX/XOR ECX/threezeroDWORDstores/plainRET, sameOriginal/COFF/uniqueLinked13;0calls/relocs. Hypothetical paddedfastcall interface ECXactualfreshunownedwritable12header/unusedEDXregister/noStackargs; EAXroot ECX0 EDX/allnonvols/ESP/ES/DF preserved, XORdefined8C5=44 AFundefinedexcluded. Currentcoherentempty{count,head,tail} layout only, no resettingpopulatedowner. Root complementary3freshTUs/184headers7libs/BSParchives0/oldobjects0 at2Ebase:3actual44buffers with12header+16+16inallocationcanaries; SourceDF1/OriginalDF0/ordinarySourceDF0, newD5/E9/72poisons and GPR/EDXsentinels,3actualalloc/3entries/3actualbasefrees. Original13 isunmodifiedRXcopy. All96capture/full44buffer/header12/guard32 independentlydecoded; EAXfullroot/ECX0/EDXseed/allnonvols/ESP/ES/definedflags/DF PASS. Whole25linkedapplication/canonical/probe/helpercodegates incl2454B682main106relocs, realordinary6B2directCALL/RET andraw172B62, complete14B4cookie/externalfailuretailJMP; physicalI386matchingUCRTmalloc/free/newhandler IAT/export/NTpath/fileID/fullSHA/normalized32code before0alloc/targets andafterfrees, Native/source/headers/libs/tools/input/code/priorbookendsPASS. Exact133listed+seal134,16385olderpins preserved. No oldsuccessfulstage/recipe/helper/process replay. CombinedMainWin32/all3existingchecksPASS. Actualgenuineownerdescriptor/iteratorcallback staticassociation retained, owningparent/iteratorEH/sentinel/privateheap/fullclass/World/game ABI/gameplay remain unadmitted.

Independent seal `3a0957e45617408c9fce00c33892cb8ca55a1fc507104567e66b27ffaacd3f11`.

## Historical standalone fixture helper coverage hold

Root reviewed the independent Header13 audit: all26 retained symbols/24 physical TU bodies and whole literal13B6 constructor are gated, but ten normal external spans are absent: actual43-byte stack helper (five further alignment bytes separately classified) and nine6-byte import thunks. Six cold exception/delete-chain spans are also absent; separate GS failure remains unadmitted. Standalone complete-helper fixture Source admission is reopened to0 pending fresh normal-helper coverage. Three full96-byte captures,44-byte caller buffers and rawSourceDF1/Original and ordinaryDF0 observations remain historical evidence; definedXOR flags mask8C5 excludes undefinedAF. This hold makes no blanket later clear70 or callback family claim. No Source edits, old helper/process replay or new Native/provider queries.

## Current saved-analysis publication

The supported annotation tool retained the prior name/comment, appended current
qualification evidence, and saved `bsp.gpr` / `/battlestationspacific.exe`.
Affected exports were refreshed. Snapshotting correctly skipped the unchanged
64,728-function project, and the index was rebuilt. Prior values are preserved
in `local/ghidra-annotations-20261009T024236Z.json`. The exact raw-storage name
remains provisional.
