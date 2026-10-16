# GOAL16AE implementation of the GOAL16AB gen_codes design

Only **00BCF140..00BCF1B5 gen_codes__prov**, complete **118B/44 instructions/five conditional branches/zero CALLs**, receives Native credit. The historical approximate128B inventory remains preserved; it is not the typed extent. Two real Source functions emit **140B/53 instructions**: complete private kernel118B/44ins and public returning adapter22B/9ins. Adapter Native credit is zero. Strict and actual registered Win32 kernel bytes equal the entire Native body, SHA256 `71f8deb54929824116602b9c569c4fd3541fa5051932e8ba6dc434c7d6e01b94`. This is evidence for the current literal artifact, not a universal byte-identity requirement or Native-size-prefix shortcut.

The MAX AB design was read in full193lines. Its seals are HANDOFF `4da5ba87e5609fa6c405a456da5065e9a26d46dd779f2a9e30e287f7c9549fc0`, DESIGN `b29f8d023b9b567eeb8ffb9fb1cb79414139af8959251f5e0222c73852156801`, report `ac64e7b7d6ec9e2490bc4ebbd65785eb4f92f2a9d17fb8573757621b89fc0f19`, receipt `2f4f7464a64c8898b5c349304cf55595c20f119a9f27a37465b840747429558e`. All32 declared records480,612B plus the additional receipt were verified/copied. Fresh author typed/live/installed-PE evidence independently verifies the complete118B range in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, I386/base00400000, no thunk, entry flow overrideNONE and **Original no_return=false**. Correct provisional `gen_codes__prov`, old prototype and comments remain unchanged; no Ghidra write occurred.

## Returning Source entry and real frame

```cpp
extern "C" void __cdecl bsp_native_zlib_gen_codes_00bcf140(
    const void* counts, std::int32_t max_code, void* tree) noexcept;
```

The three real argument words select counts, all signed max_code bits and raw writable tree. The public naked entry executes exactly PUSH EBX; PUSH EDI; MOV EDX,[ESP+Ch]; MOV EBX,[ESP+10h]; MOV EDI,[ESP+14h]; CALL defined private kernel; POP EDI; POP EBX; plain RET. At public entryT, the real caller return is[T], arguments[T+4/T+8/T+C]. The real CALL gives private entryS=T-C and an actual Source continuation. No borrowed ESP, noreturn, unused ECX formal parameter, copied scratch/frame, saved continuation, fake provider/global, guard or forced restore exists.

Original/private physical inputs are **EDX raw counts, EBX signed max_code, EDI raw tree**, zero actual stack arguments, one plain RET normally giving S+4. EAX/ECX are overwritten before consumption; ESI and conditional EBP preimages are actual saved CPU state, not extra inputs. Private `void __cdecl gen_codes_body()` describes no stack arguments/plain RET, not a recovered custom-register ABI; only the explicit assembly CALL enters it. Source const-void/int32/void/noexcept declarations are choices, not formal Original class/layout/version/return recovery.

If the real inner return still reaches the CALL continuation, private RET leavesT-8, public POP EDI/POP EBX reachT and outer RET reachesT+4. The cdecl caller owns all three argument words; there is no RET0Ch. The public prefix/suffix do not change arithmetic flags. They add real saves, argument reads, CALL/continuation, POPs/RET and corresponding fault/code/current-frame effects. Normal well-formed return is useful new Source behavior; arbitrary control aliases are not forced to return or preserve nonvolatiles safely.

## Whole kernel and SS-default storage

All effective-address arithmetic wraps modulo32. Tree fields are observed WORD[tree+4*node+2] and WORD store[tree+4*node], not a concrete owner or extent. No input cap, alignment/bounds/null guard, snapshot or disjointness premise is introduced.

1. SUB ESP,20h; PUSH ESI; LEA ESI,[ESP+6]; XOR ECX0; MOV EAX1; SUB EDX,ESI. The actual32 scratch bytes atS-20h..S-1 are not blanket-initialized; actual incoming ESI is saved atS-24h.
2. For actual EAX1..15, LEA ESI,[EDX+2*EAX], read only **SI** from WORD **SS:[ESP+ESI+4]**, ADD SI,CX, ADD SI,SI, MOVZX ECX,SI, store WORD **SS:[ESP+2*EAX+4]**. ESP remains the encoded base, retaining SS default. The linear offset simplifies to counts+2*EAX-2, but no DS counts-pointer load is substituted and no SS=DS premise is used. Partial SI keeps ESI upper16 from the calculated address; the two WORD ADDs and flags are retained, not widened or shifted. Each read precedes its scratch write, so aliases affect future counts. Scratch words1..15 are written in order; word0 remains untouched. ADD EAX1/signed CMP15/JLE complete this loop.
3. XOR ESI0; TEST signed EBX; JL skips node work only **after** all scratch accesses. Nonnegative max pushes actual EBP atS-28h, then MOVZX EDX from WORD **DS:[EDI+4*ESI+2]**. Zero length skips scratch/code work. Negative max is not an early guarded return.
4. For every nonzero raw WORD length1..65535, MOVZX EAX from WORD **SS:[ESP+2*EDX+8]**, MOVZX ECX,AX, ADD EAX1 as a **DWORD**, then store only AX to the same WORD. This is a separate read/register ADD/store, not a narrow memory INC/RMW. Old FFFF gives full EAX10000 at the store prefix while AX wraps0. XOR EAX0 then starts reversal.
5. Preserve the exact inline do-loop MOV EBP,ECX; AND EBP1; OR EAX,EBP; SUB EDX1; SHR ECX1; ADD EAX,EAX; TEST EDX; signed JG. SHR EAX1 follows and AX is stored to DS:[EDI+4*ESI]. No helper, intrinsic, masked shift, separate reversal call or len<=15 repair is added; DWORD shifts/wraps for all raw lengths remain.
6. ADD ESI1/signed CMP ESI,EBX/JLE traverse actual nodes. Max_codeINT32_MAX admits wrapping traversal/nontermination if accesses keep succeeding. No progress guard narrows it. Tree stores may alter future lengths/counts/scratch/saved/control/transport memory.
7. POP EBP only if pushed, POP actual ESI, ADD ESP20h, plain RET. The final ADD supplies arithmetic flags for this actual Source S; a different Original stack address need not give identical incidental flags. Saved cells may have changed. Kernel EBX/EDI stay unchanged; EAX/ECX/EDX results are path-dependent machine outputs, not recovered logical C++ return values.

