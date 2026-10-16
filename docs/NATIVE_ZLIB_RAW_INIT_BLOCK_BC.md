# GOAL16AN: complete raw init_block

Only **00BCDE20..00BCDE8D `init_block`**, complete **110B/29 instructions**, receives Native function credit. One genuine returning Source function emits **111B/30 instructions**: same-function `MOV EDX,ECX` followed by every Native operation. There is no private kernel/export, wrapper frame, borrowed ESP, unused formal, opaque byte payload, provider/global stub, guard or snapshot.

| Routine | Coverage | Original input/cleanup | Source entry | Whole emitted extent |
|---|---|---|---|---|
| 00BCDE20 init_block | complete | EDX raw backing; no ECX input or stack words; plain RET | void fastcall(backingECX), then MOV EDX,ECX | 111B/30ins, one CODE section/function, zero relocations/undefineds |

```cpp
extern "C" void __fastcall
bsp_native_zlib_init_block_00bcde20(void* backing) noexcept;
```

The one Source input is genuine. Original consumes EDX; Source transports ECX into EDX without memory, ESP or arithmetic-flag effects. This is a new interface, not recovered Original formal pointer/class/return/fastcall compatibility. The old Ghidra two-formal rendering does not establish a consumed ECX input. Correct `init_block`, Original prototype/comments and no_return=false are retained; the worker performs no Ghidra mutation.

The complete body first PUSHes incoming ESI into actual S-4, computes EAX=backing+8Ch, loads ECX286 and XORs ESI0. The actual MOV EDI,EDI remains. It performs286 WORD-zero stores at stride4, ADD EAX4/SUB ECX1/JNZ after each; last offset500h. It then performs30 such stores from+980h through+9F4h. Finally it computes+A74h, loads19, retains LEA ESP,[ESP], and performs19 stores through+ABCh. Those are335 WORD-zero writes on an unfaulted ordinary traversal, without widening SI, bulk clearing, alignment/range/null guards or replacement loop arithmetic.

Four separate DWORD stores follow in exact order: zero at+16A4h,+16A0h,+16A8h,+1698h. Only then store WORD1 at+48Ch, POP current actual S-4 into ESI and plain RET. All32-bit backing/address arithmetic wraps; partial and unaligned writes, actual save/return/data/control aliases and their access/fault order remain real. No detached saved ESI or repaired return is substituted.

On an ordinary completed path EDX remains backing, EAX wraps backing+AC0h, ECX0 and ESI is the actual current saved word. EBX/EBP are untouched; MOV EDI,EDI retains EDI. Final arithmetic flags come from the third-loop SUB1-1; later stores/POP/RET preserve them. Other CPU/DF/FP state is not normalized. These are static instruction effects, not a C++ logical EAX result, universal nonvolatile/safe-return promise or runtime measurement.

The added register/fetch prefix changes the Source entry contract and incidental ECX context before Native's overwrite. Code/fetch/return addresses and current-frame placement differ. Arbitrary code/control/saved-frame aliases, measured faults, SEH/unwind, full Original ABI/CPU environment, concurrency, actual caller adoption and gameplay remain unproved. The whole raw input domain is not replaced by a disjointness or forced-success assumption.

Both incoming sites were checked. `_tr_init` atBCF12B moves EDX,ECX and BCF12D is a real E9 tail-JMP to this leaf, with no extra call frame. `_tr_flush_block` captures entry ECX in ESI and atBCF8ED moves current ESI into EDX before its E8 CALL atBCF8EF; this leaf consumes no stack argument and returns with plain RET. Ghidra xrefs label both UNCONDITIONAL_CALL, so actual bytes distinguish the tail transfer. Neither parent is reconstructed or adopted by this packet.

Fresh typed metadata6926 verifies exactly one110B range/no thunk/no-return=false at `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, base00400000. Every byte matches fresh Ghidra and installed PE. All29 operations and3 loop targets map to Source. Native LEA ESP,[ESP] is4B `8D642400`; readable Source is3B `8D2424` with the same effective operation. Prefix `8BD1` adds2B/1op, hence111B/30ins. No encoding is forced, suffix omitted, alignment hidden or Native-size prefix compared. Byte identity is not a universal Source gate.

The entire first strict791B and actual registered823B COFF files are audited: four sections, one offset0 public function occupying all111B CODE, no relocations/undefined helpers/globals; full directives, CodeView OBJNAME/COMPILE3, four section auxiliaries, checksum metadata, symbols/strings and padding. Strict17/MD/O2/W4/WX/fpstrict passed first attempt; registered code is identical. The actual project has one Source item and C++17; its exact keyed compiler command, header read, grouped write and Source/object item records are retained. No unrelated command excerpt is substituted.

Actual Core94379292B has2494 complete real COFF members and both complete83034-symbol dictionaries uniquely bind the whole823B member at20145576. All current members/selected definitions are accounted. The genuinely warm tree was clean and fast-forwarded from822facbc1 to published0f57e25ee, spanning221 Source/header/registration paths. Relative to its old2394-member Core:2355 payloads unchanged,39 changed (31whole CODE-identical/unchanged CPP;8corresponding changed Main CPPs),100new (AN1, Main99),0removed. No false all-prior-object-unchanged or extra AN Native-credit claim is made. Full attribution and old/current archive proofs remain in the frozen packet; old seals/caches are not recursively copied or rehashed.

One normal `scripts/build.ps1` run passed both existing checks in101.135124s, including CTest8.25s. The existing unrelated LNK4006 spawn_request_id_matches warning remained. No new tests/probes, selected Source/Native execution or game deployment occurred. Two static-helper corrections (typed modification-number string conversion; grouped CL.write key) are retained separately from Source/compiler results. Source/header stayed unchanged after first compile.

The actual stock private init_block118B/31ins is complete algorithm Source, but uses a different cdecl EBP frame/register/loop/fault schedule and WORD1 placement. It is not direct full raw-body reuse; neither it nor the prefix adds Native credit. AM readiness and its Root acceptance are referenced, without copying old review trees. `_tr_init`98B still needs genuine persistent descriptor binding (known60B descriptors plus1584B rooted tables); `_tr_tally` has no identified Native entry in this bounded stream and receives no credit.

Root alone handles later annotations/integration/publication, retaining correct names, old Original metadata and all ABI/fault/parent qualifications. `reports/native_zlib_raw_init_block_bc.json` and ignored `local/goal16an_init_block` hold full hashes/operands/archive/actual build provenance. Final pin and inclusive setup/build/correction/docs/metadata/commit/release/seal time are delivered by the frozen handoff; no speedup claim.
