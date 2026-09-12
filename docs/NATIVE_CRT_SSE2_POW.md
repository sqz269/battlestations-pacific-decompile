# Complete native CRT SSE2 pow wrapper and core

`src/native_crt_sse2_pow.cpp` reconstructs complete `C19260[25]` and `C19279[2872]` with qualified raw MSVC Win32 interfaces. All 645 core instructions, 15 returns, 59 constant read sites and exactly 14,616 required read-only data bytes are retained. Names describe established behavior; they are not recovered symbols. The original full algorithm, packed lanes, current FP environment and error-service calls determine behavior.

The [audit](../reports/native_crt_sse2_pow_audit.json) records immutable native/source inputs, the instruction correspondence, table mapping, source bindings and verification boundaries. This packet owns only its source, header, document and audit. The parent owns `BFEB10` dispatch, shared build integration, address ledgers and Ghidra annotations.

## Native evidence

The live guarded BSP client verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` for every query. This packet reread all support-discovery evidence: 51 disk-backed spans totaling 19,539 bytes, and five separate saved-image virtual-zero regions totaling 32 bytes. Every disk span agrees with installed `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`, SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Saved zero fill is not an observed current runtime global value.

Fresh exports and raw native bytes, current dependent source/header copies, generation and proof scripts are archived under ignored `local/sse2_pow_body/`. The earlier [support discovery](NATIVE_CRT_SSE2_POW_SUPPORT_DISCOVERY.md) supplies the independently reviewed finite indexing, dependency and ABI evidence. This packet revalidates that evidence rather than assuming the previous live image or source is unchanged.

## Qualified raw source ABI

The wrapper is assembly-only `native_crt_sse2_pow_x87_00c19260(context&)`: ST0 is exponent y, ST1 is base x, and the added context word is at entry ESP+4. Original `FXCH; FSTP [ESP]; FSTP [ESP+8]` consumes the two inputs in their original order under the current x87 environment. Normal return leaves one result in ST0 and retains deeper caller x87 entries. `LEAVE; RET` restores entry ESP and EBP.

The wrapper reserves 20h instead of 10h bytes before `AND ESP,-16`, making room for the added context without overwriting saved EBP. Two inserted integer MOVs forward `[EBP+8]` to outgoing `[ESP+10h]`. They change no FP state or flags. The first original core `PEXTRW EAX` at `C1929C` overwrites incoming EAX before any read, and original `AND EAX,FFh` at `C192AA` overwrites integer condition flags before any branch. The wrapper's final alignment AND also overwrites flags from its enlarged reservation. Outgoing x and y remain aligned as originally; core-entry ESP is 12 modulo 16.

The core is assembly-only `native_crt_sse2_pow_00c19279(double x, double y, context&)`. At entry E, x remains `[E+4]`, y remains `[E+Ch]`, y's high DWORD remains `[E+10h]`, and the added context is `[E+14h]`. All original ESP-relative operands and all 15 RET instructions remain unchanged. Plain RET removes only the return address; the qualified caller removes 20 argument bytes. Return value is x87 ST0, with no universal XMM0 numeric return. The `void` declarations prevent a false C++ numeric-return contract.

Core-owned effects remain EAX/ECX/EDX, XMM0..7, integer flags, x87/SSE status and the original stack storage. EBX/EBP are untouched; both slow-scaling paths save and restore ESI/EDI. No implicit compiler prologue or epilogue is permitted. Each original final flag writer is retained in the audit; paths finishing in stack ADDs and paths finishing in comparisons remain distinct. Code addresses, provider-side volatile behavior, stack depth added for context, and fault/SEH continuation sites are qualified source differences, not original binary ABI compatibility.

## Complete error-service bridge

The sole external original core call is `C19AA3 -> C0F0E4`. The preceding original instructions still allocate 1Ch bytes, store XMM0 at private result `[E-Ch]`, store the selector, and pass pointers to x `[E+4]`, y `[E+Ch]` and that result. After normal return the original `FLD current_result; ADD ESP,1Ch; RET` remains unchanged.

That call binds to a fixed eight-instruction naked bridge. At bridge entry B=E-20h, `PUSH [ESP+34h]` copies the added caller context. Four successive `PUSH [ESP+14h]` copy selector, output pointer, y pointer and x pointer as ESP moves. The bridge calls the actual complete `native_crt_libm_error_support_00c0f0e4(first, second, output, selector, context)`, then uses `LEA ESP,[ESP+14h]; RET`. This preserves original private-result storage and caller x/y addresses while allocating the service's fifth argument below the core frame. It adds no XMM/x87 operation or integer-register/flag normalization around the service.

The full provider, its actual pointer decoder, canonical name identities, callback registration/storage domain and owning-CRT errno provider retain their separately documented contracts. Context references remain explicit and stable; there is no new ambient TLS or private runtime binding. Every core selector, including 24, 25, 26, 27, 28, 29 and 1006, reaches the same full provider. The provider's complete selector table is a dependency of closure even though this core emits only those seven selectors.

## Exact instruction and data contract

Core translation preserves every original instruction mnemonic, operand width, register and stack displacement. Internal branch destinations become address-labeled source labels. The one direct external call binds to the fixed bridge. Each of the 59 original absolute/indexed read operands becomes its corresponding rebuilt read-only array plus the original byte offset and index expression. No instruction is removed or mathematically simplified.

| Original region | Required bytes | Indexed entries and stride |
|---|---:|---|
| D70B10 | 1,032 | 129 at 8 bytes |
| D70F20 | 2,064 | 129 at 16 bytes |
| D71730 | 1,032 | 129 at 8 bytes |
| D71B40 | 2,064 | 129 at 16 bytes |
| D72350 | 2,056 | 257 at 8 bytes |
| D72B60 | 4,112 | 257 at 16 bytes |
| D73B70 | 32 | Two 16-byte constants |
| D73B90 | 2,048 | 128 at 16 bytes |
| D74390 | 8 | One scalar constant |
| D743A0 | 168 | Scalar and paired constants through D74448 |

Every array is `alignas(16)` and contains integer bit patterns recovered from native little-endian bytes, with a per-region hash and aggregate `sizeof` assertion. All original 16-byte operand addresses remain aligned. Four unread eight-byte alignment holes are excluded. Table bounds come from the original finite masked-index domains; no adjacent-symbol length inference is used.

Scalar memory `MOVLPD`, register `MOVSD`, packed arithmetic, shuffle and conversion instructions retain their different lane effects. Incoming upper XMM lanes are not zero-filled. The core uses current MXCSR rounding/masks/DAZ/FTZ and final FLDs use current x87 controls. Neither body saves, clears or restores FP controls/status. Deliberately generated invalid, divide-by-zero, overflow and underflow operations remain present even where a following constant fixes the returned bits. No host pow, forced legacy dispatch, reduced special-case implementation or semantic error callback is used.

## Verification status

Primary reviewed the entire wrapper, bridge, core, header and generation/proof scripts before the strict build, and independently compared all 14,616 source-array bytes with the installed executable. The approved source and header remained unchanged through verification.

`scripts/build.ps1` passed under MSVC Win32 `/W4 /WX /fp:strict`. Both existing CTests passed after all eight native reference seeds matched the installed image. The ignored worker overlay added this source, the actual complete decoder, and the pinned full error-service source/header; tracked shared build files stayed unchanged. Four actual CL command records and all 238 source/header read dependencies are archived.

The actual library's entire owned COFF object matches exactly one archive member. Its seven sections are fully classified: 30-byte bridge, 2,872-byte core and 32-byte wrapper code; 14,648-byte read-only storage comprising the exact 14,616-byte operand footprint plus 32 unread zero alignment bytes; linker directives, debug information and compiler checksum metadata. All 645 linked core instructions, 15 returns, branch destinations, 59 data reads and all 62 owned code relocations passed. Every original core byte outside the 60 core relocation words is identical. The wrapper's 12 instructions and bridge's eight instructions match their explicit source ABI changes.

The same address-only linked executable includes the actual full service/default/decoder/strcmp/OS-major/module-gate entries. Unchanged provider verifiers check every one of the service's 176 original instructions against its 255 emitted instructions/784 bytes, exact three-byte default handler, and all 13 read-only selector-table bindings. All six complete linked dependency bodies match their actual archive members after individually checked relocations. The unchanged earlier decoder verifier also passes all 36-to-46 instruction mappings, ten fixed bindings, actual name/import bindings, exact 136-byte comparator and normal stack accounting. All seven decoder/support COFF code sections match previous reviewed full code bytes; the worktree-derived anonymous-namespace symbol hash is the only normalized name component, with both raw spellings retained in the evidence.

The link artifact embeds a manifest and was never executed. No game execution, general original-address ABI compatibility or SSE2 pow numerical differential testing is claimed by these source/static checks or the repository's eight unrelated native math seeds. The final ignored handoff seals the exact archive, objects, executable/map, actual compiler inputs, original bytes, helper scripts, proofs and current four owned files.
