# Qualified native type-8 byte-array storage

Worker implementation for `cc12_scene_property_record_type8_byte_array_storage`.
The complete `[008EF2F0,008EF360)` constructor is reconstructed as an MSVC Win32
raw-storage API. Its 112 bytes and 34 instructions include both branches. Only
the two CALL operands bind genuine current allocation and copying providers;
the other 104 bytes match the installed Original exactly.

Worker Source credit remains **0 pending Root's independent validation,
main-build registration, metadata/Ghidra publication and integration**. This
fixture proves a qualified successful current-provider domain. It does not
establish private Original CRT/EH, class ownership, a drop-in class replacement,
World behavior, startup or gameplay.

## Native evidence and interface

Native SHA256:
`4c3786af3a642703dc38315ae63d9dee0468552f47a03bc1405fc0e90ab5c928`.
The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Before and after the accepted process, the batch verified the configured
`C:/Users/sqz269/bsp.gpr`, actual `/battlestationspacific.exe`, installed PE,
all 112 bytes and all 34 live listing instruction starts. No Ghidra mutation
was made. The earlier remaining-readiness audit supplies the one genuine
clone caller and bounded dependency graph; this packet does not re-expand it.

`construct_native_scene_property_record_type8_byte_array_storage_008ef2f0`
uses `void* __fastcall` with root in ECX, an unused EDX padding argument, then
actual byte pointer, positive byte count and full flag DWORD on the stack.
The three stack words are at entry `T+4`, `T+8`, `T+C`; both branches return
root in EAX with `RET0C`. The descriptive name is a hypothesis.

Supply fresh, unowned writable 56-byte root storage and a valid stable readable
positive-length byte span. Root, input and active call frame are disjoint and
ranges do not wrap. Require DF clear for the genuine current CRT. The API
does not scan for NUL and makes no `noexcept` promise.

The initial `CMP byte [T+C],0` precedes all root writes. Only that low byte
selects the branch; the upper 24 bits do not matter. Phase/tag stores precede
the byte-count load. The constructor writes 29 bytes and preserves 27:

| Root range | Effect |
| --- | --- |
| `00..07` | Phase token `00CE89D4`, tag 8 |
| `08..17` | Preserved |
| `18..1F` | Zero |
| `20..23` | Actual borrowed or newly allocated pointer |
| `24..27` | Exact input byte count |
| `28..2B` | Preserved |
| `2C` | Byte 1 in both branches |
| `2D..33` | Preserved, including the DWORD at `+30` |
| `34..37` | Zero |

For a zero low flag byte, `+20` retains the exact input pointer. There is no
allocation or provider call. The input remains caller-managed while that
pointer is used; it must not be freed as a newly allocated child. ECX returns
root and EDX input. CMP defines all six arithmetic flags: `EFLAGS & 8D5 = 44`,
including AF zero.

For a nonzero low byte, Original CALL `008EF31E` uses a genuine private CDECL
size adapter that forwards `object, native_bytes=n, host_bytes=n` to the
unchanged canonical allocator. The adapter earns no native reconstruction
credit. The actual returned allocation is stored at `+20` before Original
CALL `008EF32D` copies exactly `n` bytes through genuine `VCRUNTIME140 memcpy`.
Observe the owned child while live, free it exactly once through the matched
canonical free, then dispose the root separately. Byte `+2C=1` in both branches
does not establish a class ownership rule.

The allocation argument remains at `T-12`; memcpy enters at `T-28`. After
memcpy returns, the final `ADD ESP,10h` uses `(T-24)+16=T-8` and defines all six
arithmetic flags. Copied ECX/EDX are actual provider residuals and are recorded
without guessed assertions. ESI/EDI are saved/restored, and EBX/EBP follow
the providers' ordinary ABI. No blanket ES/DF/FPU/MXCSR preservation through
providers is claimed. Initial stores can survive a provider failure; zero
size, failure, invalid spans, aliasing, reentry and unwind are excluded.

## Fresh build and static gates

The accepted ignored family is `local/t8b`, based on private worktree baseline
`b26d9e9900d20e4a9df4c29683e1275336b5820d`, descended from Root registration
`f2dc5f4352e0b5e1905bb097dbd0aaf5b16d773a`. Three fresh translation units build
the constructor/adapter, unchanged canonical allocation/free, and new probe.
No BSP archive, prior object or prior process is reused. The x86 MSVC 14.51
build uses `/O2 /MD /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL-`, Source
`/Oi-`, and an embedded as-invoker manifest. The probe has a safe filename,
fixed preferred image base `25000000` with `/DYNAMICBASE:NO`; imported provider
ASLR is resolved from the actual mapped modules.

Thirteen whole spans pass COFF-to-linked relocation and byte gates: 112-byte
Source, 61-byte size adapter, 90-byte canonical allocator, 6-byte matched free,
both ordinary ABI callers, 191-byte raw caller, 24-byte bad_alloc constructor,
6083-byte main with 192 individually resolved relocations, complete 14-byte
normal cookie helper, memcpy import thunk, cold throw import thunk, and the
48-byte map-owned stack helper. The named cookie-failure tail and private EH
are outside execution scope. The unique linked constructor masks only
`[47,51)` and `[62,66)`. Ordinary callers are static ABI checks, not extra
target entries. Main's critical call, comparison, ownership and cleanup paths
were reviewed before the first entry. The exact required
`manual_static_review.json` / `passed` contract was checked before compilation.

## Sole accepted fixture

