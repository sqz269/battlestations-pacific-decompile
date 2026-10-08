# Native scene enum registry scalar deletion

`004D2620` is a complete 30-byte, 11-instruction scalar-delete wrapper on the
admitted registry lifetime domain. Its preserved compiler-derived Ghidra name
is `CG_scalar_deleting_dtor_004d2620`; the Source name is descriptive.
Native bytes SHA-256: `04b3071300c08f35ca5d07df369f29f5a2f846a80f29d28dbde8326903397600`.

Native ECX is the genuine `10Ch` root. The wrapper saves ESI and captures ECX
before calling ordinary `008F4F00`. It then tests only flags bit 0 at `[ESP+8]`,
optionally pushes the captured root and calls `00BF65AC`, executes returning
`ADD ESP,4`, copies ESI to EAX, restores ESI and returns with `RET 4`.
`00BF65AC` jumps into historical CRT `00BF9DC8`; it is not a direct IAT call.
The primary repaired the three-byte listing gap at `004D2635`, restored the
exact body, saved/refreshed exports and preserved the existing name. Workers
did not mutate Ghidra. Both exact native call rows passed the existing verifier.

The new explicit MSVC Win32 Source interface captures `uintptr_t(owner)` before
destruction/free, calls genuine complete `destroy_native_scene_enum_registry_008f4f00`
once, tests `flags & 1u`, optionally calls current `singleton_lifetime_free`
(`std::free`), and returns the captured integer. A deleted result permits no
storage access. Ordinary destruction preserves its first real clear plus
second empty clear. No callbacks or synthetic lifetime providers are introduced.

Admission requires separately allocated compatible current-CRT `10Ch` storage
initialized by only genuine `004D3069..004D308B`, real distinct 14h table and
symbol pools, actual owning raw strings, and unique current-CRT `19Ch` CEnums
from `008F4DD0` or null mapped payloads. The complete actual `008F59C0` payload
scalar, `0041DD20` key release and `0043B0A0` table slot return remain connected.
Original global pool cells/startup and whole constructor are outside this claim.
There is no default/reset/null guard, fake constructor/profile/allocator,
alias/reentry/concurrency/fault policy or historical class-ABI bridge.

One new ignored family is `local/cc11_enum_registry_scalar_delete`, accepted
attempt `run04`. It retains the genuine two-registry installed 22+6 enum case,
fresh 22-symbol reinsert, colliding nonnull-before-null chain and empty key.
Flag 2 retains A after ordinary destruction; a full `10Ch` comparison checks
both phase words and every zero count/head byte before explicit A release.
Flag 257 deletes B; its complete live input and integer address are captured
before the wrapper call, and only returned integer bits are compared afterward.
There is no freed B root read or second free. There is no in-branch pre-free
runtime snapshot: complete Source/COFF call order plus retained A evidence
qualify B's ordinary-before-free schedule. All 50 symbol/five table slots,
131 query-temporary prefix checks and real trim/unlink/string shutdown pass.

All 17 fresh TUs compiled strictly (`/O2 /GL- /EHsc /MD /GS- /W4 /WX
/fp:strict /showIncludes`) and linked an embedded `asInvoker` manifest.
The accepted run uses the exact primary Community Hostx64/x86 compiler/linker
and C1xx/C2 hashes with before/after pins. It consumed 46
project headers, 202 host headers and
16 searched libraries, all sealed from actual logs.
The old b92b4710f-built support libraries were copied/bookended before parallel
main rebuilds; Source packet baseline was `20341af21` (metadata-only successor).
Actual consumed frozen Core SHA-256: `366baab13b62a0639b68f6aed98740e62780d0912971b253edd6e5c47d814aa3`. Lua/zlib hashes,
all 341 earlier consumed pins, 3853 historical
files and 613 attempt files are bookended
in the manifest. The old fixtures were never replayed.

Complete Source wrapper COFF is 46 bytes/16 instructions with exactly two
ordered relocations: ordinary then current free. Its low-bit test follows the
ordinary call, and its post-free return uses captured bits. Three reused
complete clear/ordinary/payload scalar bodies exactly match the primary COFF.
All four full linked bodies match their complete COFF after relocation fields
are normalized and resolve to the intended linked targets. The 17-object
inventory contains 1626 code sections/120271
bytes/3220 ordered relocations. Actual linked Source
coverage is 244 physical code spans/
39975 bytes/
1156 ordered relocations;
membership proves linked identity, not execution of every provider.

Attempt receipts are preserved: run01 stopped before compilation on a pin-helper
Path/string error; run02 passed Source using Insiders but lacked actual backend
pre-pins; run03 stopped before compilation when x86 selected the other compiler
host; run04 pins exact primary x64_x86 tools and passes. No new tracked tests.

Receipt: `J:\PROG\battlestations-pacific-decompile-cc11_enum_registry_scalar\local\cc11_enum_registry_scalar_delete\run04\receipt.json`.
SHA-256: `44ca9c01fffdbc1d2ca3967e0e5c35bbcaa9ec89d3296211b4bf09a1ca4df0da`.
Structured details: `reports/native_enum_registry_scalar_delete_cc11.json`.

Original 30-byte execution, ECX/flags/RET4 class ABI, private FS/EH machinery,
historical CRT heap/free graph, native generic profile dispatch, global namespace
startup, traffic/parser/world/runtime/game remain unbound. Primary CMake/full
Win32 build/existing CTests/integration are pending. Nested `004D0EB0` remains
unimplemented: actual root+4 is an interior 108h map and must never be freed;
standalone deleting allocation lineage is unproved.

## Primary integration

Whole normal Native30B11 through new contextual Source46B16: captureuintptr before genuine ordinary8F4F00, test flags lowbit1 after ordinary, conditional current std::free, return captured bits without deleted-root access. Preserve compiler-derived name; primary repaired returning-free override/gap and full11-instruction body, saved/exported. Independent17 freshTUs against CURRENT built Core plusLua/zlib consume genuine separate10Croots, real14h table/symbol pools/currentCRT19C CEnum and rawstrings. Flags2 retainedA validates whole10C after ordinary then explicitfree;257 deletedB snapshots fullinput BEFORE call and observes returnedbits only afterward. Ordinary-before-free forB is completeSource/COFF/link order evidence, no artificial callback snapshot. Real131prefix/50symbol/fiveTable returns and trim/unlink/string shutdown. CALL instruction offsets18/34 DECIMAL; operands19/35 DECIMAL=13/23hex. Worker four attempts preserved (setup pin error, Insiders backend omission, Hostx86 mismatch, final pinned Hostx64 PASS); primary setup schema correction before compile and inspector offset notation corrections changed no Source or fixture execution. Original30 not executed; original classABI/EH/historicalheap/startup/namespace/world/game unbound. Nested4D0EB0 interior108map deleteflag1 remains UNREADY; never freeowner+4.

Main Source `3ef9afe67` passed the full MSVC Win32 build and all three existing CTests. The independent primary fixture consumed 17 freshly compiled TUs; its receipt is `local/cc11_registry_scalar_current_primary/inputs_after.json`. Its actual consumed headers, toolchain, libraries and historical inputs remained stable. Saved annotations, exports and snapshot follow in the report.
