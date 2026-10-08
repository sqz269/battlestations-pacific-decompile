# Native raw list payload removal, CC12

`remove_native_unit_list_payload_004845a0` reconstructs the whole ordinary
`[004845A0,004845CE)` body: **46 bytes, 19 instructions, one CALL**. Its live
descriptive name, `BSP_UnitList_Remove`, remains a naming hypothesis. Installed
PE and live Ghidra whole bytes match SHA256
`021c3de90775c6eba0451ba26f707b87d2c72cc853246f4c0b784c2a411303db`:

```text
8b410485c0568b742408741c8d642400397008740d8b400485c075f48bc65ec2040050e808f2ffff8bc65ec20400
```

The Source emits every native instruction, including the four-byte NOP. Only
the genuine CALL operand at offsets `[36,40)` is relocated to the accepted
`erase_native_unit_list_004837d0`. All other 42 bytes are invariant. There is
no dispatcher, callback, provider shim, new guard or invented owning type.

## Physical interface and behavior

```cpp
void* __fastcall remove_native_unit_list_payload_004845a0(
    void* actual_list_ecx, void* unused_edx, void* borrowed_payload) noexcept;
```

ECX supplies a stable coherent 12-byte root `{count, head, tail}` at offsets
0, 4 and 8. Incoming EDX is unused. The explicit second formal places one
full borrowed-payload DWORD at entry ESP+4; after PUSH ESI it is read from
ESP+8. Both physical returns are `RET 4`, at native 004845BF and 004845CB.
ESI is saved/restored. The ordinary accepted erase/free ABI preserves the
remaining nonvolatile registers. EAX always returns the requested full
payload identity, including empty and absent paths and the matched path
after node free; the erase successor return is discarded.

The scan follows head and then node+4, comparing node+8 to the payload. Only
the first matching node is erased. Admitted nodes are actual coherent
12-byte `{prev, next, payload}` allocations produced by accepted append
Source using the unchanged canonical current allocator, and consumed by
accepted erase Source in the same current free domain. Payloads are borrowed
live addresses; this body neither dereferences nor destroys/frees them.
The supplied root is caller-owned storage; Source does not allocate or free it.

Empty/absent paths retain incoming ECX/EDX and final TEST zero defines CF0,
PF1, ZF1, SF0, OF0; AF is undefined and excluded (`mask 0x8C5 = 0x44`).
Matched ECX/EDX remain volatile across real current free. Match arithmetic
flags come from erase's final ADD ESP,4, derived from the actual nested raw
caller: T is ESP after PUSH payload; remove entry T-4, saved ESI T-8, pushed
node T-12, erase entry T-16, erase saved ESI T-20, free argument/return T-24,
final ADD result T-20, and both RET4 epilogues restore raw ESP to T+4.
The matched mask `0x8D5` includes ADD-defined AF. Ordinary DF0 is required
for the real CRT. No blanket FP preservation across CRT is asserted.

## One new connected family

The ignored family is `local/rm46/r02`. Five fresh TUs compile this Source,
unchanged accepted erase, unchanged accepted append with its actual size
adapter, unchanged `singleton_lifetime.cpp`, and a new probe. BSP archives,
old objects and new tracked tests are all zero. Compilation used Win32
Visual Studio 18 Community 14.51.36231, SDK 10.0.26100.0, `/MD /O2 /W4 /WX
/fp:strict /permissive- /EHsc /Gy /GL-`, and an embedded `asInvoker` manifest.
There are 186 exact consumed header pins (10 project headers) and seven
resolved system-library pins, with compiler/backend/linker/SDK tools pinned.

Before any new producer/target entry, whole COFF and unique linked code
proved 46B/19 and the sole REL32 to actual accepted erase. Complete erase
79B/31, append 87B/36, adapter 61B/17, canonical allocator 90B/34 and free
6B/1, ordinary removal caller 18B/5, ordinary append caller 16B/5, raw capture
140B/52, bad_alloc constructor and the actual cookie helper were captured and
reviewed. The compiler caller traces prove original full-DWORD payload flow,
receiver ECX, EDX padding and stack balance; the raw caller measures T at its
actual PUSH payload and captures all GPRs/flags before altering them.

