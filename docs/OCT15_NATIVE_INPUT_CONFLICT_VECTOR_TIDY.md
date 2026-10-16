# Native conflict vector range and tidy

The distinct actual-storage interfaces in `native_input_conflict_vector_tidy.hpp/.cpp` reconstruct whole `0069EEA0` (54 bytes, 25 instructions) and whole `006A1420` (61 bytes, 21 instructions). Fresh current typed ranges, PE/live bytes and every current instruction start agree. Existing `STL_DwordVectorRange_Destroy_0069EEA0` and `STL_DwordVectorVector_Tidy_006a1420` identities are retained; descriptive Source/member names remain hypotheses.

| Entry | Native physical inputs and return | Source |
| --- | --- | --- |
| `0069EEA0` | ECX first, EDX last, two ignored stacked DWORDs; RET8 | `void __fastcall destroy_native_input_conflict_vector_header_range_0069eea0(void*, void*, uint32_t, uint32_t)` |
| `006A1420` | ECX actual16B header, EDX unused, no stacked words; RET | `void __fastcall tidy_native_input_conflict_vector_006a1420(void*, uint32_t)` |

The core captures the range end once and compares before any element access. For each reached16B header it captures +4 once, conditionally frees that captured backing pointer, and unconditionally clears +4, +8 and +C in order. It retains opaque +0 and advances by unsigned DWORD +10h until equality with the captured end. It reads no element end/capacity and adds no child walk, null-header skip, range validation or rollback.

The tidy reads initial +4 unconditionally. Only nonzero initial begin reads +8 and invokes the core. After the core it reloads CURRENT outer +4 and frees that value, including zero, within the initial-nonnull arm. Finally it clears +4/+8/+C in order on both arms. Header aliases therefore retain the native captured-versus-current distinction.

Native `PUSH ECX` saves KNOWN incoming header bits in a compiler-private slot. After `PUSH ESI`, `MOV EAX,[ESP+4]` reads those known bits; both native child metadata words contain header bits. Neither word is consumed by the core. Source passes `(header,0)`, explicitly normalizing the second dead word. The inherited privateword description in the frozen readiness receipt is corrected here and in Source/header/report/ledger; its native bytes/listing remain valid evidence. No semantic frame reader is introduced.

Owned transfers:

```asm
0069EEB8 CALL 00BF65AC
006A1434 CALL 0069EEA0
006A143D CALL 00BF65AC
```

Bounded read-only support: `006AA531..006AA560` (48 bytes/17 instructions) captures the conflict range and reloads CURRENT outer begin; logical unwind action `00C7F9A4..00C7F9AE` (11 bytes/3 instructions) selects the tidy. These support entries are not owned or promoted by this packet.

```asm
006AA547 CALL 0069EEA0
006AA550 CALL 00BF65AC
00C7F9AA JMP 006A1420
```

The current service is `singleton_lifetime_free` / current CRT. This free-only pair needs no RawPoolContext, callback/provider, allocator model or dependency on the separate4D49B0 packet. Existing actual AV `destroy_native_input_settings_conflict_pair_range_0069eea0` and its valid fixtures remain unchanged. Its generic words path includes an extra end read/trivial child loop and current-begin reread; the new separate interface preserves the exact native captured-pointer schedule. Whole RawCtx `006AA460` remains outside this packet.

Neither owned body has local EH. Source adds no catch/cleanup guard; a failure stops subsequent cleanup. Native EAX residue is explicitly not a semantic result of the Source `void` APIs. Matching physical word/callee-pop shapes does not establish original CRT heap, private frame, ambient register, FH3/SEH/hardware-fault, binary replacement ABI or game behavior.

The required `./scripts/build.ps1` MSVC Win32 Release build passed all three existing suites in8.56s. No new tests. Actual CMake COFF: core76B/33 instructions with RET8, tidy137B/53 instructions with plainRET;14 focused checks/no findings. The actual compiler retains an EBP frame; independent optimized object ESP-local layouts differ (79B/30 and149B/50), so they are not byte-identical. Every actual instruction was reviewed for the same rawread/free/store/step/branch effects. Both inline the same authored core into the tidy and preserve the fresh postrange outer-begin reload. Existing native differential checks cover seededmath; this is build/static/compiled proof, not cleanup runtime/game validation. Detailed evidence and preservation hashes are in `reports/oct15_native_input_conflict_vector_tidy.json`.
