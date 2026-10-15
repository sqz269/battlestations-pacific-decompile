# Native eight-byte POD range helpers

This packet reconstructs the complete four physical bodies used by the draft-pair insertion path and push-back. The Source entries operate on actual raw storage. Existing `avoid_zone_draft_layers` and CC13 projections are separate interfaces and are not providers for these bodies. Descriptive names are hypotheses; the existing Ghidra library-role names and comments are retained.

| Native complete span | Source entry | Original physical ABI |
| --- | --- | --- |
| `00419960..0041998B` (44 bytes) | `copy_native_pod8_range_00419960` | ECX first, EDX last; stack destination, ignored0, ignored1, tag; `RET 10h`; EAX advanced destination |
| `0041C7E0..0041C80B` (44 bytes) | `fill_n_native_pod8_range_0041c7e0` | ECX destination, EDX unsigned count; stack value, ignored0, ignored1, tag; `RET 10h`; no established semantic result |
| `0041E3C0..0041E3F1` (50 bytes) | `fill_n_native_pod8_dispatch_0041e3c0` | ECX metadata preimage; stack first, count, value; `RET 0Ch`; EAX first + count*8 modulo 2^32 |
| `0041F4E0..0041F504` (37 bytes) | `copy_native_pod8_dispatch_0041f4e0` | ECX metadata preimage; stack first, last, destination; `RET 0Ch`; EAX from the actual copy kernel |

The kernels use an exact six-word Source fastcall declaration. Each wrapper uses an explicit Source fastcall adapter with ECX preimage and an unused EDX placeholder before the original three stack arguments. **That EDX placeholder is not an original parameter.** A reconstructed assembly caller can set ECX and push only the original three words. No Source adapter prototype is to be installed as an Original Ghidra prototype.

## Memory and register behavior

The copy loop first compares ECX and EDX, loads its destination into EAX even on an empty range, and returns immediately for equal endpoints. Otherwise, each iteration tests the current EAX destination. A zero destination skips both source reads and both stores. For a nonzero destination, it reads source word0, stores destination word0, then reads source word1 and stores destination word1. The first store can change the second source read. Both cursors advance by native DWORD ADD8, including a skipped iteration, and termination uses equality after that increment. No endpoint subtraction, overlap repair, float conversion, cached pair or C++ pointer arithmetic is introduced.

The fill loop uses `TEST EDX,EDX; JBE` at entry. TEST clears CF, so the guard admits every nonzero uint32 count, including the sign bit. Count0 returns without loading the value pointer and preserves incoming EAX. A nonzero count loads the value pointer into EAX; every iteration performs the same current-destination null test and ordered two-word copy, rereading the value each time. `SUB EDX,1; ADD ECX,8; TEST EDX,EDX; JA` retains unsigned count and address wrap. Nonzero completion leaves EAX equal to the value pointer, ECX advanced, and EDX zero. Both kernels preserve ESI; EDI is untouched. The fill Source return type is void because no consumer establishes an EAX result.

Both wrappers save incoming ECX as a local and zero **only that local's low byte**. The fill wrapper passes the actual value, original unmasked ECX preimage, value again, and masked tag as the four stack words, with actual first/count in ECX/EDX. It computes its endpoint after the genuine fill call. The copy wrapper passes actual destination, unmasked ECX preimage, destination again, and masked tag, with first/last in ECX/EDX. The final POP ECX restores the masked preimage, not the incoming value. The fill wrapper preserves ESI/EDI. Exact instruction order and even the native seven-byte no-op LEAs are retained.

These are unchecked Native schedules. The copy +8/equality schedule must terminate; all memory accesses actually reached must be admitted. Empty, zero-count and current-null-destination cases retain their narrower no-access behavior. There is no new validation, allocation, lifetime policy or replacement container.

## Caller and target evidence

All live queries use `bsp.py ghidra`, whose client verifies project `bsp`, program `/battlestationspacific.exe`, language `x86:LE:32:default`, and image base `00400000` before each batch. The existing project is `C:/Users/sqz269/bsp.gpr`. Complete four-body assembly and every discovered argument setup were read before fixing the Source ABI.

The byte census finds thirteen direct CALLs: six to copy, three to fill, two to each wrapper, with no literal-address or tail-JMP occurrences in the scanned sections. Nine sites have live function/listing ownership. Four additional complete 33-byte wrappers are not defined as Ghidra functions: heads `0041CAC0`, `0041DC80`, and `0041E6F0` dispatch to copy; head `0041D9B0` dispatches to fill. Their full argument setups, native CALL targets and RETs were inspected from disk and captured live memory. Each leaves ECX/EDX unchanged and pushes masked ECX, incoming stack word2 twice, then stack word1; these match the kernel's four stack slots. Their reconstruction and Ghidra creation are outside this packet.

The index incorrectly attributes some insertion sites to `Catch_All@004210E7`. Live containment places `00421080`, `00421141`, `0042111E`, and `00421188` inside `00420F80..004211C0`; the separate catch body is `004210E7..004210FB`. This packet changes no caller flow or function ownership. Exact call rows and old names/comments/prototypes are recorded in the JSON report.

## Verification and limits

The standalone object compiled without warnings under MSVC x86 14.51.36244, C++17, `/MD /O2 /W4 /WX /EHsc /fp:strict /permissive-`. All four positive defined COFF function sections have the exact complete Native lengths. Live Ghidra captures equal the installed PE bytes. All 167 bytes outside two four-byte CALL displacement operands match. The only two COFF relocations are `IMAGE_REL_I386_REL32` to the actual defined fill/copy kernel symbols; native displacements also resolve to the expected exact entry addresses. No opcode, branch, tag operation, return, stack cleanup or unknown relocation was masked.

One ignored `local/goal15f_pod8/pod8_probe.cpp` executable passed the concrete alias/null/wrap risks: word1 reread after aliasing word0 store, raw signaling-NaN bits, inaccessible source skipped for null destination, first cursor wrapping from `FFFFFFF8h` to0 on that skipped copy, count0 preserving EAX, wrapper masked ECX, endpoint and RET cleanup. It runs the reconstructed Source bodies, not captured Native code. It is linked with `/MANIFEST:EMBED`; no new test target or framework was added.

Artifacts and exact bytes are in `reports/goal15f_pod8_range.json`; temporary compilation, census, captures and probe live under ignored `local/goal15f_pod8/`. The primary integrator owns CMake registration, the required full Win32 build/existing checks, Ghidra annotations and refreshed exports. No worker Ghidra mutation occurred. Standalone object/Source fixture proof is separate from registered archive membership, Original ABI installation, captured-Native execution, original runtime/CRT identity, exception behavior and game validation; those are not claimed here.