The linked cookie helper is the complete 14B/4 local function at 23003AA0,
including its successful RET and tail JMP to the distinctly mapped
`___report_gsfailure`. The external failure body and intervening alignment/
other functions are excluded. Stopped `r01` compiled/linked successfully but
its old distance-based helper parser crossed function ownership and failed
the contiguity gate; it performed zero producer/target entries or node
allocations. All 83 stopped-stage files remain unchanged, with no replay.

Actual malloc, free and _callnewh IATs equal GetProcAddress and the corresponding
export RVAs in the loaded I386 UCRT. MEM_IMAGE ownership, module identity,
mapped NT path, physical file handle identity, full physical SHA256 and
relocation-adjusted export prefixes were checked before zero producer/target
entries and again afterward. The logical module name is System32; the actual
mapped/physical file is SysWOW64/ucrtbase.dll, SHA256
`60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`.
This is physical identity evidence, with no guessed path normalization.

The sole fresh process executes Original46 and Source on the same caller
root and live payload identities, with separate fresh A,B,A,C lists produced
by accepted append. Each phase covers empty and absent, B middle, C tail,
first A head and remaining A singleton. One additional ordinary compiled
Source caller removes a genuine singleton. All nine actual node allocations
receive nine accepted erase dispositions; total removal entries are 13
(Original6, Source7), with 12 raw captures and one ordinary caller.
Full requested EAX, nonvolatiles/ESI, ESP/RET4, defined flag masks, DF0,
surviving count/head/tail/links/payload identities, other live root, borrowed
payload bytes and external caller-storage/capture canaries pass. Removed
nodes are excluded from subsequent reads. Root-adjacent caller storage is
checked; adjacent heap red zones and allocator metadata are not read or claimed.
Complete target/provider/caller code, Original bound bytes and UCRT bookends
match. No Original append or Original erase execution occurs: Original46
changes only its four real CALL-operand bytes to the same Source erase.

Postprocessing confirms 8,901 unique prior artifacts unchanged, including
all frozen historical families and the stopped r01 files. Old original-path
metadata associations stay historical and unconsumed; no accepted receipt
is repinned to current main metadata. Every current consumed Source/header/
tool/library, object, executable, gate and PE/live body is stable before/after.

The accepted r02 seal is immutable: 113 listed artifacts plus the seal.
The complete append-only handoff inventories 202 actual artifacts plus its
two exact-path-excluded seals, including the unchanged initial r02 seal:

| Artifact | SHA256 |
|---|---|
| `local/rm46/r02/seal.json` | `7dfd6992be265a15a95d5a60547decc1ce90d863cffaaf5147bc31c5d156f143` |
| `local/rm46/handoff/receipt.json` | `93b3832df798051bffab69f9ac1e29d664677fa30fb3be5e6ad6b0cc60e50899` |
| `local/rm46/handoff/artifact_manifest.json` | `145f7c7e7589ce52c82fa3a8dc6e4f1883ab705416fa965d33125c51fdab084a` |

The recipe, probe, static/caller gates, runtime observations, post receipt,
tools and complete file inventory are bound by these seals. Historical
readiness is separately preserved in the registered packet's report.

## Qualification and integration boundary

This admits one whole raw removal Source body in the established current
canonical producer/erase domain. Original private allocator/CRT, native
class/node/root census, parent ownership, destructor or payload ownership,
failure/new-handler/EH execution, world/game runtime and gameplay remain
unbound. The 36B parent witness at 00928570 calls erase directly and is not
promoted to a Source caller or class ownership proof. The new process is a
controlled native fixture, not game execution. Root owns the separate fresh
complementary family, shared build wiring, full Win32 build/all three checks,
Ghidra annotation/save/exports/index and publication.
