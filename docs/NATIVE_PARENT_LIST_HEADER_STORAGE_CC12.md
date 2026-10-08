# Native parent list header storage (CC12)

Implemented whole `[004B 7EC0, 004B 7ECD)` as the literal **13-byte / 6-instruction**
MSVC Win32 naked function `initialize_native_parent_list_header_storage_004b7ec0`.
The name is a hypothesis. Worker Source admission credit is **0 pending Root's
independent family, combined-main build, annotation, and integration**.

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

## Fresh connected validation

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
