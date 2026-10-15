# Record vector tidy at 0041F950

Addresses: 0041F950 (owned); 00BF65AC (read dependency); 00C5E6D8,
00C92988, 00424A10 and 0083B120 (read owner evidence).

The existing `STL_Vector_Tidy_0041f950` name is retained. The complete body
releases a raw allocation and clears three words; no element type or complete
embedded class is inferred from that template-shaped name. The source entry
works on the existing receiver bytes through the established real CRT service.

| Routine | Native body inclusive | Coverage | Original ABI |
|---|---|---|---|
| STL_Vector_Tidy_0041f950 | 0041F950..0041F979, 42 bytes | complete | ECX receiver, no stack inputs, plain RET, ESI preserved |

`tidy_native_record_vector_0041f950(void*)` uses MSVC Win32 `__fastcall`, keeping
the same input register and return shape. It is a new source function, not an
installed original binary entry. Native EH frames, asynchronous SEH behavior,
and original CRT heap identity remain outside the claim.

## Complete schedule

1. Capture the DWORD pointer at receiver `+60h` (0041F953).
2. If nonnull, call `00BF65AC` at 0041F95B. `ADD ESP,4` at 0041F960 establishes
   one cdecl stack argument. The existing `singleton_lifetime_free` provider
   calls genuine `std::free`; no callback or success substitute is added.
3. Zero DWORDs at receiver `+60h`, `+64h`, and `+68h` in that order
   (0041F963, 0041F96A, 0041F971), including on the null-pointer branch.
4. Restore ESI and return. No pointee element is inspected or destroyed;
   other receiver words are untouched and the outer receiver remains allocated.
   There is no count iteration or element destructor.

The pointer must match the source CRT allocator, and the supplied existing
receiver extent must reach `+6Ch`. No null-receiver guard, allocation, shadow
owner, alternate layout, or implicit destructor is introduced.

## Owner and producer evidence

The complete physical census (`tools/callsite_census.py 0041f950`) found exactly
two rel32 JMPs and no literal-address occurrences in its scanned image sections.
Live xrefs agree. Both are complete unwind helper bodies:

| Unwind helper | Setup | Tail site |
|---|---|---|
| 00C5E6D8..00C5E6E5 | ECX = `[EBP-10h] + 180h` | 00C5E6E1 JMP 0041F950 |
| 00C92988..00C92995 | ECX = `[EBP-10h] + 180h` | 00C92991 JMP 0041F950 |

Read-only FH3 metadata/xrefs connect the first helper's unwind-map entry at
00D84BDC through function-info 00D84BAC and EH handler 00C5E75A to constructor
`BSP_GameplayTuningSettings_Construct` at 00424A10. The second entry at
00DC43C8 connects through function-info 00DC4398 and handler 00C92A02 to the
settings destructor at 0083B120. These are evidence addresses, not newly owned
or reconstructed routines.

The constructor's assembly clears parent `+1E0h/+1E4h/+1E8h` at
00424A3E/00424A44/00424A4A. Its pseudocode also records the settings vtable
and the same fields as DWORD indices 78h/79h/7Ah. The normal destructor's
pseudocode frees word 78h when nonnull and clears those same three words.
Together with the two `+180h` adjustments, this establishes the observed parent
offset relationship: `180h + 60h = 1E0h`.

`GameplayTuningSettings` in `include/bsp/gameplay_settings.hpp` already reserves
the affected bytes within `gap_1DCh[10h]`; that existing schema is preserved.
The packet declares no competing settings record or new member type. Vector
element semantics and the complete subobject beginning at parent `+180h` remain
unknown. Passing the parent base directly to this helper would address the wrong
fields; the source parameter is the native adjusted receiver.

## Verification and integration

The configured live bridge verified project `bsp`, program
`/battlestationspacific.exe`, x86 base 00400000, using
`C:/Users/sqz269/bsp.gpr` before each analysis/export batch. Full assembly of
the routine and both unwind helpers was read. Live containing-function queries
confirm the one free call and both tail-jump sites. The CRT free body and its
existing `singleton_lifetime_free` implementation were inspected in the related
pool cleanup packet in the same worker session.

Worker checks passed: MSVC x86 C++17 `/c /W4 /WX /permissive- /O2` object compile,
all three report call rows, all three markdown call claims, and `git diff --check`.
The primary integrator registers
`src/native_record_vector_tidy.cpp` in `bsp_core`, then runs the required full
`scripts/build.ps1` build and existing CTests. No new tests are added for this
small direct composition. Object compilation is not library-linkage, original
ABI, unwind execution, runtime, or game validation.

The worker adds only this address's name/reconstruction ledger records and its
four owned outputs. CMake registration, Ghidra annotations and affected export
refresh remain primary-owned; the existing library-shaped name is preserved.

Primary integration: registered MSVC Win32 build and all three existing
CTest suites pass. Ghidra annotations are saved and exports refreshed.
See [the batch receipt](../reports/goal15b_reconstruction_batch.json) for
the precise Source fixture/static-proof scope and remaining Native limits.
