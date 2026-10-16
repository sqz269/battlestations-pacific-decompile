# CC12 raw wide-string copy constructor101

Whole raw `4C8DD0[4C8DD0,4C8E18)` is reconstructed as `copy_construct_native_wide_string_header_004c8dd0` in the existing native wide-string module. It uses the genuine existing raw `4C53E0` resize and the caller's SAME retained `NativeStringStorage` domain. Production must borrow the existing `GameNativeStringProcess::strings()` actual pool service; this packet creates no owner, default, global getter or replacement resize callback.

The header/source change is append-only:14 header lines and25 C++ lines. Existing raw constructors, resize and both destructor overloads are unchanged. The old CC13 typed `NativeWideString` wrapper and its ledger status are preserved. No Text copy, cursor, rebuild, factory or production admission is added.

## Original contract and evidence

The checked clean baseline merge is Root `1e2ee9972c5a3ff96c1105cde298ce72e5663536`. The configured existing project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; live reads and export refresh use verifying `bsp ghidra` wrappers. No import or listing repair is needed.

Native body is72 bytes/27 contiguous instructions. ECX is the actual8-byte destination header; one stack DWORD is the actual source header; EAX returns SAME destination. It preserves ESI/EDI and RET4 begins at `4C8E15`, ending exclusively `4C8E18`. There is no local FH3 frame, owned acquisition, destructor or rollback. Descriptive names remain hypotheses. The new three-argument C++ interface is not its binary calling convention.

The complete current live bytes match packet100 and the preserved Original executable at `I:/SteamLibrary/steamapps/common/Battlestations Pacific/battlestationspacific.exe`, SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. All27 listing starts agree with the matching Original decode. The one direct native call, `4C8DEE ->4C53E0`, passes live report verification with0 failures; that verifies a call site, not execution.

## Raw schedule and storage

The destination is length DWORD+0 and data DWORD+4. Only those two header words are initialized. Caller-provided headers, reached backing and storage must be valid for every observed access and remain alive through call/unwind. No capacity field, typed string object, copied allocator or implicit owner is introduced. The existing storage interface does not authenticate provenance: binding the SAME existing actual process service is a required caller contract, not a service that this helper invents or validates.

| Original point | Behavior |
|---|---|
|`4C8DD8`|Compare exact destination/source header identity.|
|`4C8DDA`, `4C8DE0`|Clear destination length, then data, including exact self identity. Any prior constructor preimage backing is abandoned without release.|
|`4C8DE7`|Self branch returns SAME now-zero destination; it does not return early before the clears.|
|`4C8DE9`, `4C8DEE`|Capture source length, call the genuine existing resize with preserve1.|
|`4C8DF3`|Test CURRENT source length after resize. Zero skips all source/destination data-pointer reads.|
|`4C8DF8`, `4C8DFB`|For nonzero length, capture CURRENT source data first, then CURRENT destination data.|
|`4C8E00..4C8E0F`|Load one WORD, store it, advance both cursors by2, test that captured WORD, repeat through and including NUL.|
|`4C8E12..4C8E17`|Return SAME destination with native saved registers/RET4.|

The Source helper uses volatile DWORD header accesses and volatile WORD backing accesses. Pointer cursor arithmetic uses the existing module's32-bit `uintptr_t` domain. It never snapshots the source header across resize, hoists data reads onto the zero-length branch, uses the requested length as a copy bound, shares source backing ownership or substitutes counted memcpy. The loop captures each unit before storing/testing it, so it preserves forward streaming behavior for valid reached aliases.

There is no new validation, catch, cleanup or rollback. Existing Source resize/storage allocation, release and noexcept contracts remain inherited; this is not a claim that all Original native/private exception origins are reproduced. A failure can leave the constructor's actual partial effects. Future raw Text copy calls this at `ABB303` while outer state0 is armed; wideEC cleanup is not armed until that call returns. No new Text caller is implemented here.

## Optimized-code limits

Strict optimized MSVC Win32 output in `local/native_wide_string_raw_copy_101.asm`, lines402–487, has these observed properties:

- Destination clears are at416 and420; the emitted identity CMP is at421, followed by the self branch425. Original evaluates CMP before its clears. Identity is invariant with no intervening call, so the same header effects and self result hold. This emitted ordering difference is explicitly retained in the evidence; no byte/order equivalence or Native flag/ABI claim is made.
- Genuine resize is called at437. CURRENT source length is tested at442. Source data447 and destination data451 are captured only on the nonzero branch and in that order; no hoisted pointer reads were observed.
- WORD read459 precedes WORD store470 and captured-unit NUL test474. The compiler calculates local cursor advances before the store and addresses it as `[edx-2]`; the observed memory-access sequence and through-NUL result remain the same. This also is not an instruction-for-instruction claim.
- SAME destination returns at484. Source uses its new C++ call frame/RET0, not native ECX/stack-source/RET4.

Root independently reviewed the append-only header/CPP and then the whole optimized helper against all27 Original instructions: both passed. The comparison placement and local-cursor scheduling differences were explicitly reviewed and accepted within the new Source interface.

## Verification and publication

- Strict Win32 `/MD /O2 /W4 /WX /fp:strict /std:c++17 /EHsc` compilation passed with0 diagnostics.
- Required `./scripts/build.ps1` exited0. The existing registered `src/native_wide_string.cpp` TU compiled, and both configured checks (`reconstructed_math`, `tool_tests`) passed. This does not exercise the new helper's behavior.
- One live call row checked,0 failed. No new tests or executable probes were added.
- The report seals current Source/header, strict object/assembly, required build log/output, live Original bytes/prototype and saved annotation/history/export receipts. Existing Source85/93 and CC13 typed source hashes are unchanged.
- Only owned `4C8DD0` receives locked evidence annotation/history/save/export and a new unique raw function record. Existing descriptive Ghidra name and prior comments are retained; earlier typed reconstruction rows/status remain unchanged.

See `reports/cc12_native_wide_string_raw_copy_101.json` and its final artifact manifest for exact hashes and publication receipts. Raw helper execution, native FH3/private calling ABI, whole Text-copy behavior, gameplay and rendering remain unproved.
