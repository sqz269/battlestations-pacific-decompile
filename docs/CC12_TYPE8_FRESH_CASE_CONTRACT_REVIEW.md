# Type8 fresh-case contract worksheet

This is a Source/TEXT expectation worksheet, not process observations or validation. It adds Source credit0 and does not inspect Root's active `local/t8p3` or any accepted fixture directory. Four current Source/header files and the two explicitly selected text files are frozen and checked before/after. The exact probe SHA is `4b50d1f627bfcc19a9cc2a1cd03c6d6b519342bf848dce18e5acae26a93f08d4`; recipe SHA is `96aa02347e03b5cbafbbaf8d766ba613d3656eece578d5b8659ec50e109247ed`.

The actual current constructor is `construct_native_scene_property_record_type8_byte_array_storage_008ef2f0`. ECX receives fresh/unowned writable56 storage; EDX is an unused padding formal. Stack entryT+4/+8/+C are DATA, uint32 byte COUNT, and FLAG, with RET12/EAXroot. Count has no scaling. Only the unsigned low flag byte selects the branch, without Boolean normalization. Require positive stable readable input, disjoint nonwrapping root/input/frame and DF0 for successful actual current-provider calls. Failed allocation/copy may leave partial storage; the worksheet provides no failure/unwind cleanup. Evidence: CPP11–67, HPP16–44; canonical allocation/free Source51–66.

| Raw case | Input | Exact COUNT-byte sample | COUNT | FLAG / low byte | Root poison | Expected pointer+20 |
| --- | --- | --- | --- | --- | --- | --- |
| Source_retained | 0 | `63D1008B2FA4E7` | 7 | `D6B23900` / `00` | `56` | exact DATA pointer |
| Original_retained | 1 | `AE257900D364B80F42ED00` | 11 | `7C48E100` / `00` | `8C` | exact DATA pointer |
| Source_copy | 1 | `AE257900D364B80F42ED00` | 11 | `A53D6C80` / `80` | `2B` | new COUNT-byte child |
| Original_copy | 0 | `63D1008B2FA4E7` | 7 | `2E917F02` / `02` | `91` | new COUNT-byte child |

Both copied samples include embedded NUL, and the full11/7 bytes must be compared; they are not strings. Input0's full16 bytes are `63D1008B2FA4E719B6004CF29538DA70`; input1 is `AE257900D364B80F42ED009731CA568D`. Each `GuardedBytes` is48 bytes: preDWORDs at0..15, input16 at16..31, postDWORDs32..47. Pre guards are `9C710000+i*100+j`, post guards `638E0000+i*100+j` (hex arithmetic, i=input index,j=0..3). All48 bytes of both objects must remain unchanged after every target. These are guards inside actual local input objects, not adjacent heap red zones or metadata.

All roots begin as56 poison bytes. Source writes29 bytes in half-open ranges `[00,08),[18,28),[2C,2D),[34,38)`, preserving27 in `[08,18),[28,2C),[2D,34)`. The phaseDWORD00 is literalCE89D4, tag04 is8, DWORD18/1C are0, pointer20 is actual DATA/child, DWORD24 is COUNT, byte2C is1, and DWORD34 is0. OwnerDWORD30 remains poison, as do2D..2F; no completeDWORD marker1 or owner initialization is inferred. Pointer fields use actual live32-bit addresses, not invented fixed addresses. Flag is read before stores; phase/tag precede count read, and copied child20 publication precedes exactCOUNT-byte memcpy. The JSON provides all56-offset write/preserve masks and case-specific complete root templates.

`Capture` is140 bytes: pre fourDWORDs0..15, `Registers`108 bytes16..123, post fourDWORDs124..139. All fields are little-endian32-bit. The following offsets are decimal:

| Register field | Capture offset | Registers offset |
| --- | --- | --- |
| esp_before | 16 | 0 |
| esp_after | 20 | 4 |
| eax_after | 24 | 8 |
| ecx_after | 28 | 12 |
| edx_after | 32 | 16 |
| ebx_before | 36 | 20 |
| esi_before | 40 | 24 |
| edi_before | 44 | 28 |
| ebp_before | 48 | 32 |
| ebx_after | 52 | 36 |
| esi_after | 56 | 40 |
| edi_after | 60 | 44 |
| ebp_after | 64 | 48 |
| flags_before | 68 | 52 |
| flags_after | 72 | 56 |
| es_before | 76 | 60 |
| es_after | 80 | 64 |
| stack_guard0 | 84 | 68 |
| stack_guard1 | 88 | 72 |
| data_slot | 92 | 76 |
| data_word | 96 | 80 |
| count_slot | 100 | 84 |
| count_word | 104 | 88 |
| flag_slot | 108 | 92 |
| flag_word | 112 | 96 |
| esp_before_call | 116 | 100 |
| entry_esp | 120 | 104 |

