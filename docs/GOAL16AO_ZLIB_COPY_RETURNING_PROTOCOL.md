# GOAL16AO returning copy_block Source protocol

This extends the already complete `00BCF040 copy_block` module with a genuine returning Source interface. **Additional Native credit is zero.** The old private body134B/50 instructions and borrowed-frame NR entry21B/5 retain identical entire code and symbol identities. The new entry is24B/8; the complete current module is179B/63 across three Source functions. Original remains the returning custom EAX/ECX/EDX plus actual stack-header `RET4` function; neither Source prototype is projected onto it.

The accepted AK design is SHA256 `8bf27b3d691ac7857cf390ef8a46de988327632d1800ca212a193745afe1cb67`, receipt `ddff0f219009ce9d3a34cb6a047329fa944d950b48ec1a8e4a43a5c51e07627f`. All13 declared records/127709B and top seals were authenticated before changes; the complete design and eight-operation candidate were read. AG pin `3111c0cabfe01cb3f677db310964f5809acea2c8` and its independent review were accepted first. The genuine warm branch fast-forwarded to published Main `0f57e25ee18f9d6babff8fd2612ca2d9e7a4cd40`; AG and AD Source/header inputs were unchanged by that refresh.

## Four actual inputs and continuation

```cpp
extern "C" void __fastcall bsp_native_zlib_copy_block_00bcf040(
    void* backing, const void* bytes, std::uint32_t length,
    std::uint32_t raw_header) noexcept;
```

Let T be the actual Source entry ESP: ECX=backing B, EDX=bytes P, DWORD SS:[T+4]=length L, SS:[T+8]=raw header Q, and SS:[T] is this call's actual continuation R. Its exact complete prefix is:

```asm
mov eax, ecx
mov ecx, dword ptr [esp+4]
push ebx
mov ebx, dword ptr [esp+4]
mov dword ptr [esp+8], ebx
pop ebx
add esp, 4
jmp copy_block_body
```

L is read before its transport cell is consumed. After PUSH, ESP=T-4; the next read obtains current R at T and the write relocates it to T+4. POP reads the actual EBX save; ADD selects C=T+4. At the private-body boundary EAX=B, ECX=L, EDX=P, SS:[C]=current R, and SS:[C+4]=the actual original Q cell. There is no fifth parameter, fake/historical frame, copied header, extra CALL, private export, protected return, snapshot or post-body repair.

The body's PUSH EBX at C-4 overwrites the old return T; PUSH ESI at C-8 overwrites the temporary prefix save. PUSH EDI is at C-12, and the genuine AD CALL writes its actual continuation at C-16. Windup reads raw valid before its own PUSH at C-20. Prefix temporary cells are therefore replaced before the first existing DS backing read. In the prospective AH frame C=S-12 this leaves the concrete valid-cell counterexample at S-32=C-20 intact until that read. This is an ordered static argument under successful sequential ordinary32-bit stack execution, not AH implementation or adoption.

On an intact intended return, the unchanged `RET4` reads current SS:[C], skips C+4 and leaves C+8=T+12, the normal fastcall return position for two stack arguments. Header/count and continuation remain actual mutable cells. Raw writes can change saved registers, loop length, header, return or future caller cells. No dedicated R survives in a register through the body. Altered returns, nontermination and faults remain possible; no universal safe-return/nonvolatile guarantee is claimed for malformed aliases.

## Complete body and real provider

All49 Native instructions and the existing AD adaptation remain in the134B body. The actual header is compared before the metadata store, can be changed by windup, and becomes the real count spill/reload. BL/BH/CL/CH effects, optional real EBP save, every late pointer/pending read and RMW, source read after count spill, destination store before count reload, and actual final POPs/RET4 retain their schedules. All32-bit counts, raw nonzero headers, wrapping/unaligned addresses and backing/source/output/frame aliases remain admitted without clamps or repair. The complete schedule remains in [GOAL16AC_ZLIB_RAW_COPY_BLOCK.md](GOAL16AC_ZLIB_RAW_COPY_BLOCK.md).

The CALL still names the whole genuine `@bsp_native_zlib_bi_windup_00bcefc0@4`:117B/36, including MOV EAX,ECX plus all115B/35 Native instructions. The AD module236B/68 and entire1086B current object are unchanged. Its EAX backing/ECX zero on intact completion, path-dependent EDX/flags, actual aliased saves, POP ESI before WORD-buffer then DWORD-valid reset, and later POP EBX are machine evidence beyond the void declaration. The NR entry remains available unchanged.

## Measured evidence and limits

First strict C++17/MD/O2/W4/WX/fpstrict compile passed in1.5144634s:1238B whole COFF. Actual registered COFF is1262B. Both have six sections and the same complete179B/63 instructions. Every function starts at executable-section offset0. All code/data/directives/debug/checksum bytes, symbols/auxiliary records, strings, padding and relocations are retained and accounted. Exactly three REL32 operands exist: private CALL operand10 to whole AD, old NR JMP operand17 and returning JMP operand20 to the real defined same-object private body. No helper, `_emit`, absolute Native target, new undefined provider or registration is added.

Actual Core94380984B has SHA256 `f54087b4dd69ab705da99bac5474792972a7202a16ee4dbe39b7c9146b1670d4`. Both complete83034-symbol dictionaries uniquely bind new `@bsp_native_zlib_copy_block_00bcf040@16`, old NR and both AD publics to real whole members; private is not exported. Twenty-five prior selected whole members are unchanged. Recompiled27353B singleton destruction differs only at COFF timestamp bytes4/5/6; all other bytes agree. No universal old-cache equality is claimed. Old Core was not recopied/reparsed/wholly rehashed; selected old members were read by retained offsets.

Required normal `scripts/build.ps1` ran once using the warm cache: terminal0 in93.356344s, both existing checks PASS, CTest7.94s. Published-Main refresh caused116 Source compilations across the build; that cost is included separately from unchanged selected code/provider evidence. Actual unique project items, commands, objects/Core/project/tlogs/check log were frozen once. No new tests, probes, selected Source/Native execution, Ghidra writes or Main mutation occurred.

ADD changes arithmetic flags. No existing copy instruction consumes those flags before windup's CMP after its first raw valid read; subsequent defined flags follow unchanged instructions. Undefined AF/shift flags remain undefined. DF and x87/SSE state are not normalized. Extra fetches, early stack accesses and transient EAX/ECX/EBX/ESP/flags change fault prefixes. Current SS stack and actual DS data operands are used without assuming DS=SS or historical segment identity. Successful listed execution with ordinary32-bit stack-address semantics delimits the static schedule. Guard/segmentation/alignment/wrap faults, code/data aliases and self-modification, concurrency/debug/CPU effects, actual return bits/code placement, full Original ABI/SEH/unwind, higher-parent/game/runtime results remain unproved. No faults or selected function effects were dynamically measured.

AH implementation/adoption remains separately gated; this packet freezes the real returning provider for independent review first. Root retains correct library names, old comments and Original no_return=false/prototype. AO began2026-10-16 12:57:59 UTC; all setup, refresh, audit corrections, compile/build/checks, proof, documentation, commit and release are included through final ignored handoff. AK's840.610298s and earlier AG/AC/AF/review intervals stay separate; overlapping work is not summed into a speedup claim. Exact pin, hashes and closing time are under `local/goal16ao_copy_returning`.
