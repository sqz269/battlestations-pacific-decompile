# Raw wake append dependencies: 0042B2F0 and 00810160

This packet adds two explicit raw MSVC Win32 entries in `native_unit_wake_append_dependencies.cpp`: the 77-byte/31-instruction 0042B2F0 length body and the 43-byte/15-instruction 00810160 sample-copy body. The length object matches every Original byte except its two identified data/call relocations; the copy object matches all 43 bytes exactly. Both complete functions are present in their exact unique member of the actual `bsp_core.lib` archive.

The normal Win32 Release build and all three existing checks passed. **This is Source, compilation and static artifact evidence. Neither new Source entry nor either Original entry was executed for this packet.** There is no new fixture, forced/ad hoc link, probe, runtime provider admission, application binding, append implementation or gameplay validation. Both new bodies have zero matching code patterns in the existing `bsp_game.exe`; their uncalled archive COMDATs are not claimed as linked application functions.

## Public raw interfaces

| Source entry | Original contract retained |
| --- | --- |
| `float __fastcall native_unit_wake_length_0042b2f0(const float*) noexcept` | Actual vector in ECX; reads three float cells; ST0 return; PUSH ECX allocates scratch; POP ECX returns final float bits; RET consumes no caller arguments. Four local x87 slots are required, excluding the CRT provider's private requirements. |
| `void* __fastcall copy_native_unit_wake_sample_00810160(void*, void* unused_edx, const void*) noexcept` | Destination in ECX, source at entry `[ESP+4]`, destination returned in EAX, source left in ECX, EDX untouched, RET4. The unused second parameter preserves the native stack position and is not an owner/context input. One free x87 slot is required. |

These are deliberate raw calling interfaces, not ordinary cdecl wrappers. Their `__fastcall` spelling models the original register/stack arrangement; the inline assembly, including RET/RET4, is authoritative. Callers borrow actual live readable/writable storage and own lifetime, validity and synchronization. The functions do not validate pointers or clear the floating-point environment. Copy accepts real overlapping/self-aliasing storage and performs sequential accesses. Access faults and unmasked FP exception handling remain the caller/runtime's responsibility. No owner class, typed trail overlay, ring initialization or lifetime manager is introduced; descriptive names are reconstruction hypotheses.

## Whole length body and real dependencies

0042B2F0 loads Y, X, Z, uses the original ST-register multiplication/addition order for `(Y*Y + X*X) + Z*Z`, then stores squared length to a float stack cell. It loads the exact immutable double at CE3820, loads the float squared value and uses `FCOMI ST0,ST1`. `FSTP ST1` removes the threshold without changing EFLAGS. `JBE` sends ordered <= and unordered results to the zero arm. That arm performs XORPS, drops the x87 input, MOVSS-stores positive zero and FLD-loads the result.

The greater arm calls the genuine current CRT `_CIsqrt` with squared length in ST0. It retains both separate float store/reload pairs after the call. ECX receives the final float result bits from the overwritten stack word, and that float-spilled result remains in ST0. No SSE predicate, ordinary stack-argument sqrt, combined expression, omitted spill, result normalizer or invented backend replaces this sequence.

| Relocation in the complete 77-byte object body | Physical resolution |
| --- | --- |
| Offset 30, `IMAGE_REL_I386_DIR32` | Private read-only eight-byte constant `squared_length_cutoff_00ce3820`, payload `bb bd d7 d9 df 7c db 3d`, equal to the live/disk Original CE3820 double 1e-10. |
| Offset 44, `IMAGE_REL_I386_REL32` | External COFF symbol `__CIsqrt`, using the same `extern "C" double __cdecl _CIsqrt();` declaration and assembly-call contract as the existing exact 2D kernel and camera decomposition. |

The actual SDK x86 `ucrt.lib` selected by the existing executable's linker read log is physically retained. Its unique 60-byte import member for `__CIsqrt` names `api-ms-win-crt-math-l1-1-0.dll`; import-name type 2 strips one linker underscore to `_CIsqrt`. The actual existing executable's IAT and FF25 thunk agree. Its complete existing 66-byte 2D kernel and camera normalization body resolve their real `_CIsqrt` calls to that thunk. This establishes the existing static provider boundary; it does not establish a loaded module for the new uncalled function.