There are no external calls/globals/imports/providers/FP/SIMD/string/cookie/EH instructions in the44-instruction body. All five conditional branches and all register widths, SS-default operands, read/store order, partial writes, flags and static fault-capable prefixes are mapped in the report.

## Actual control and transport aliases

For either node path the raw scratch expression is S-20h+2*length; it is not restricted to the reservation. Length65535 reachesS+1FFDEh. These real words are individually read, incremented through full EAX, then WORD-written before inline reversal.

| Raw lengths | Actual Source cells |
|---|---|
| 16/17 | Low/high WORD of real inner CALL continuation atS. |
| 18/19 | Extra saved Source EDI atT-8. |
| 20/21 | Extra saved Source EBX atT-4. |
| 22/23 | Actual outer caller return atT. |
| 24/25 | Actual counts transport atT+4. |
| 26/27 | Actual max_code transport atT+8. |
| 28/29 | Actual tree transport atT+C. |

Tree stores can also touch any such cell. A changed inner RET may redirect control and skip the wrapper suffix. Nothing snapshots, reconstructs, guards or restores an old return/saved word to force ordinary return. Source public/private and Original remain returning-capable; none is noreturn. The additional Source frame/control cells and code/return/fault/fetch placement remain explicit differences from Original, not captured-frame or universal Original/Source ABI/control equivalence.

## Existing Source, validation and limits

The genuine stock trees.c gen_codes and bi_reverse, full trees.obj and separate bsp_zlib121.lib are frozen in AB evidence. Stock gen_codes is private153B/60ins with three stack args, EBP/cookie frame and two genuine cookie dependencies. Both directories show no public gen_codes provider. This is complete algorithm Source, not this118B Native physical binding; no stock dispatch/cookie dependency is introduced here. A typed next_code[16] substitute would lose raw >15 control/frame behavior and operation/segment ordering.

Whole current worker/Root/Main src/include searches and indexed lookup found no selected standalone reconstruction before creation. Paths were absent in working trees and HEAD/Main/origin; the warm W/Z/AD branch was retained without force sync/discard. The AB54-byte build_tree window records gen_bitlen then MOV EBX,[ESP+Ch], LEA EDX,[ESI+B34h], CALLBCF140 atBCF3A5 with EDI actual current tree. Named build_treeBCF1C0 is unadopted, not a missing callee dependency; no whole-parent proof is credited. There is no AD/AA helper or separate AC copy_block dependency.

Strict C++17 `/MD /O2 /W4 /WX /fp:strict /Gy` and actual registered C++17 `/MD /O2 /W4 /WX /fp:strict` are inspected independently across all sections, raw data/metadata, symbols/aux/strings and relocations. The whole118B private body and22B public entry each occupy one executable section/function at offset0; all140B/53ins are accounted without gaps, helpers or prefix clipping. Exactly one genuine IMAGE_REL_I386_REL32 CALL operand at public+15 targets the defined static private Source symbol with zero addend. No Native address dispatch or unresolved external exists. The private symbol is not an invented archive export.

Actual current Core has one unique real member equal to the registered object. Public `_bsp_native_zlib_gen_codes_00bcf140` occurs uniquely in both complete linker dictionaries; all offsets/indexes resolve genuine member headers. All ten prior whole providers and Source/header pins remain unchanged. Already frozen AD current Core/object/project/tlogs and older nine objects were hash-verified/reused in place; no transitive AA/AB/Z/W/X/P historical tree/Core copy occurred. Only new AB32+receipt and small current registration were copied; final current object/Core/project/tlogs are preserved once for review.

Normal warm scripts/build.ps1 passed both existing checks in19.430251s including CTest8.11s, repeating the existing unrelated LNK4006 spawn_request_id_matches duplicate warning. Strict Source passed first attempt, with no failures/corrections/new tests/probes/selected execution/Ghidra writes. Static code proof does not measure dynamic faults, alias/return results, SEH/unwind, full formal Original ABI/CPU environment, absolute code/frame/control/fault/fetch identity, concurrency, ordinary-return consumers, caller adoption or game behavior. Original no_return=false and provisional library identity remain preserved.

AB's conservative first-clock floor10:49:57Z through receipt11:04:19.773035Z is862.773035s with <=1second starting uncertainty. The gap to AE first clock11:13:17Z is537.226965s of review/administration/scheduling. AE includes all preflight/seal/provider preservation verification, Source/strict compile/build/checks, complete proof, docs/metadata/index/commit/release/seal overhead. The ignored final receipt closes author time; independent review/integration and unrelated historical costs remain separate. No speedup claim is made.

Root alone may annotate after independent review: preserve old names/prototypes/comments and Original no_return=false, describe provisional custom EDXcounts/EBXmax/EDItree/no-stack/plainRET and actual scratch/SS/partial-register/control/flags semantics. The Source cdecl declaration is not a recovered Original declaration. Exact commit/hashes/frozen handoff are delivered separately.