Capture initial fill is4B. For case indexi, tag=(i+1)*170 hex; preDWORDs areD8A50000+tag+j and postDWORDs275A0000+tag+j. Other live captures are compared in full. EBX seed isD93A61E7, EDI seedB74C28F5; ESI is the actual Capture address and EBP the actual raw frameP. Incoming EDX poisonE6A9523B is unused by the constructor. Stack canary0 is78C43A6F and canary1D2B795E1, inside the caller frame. EAX must equal the full actual root address; EBX/ESI/EDI/EBP must match before/after. For borrowing, ECX remains root and EDX becomes DATA; copied provider residuals ECX/EDX are recorded without value assertions. No invented selector/FPU/MXCSR values are expected.

The raw text definesR as ESP before target arguments, equalP-20. It pushes FLAG,COUNT,DATA, givingQ=R-12. Immediately beforeCALL it recordsQ and calculatesT=Q-4 withLEA; T is not sampled inside the callee. Root must review actual complete emitted caller bytes to establish that physical entry. RET12 restoresR=T+16. Dead DATA/COUNT/FLAG slots areT+4/+8/+C (R-12/-8/-4), read beforePUSHFD can overwrite them; full opaque flag upper bytes must match. RawCLD makes entryDF0; selected assertions require afterDF0 for the actual current bindings. This is not a blanket provider/historicalCRT preservation promise.

Borrowed flags are fixed: `EFLAGS_after & 8D5 == 44`, including definedAF0 fromCMP0,0. Copied flags come from actual finalADD: allocator argument remainsT-12, memcpy's three arguments reachT-24 and its entryT-28; after memcpy returnsT-24, ADD16 producesT-8. Compute six defined flags from unsigned32 `a=T-24,b=16,result=a+b`, includingAF; do not substituteR/Q or assume an observed fixed04. MOV/LEA/capture instructions beforePUSHFD do not change arithmetic flags. Exact bit formulas and offsets are in the JSON.

The future successful raw process has four target entries only: Source retained, Original retained, Source copy, Original copy. Allocate four actual56 canonical roots first; copying adds child11 then child7. Compare every complete root, both input48 objects, other live captures and all live children after each target. Save root56/capture140/input48/child11+7 artifacts while live. Free Source child, Original child, then roots0..3 once each: six canonical allocations/frees. Borrowed inputs are never child-freed. Original112 RW-to-RX/VirtualFree is a separate OS allocation. After frees, only saved address bits/captures and still-live code/module evidence may be read. Both ordinary wrappers are retained/code-only, with no ordinary dynamic target entry.

No case-value, byte-count, branch or root-mask disagreement was found. The selected text has these review limits:

- ES before/after is serialized but `machine_ok` does not assert equality. Borrowed Source has noES update; copied Source makes no blanket providerES preservation claim. This is not a machine_ok ES equality gate; an independent recorded decoder may compare ES separately and is uninspected here.
- `machine_ok` additionally requires DATA>=EBP+32 and root disjointT..EBP+32. That numeric ordering is a probe placement predicate, beyond Source's general disjointness requirement; actual compiledMain/frame addresses need review.
- Recipe ordinary checks oneCALL/plainRET and excludes selected numeric/MOVZX opcodes. Those checks alone do not prove receiverECX or DATA/COUNT/FLAG transport; full fresh ordinary/raw/Main machine bytes still need RootABI review.
- T is separately calculated beforeCALL, not a callee-entry sample. Copy afterDF0 and actual provider residuals remain predicates to check, not results observed by this worksheet.

Root must author actual final generation/review bindings and gate full fresh TUs/Main/callers/providers/helpers, only two actual constructorCALL operands, unique linked112 and reboundOriginal112 before zero heap allocations/targets and after six frees. The selected recipe requires `prelaunch_review.json`/`accepted_for_single_process`, one accepted process, actual EXE/gate/inventory/objects and exact guardAST identities. Actual I386 malloc/free/_callnewh and memcpy IAT/export/MEM_IMAGE/NTphysicalfile identity/fullSHA/ASLRnormalized code bookends remain unperformed here. No text assertion proves future linked code coverage or actual provider identity. If Root changes the generated probe/cases, those changes require its review; this worksheet does not assert that active family matches the pinned text.

Baseline is `f3c2484d9db7e0b687324750e795bc3f5b478684`, Main generation `3e3b4c1a6bd6c4957553287ee093477ee48b8c79`. Exact Source/TEXT pins, line evidence, all guards, offset tables and expected allocation schedule are in the matching JSON. `local/t8cases/receipt.json` binds its recursive `artifact_manifest.json`; only those two exact root files are excluded. Every utility/log/frozen input/commit artifact is included, and post-seal consoles remain outside. No old helper/reader import or execution, Source change, Native/Ghidra/provider/compiler/target operation, class/clone/privateCRT/EH/World/game admission or new test occurred.