The complete Original BF7030 boundary is retained separately: 20 bytes, six instructions, two internal calls. Its worker, dispatch globals, diagnostic policy, environment handling and exception behavior are not reconstructed by this packet. Original Ghidra's `undefined ...(void)` prototype omits the x87 input/output; the actual caller assembly and existing raw Source providers establish the contract used here. No original-versus-current CRT-interior equivalence is claimed.

## Whole sample-copy body

00810160 first copies ECX to EAX, loads the source pointer from `[ESP+4]`, then executes FLD/FSTP pairs for offsets 0, 4, 8, 0xC, 0x10 and 0x14 before `RET 4`. There are no calls, globals, constants or relocations. The data extent is 24 bytes; the code extent is 43 bytes.

All six load/store events are retained. With partial overlap, an earlier store may affect a later load; for example, a destination one float above the source can propagate the first value. Self-copy is still a sequence of FP operations and can quiet signaling NaNs or change status under the ambient FP mode. Integer/MOVUPS/MOVQ copying and snapshot/memmove semantics would not implement this body. The function updates no head, flag, residual or bookkeeping; its caller must supply actual selected sample addresses.

## Physical verification and preserved code

The existing `bsp.gpr` / `/battlestationspacific.exe` project/program is verified before each live query. Pre/post captures of both new target bodies, the exact existing 2D helper, BF7030 boundary and CE3820 constant agree with the installed Original PE SHA-256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Independent Capstone decoding agrees with all target instruction starts. The report maps all 31 + 15 instructions to their complete compiled object bodies, identifying the only two relocation operands.

Eight complete production COFF objects and all eight exact unique archive members are retained, together with the actual whole library, selected current sources/headers, compiler and linker recipes, current executable, genuine import library/member, native bodies, immutable data and build/check logs. The old objects cover typed 3D, exact raw/public 2D, camera math, typed append, whole-ring copy, fill and mutable geometry storage. The actual compiler recipe retains `/O2 /MD /fp:strict`. Twenty existing source/header/config/tool inputs are unchanged. The only existing tracked-file change is one appended CMake registration for the new source.

Existing `force_event_vector_length_0042b2f0` remains its real 116-byte/37-instruction typed cdecl implementation with ordinary `_sqrt`. Existing public 2D and wake helpers, the whole-ring 00815680 copy and `GameNativeGeometryGlobals` remain intact. The new leaf functions do not create a reset owner, read/reset geometry globals or implement 00810190. The existing unrelated LNK4006 duplicate `spawn_request_id_matches` warning remains; it did not fail the build or checks.

The report is [cc12_native_wake_append_raw_dependencies.json](../reports/cc12_native_wake_append_raw_dependencies.json). The independent ignored evidence family is `local/cc12_native_wake_append_raw_dependencies_evidence`; its sealed archive identity is recorded in the report. Earlier append-readiness evidence remains unchanged.

## Address accounting and limits

Before/after address lookups agree: 0042B2F0 already has a reconstructed Original-function record for the typed helper. This packet adds another raw Source view of that same address, so it contributes **zero additional Original-function count**. 00810160 has no existing reconstruction record and is one distinct newly reconstructed Original body eligible for integrator registration. This worker changed no shared ledgers or registry, so the packet's actual shared-ledger count delta is zero; the possible integration increment is one, not two.

The static instruction/ABI representation is complete for these two entry bodies under their stated memory and external-provider contracts. No execution establishes numerical cases, exception delivery, runtime ABI behavior, loaded CRT identity, wake append wiring, class construction, game startup or gameplay. The broader 00810190 append and its remaining caller/global/data bindings still require their own packet and admission evidence.

Primary review independently rehashed96 artifacts/all97 ZIP members,26 selected physical inputs and20 preserved Source pins, four complete Original byte/instruction spans, all99 functions in eight current whole objects, unique worker/current archive members and the real SDK60-byte `_CIsqrt` import member. The new raw77-byte length matches every nonrelocation byte with its actual readonly cutoff; copy43 is fully byte-identical. The current combined MSVCWin32 build and all three existing checks pass. Root registered one additional raw view of already-counted0042B2F0 and one new Original body00810160; no new entry execution or app/parent/class/game qualification. Receipt: `local/cc12_native_wake_append_raw_dependencies_primary_review/receipt.json`.