One accepted process executes Source and bound Original once per branch:

| Case | Count | Full flag DWORD | Stored pointer | Flags `& 8D5` |
| --- | ---: | --- | --- | --- |
| Source retained | 9 | `B931CA00` | Exact input | `044` |
| Original retained | 13 | `7C5EE100` | Exact input | `044` |
| Source copied | 13 | `D746A501` | New owned allocation | `004` |
| Original copied | 9 | `29BC0080` | New owned allocation | `004` |

All calls use real objects. Four fresh 56-byte roots and two positive-size
children produce six allocations and six matching frees. The two guarded
16-byte payloads contain embedded NULs and non-ASCII bytes. Their full 96
guarded bytes remain unchanged. Full root bytes, every other live capture and
every live child are checked after each call. Child buffers are disjoint from
roots, inputs and capture storage. Children are observed and saved before
their two frees; the four roots are then freed. Retained inputs are never
freed as copied children.

The raw caller saves 27 register/stack DWORDs inside a guarded 140-byte capture.
It independently records `R=001891EC` before pushes, `Q=001891E0` immediately
before CALL and `T=001891DC` at target entry. Return ESP equals R. All three
dead argument words are captured before PUSHFD. EAX, nonvolatiles, stack and
capture guards pass. Copied flags derive from `001891C4 + 10 = 001891D4`,
giving `004`; the retained CMP gives `044`. Copied residuals were ECX zero,
EDX `916D00E4` and `A95600D3`, with no assertion on those residual values.
An offline decode independently rechecked all four captures, full roots,
guarded inputs, 13/9-byte children and the bound Original's exact 104 literal
bytes without launching another target process.

The bound Original is a fresh 112-byte RW-to-RX allocation. Its only changes
are the two CALL operands, targeting the same fresh size adapter and actual
copy import as Source. The 13 code spans and all four actual provider exports
are attested before zero entries and after all frees. Heap functions resolve
the same mapped I386 `SysWOW64/ucrtbase.dll`; memcpy resolves the distinct
mapped I386 `SysWOW64/vcruntime140.dll`. File identity, NT path, full-file
SHA256, IAT/export identity and normalized 32-byte export prefixes agree.

## Evidence preservation and remaining integration

The first `local/t8a` attempt preserves a draft generator SyntaxError and a
probe compile error `C3861: open_module` (the genuine helper is `verify_module`).
It had no linked probe or target execution. Its 300 artifacts plus seal remain
immutable. The corrected `t8b` family builds all three units afresh; preparation,
build, static gate, sole launch and post each pass once. A read-only review
command initially used `whole9.asm`; the actual `whole_9.asm` was then read.
That path typo caused no file change or process execution.

Terminal post verifies all 50 strict inputs, 184 consumed headers (including
extensionless files), seven consumed libraries and 5239 prior artifacts:
4640 older pins, 298 remaining-readiness artifacts and the failed family's
301. Ten unconsumed metadata snapshots are frozen copies; moving main CMake
is not incorrectly treated as a consumed strict input. The accepted family
contains 382 artifacts plus its seal, SHA256
`5fe68aab22202b981eb5b30fa3e8bacd229cdba770bdbefb37a3b53782bf1aa5`.
No accepted stage, old recipe or prior target process was replayed.

The machine-readable report links the complete artifact inventory, command
outputs, COFF/link maps, native bookends, manual review, capture decode and
physical provider evidence. This worker changes only header, implementation,
this document and its report. Root owns independent validation, CMake/main
build, shared metadata and Ghidra publication. No startup or gameplay claim
follows from the fixture.

A post-seal read-only report-verifier draft had an `IndentationError` before
execution. Its separate receipt is linked by the report; a new complete
verifier checks public references. The sealed families and target process
were not changed or replayed.

## Primary integration

Whole [008EF2F0,008EF360)112B34 raw byte-array record storage; 104 literal bytes, only CALL operands[47,51) real size-to-currentcanonical allocation adapter and[62,66) actual VCRUNTIME memcpy rebound. Fullfast ECX actualfresh56/unusedEDX/threeactualstackDWORDs input,count,fullflag/RET12/EAXroot. Initial CMP BYTE lowflag beforestores, borrow preserves actualpointer/nochildfree and CMP8D5=44 includingAF0; copy stores genuinechild+20 before actualmemcpy, final ADD actualT-24+16 derives all6definedflags8D5. Both paths +2C1 is not an ownership discriminator. Qualified positive actual byte-span/fresh/disjoint/nonwrapping domain; embedded NUL treated as bytes. Root complementary3TU oneprocess4entries/6realallocations-frees, four140captures/all56roots/96guardedinputbytes/full16+11copies independently decoded; Rpreargs,QpreCALL,Tentry separated, three deadargument slots beforePUSHFD/nonvols/ESP/DF0 verified. Full13 codegates including6083B1549 main,191B68raw,cookie14B4/failuretailJMP,chkstk48B24; actualI386UCRT heap and distinctVCRT memcpy IAT/export/MEM_IMAGE/NTpath/fileID/fullSHA/normalizedcode beforezeroentries/after. Exact380listed+seal381/17427priorpins stable; currentcombinedMainWin32/all3checksPASS. No oldacceptedhelpers/stages/objects/process replay. Original privateheap/CRT/EH/failure/owner/class/fullclone/World/gameABI/gameplay remain unadmitted.

Independent seal `686c226f85185bd77f5bf4bc3e56037a63c99d9c07cde367aa4539fd7cddbf20`.
