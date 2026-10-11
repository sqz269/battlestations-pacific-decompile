# CC12 locale exit target metadata

BF6FF5 has fresh, matching typed metadata at saved database modification 98 and function count 64730. The target is currently named `_atexit`; the database reports an 18-address function and a 4-byte entry instruction. These are database claims. No Native code/data bytes were requested or returned, and this packet establishes no opcode, return behavior, registration, ABI or execution result.

## Scope and the prototype endpoint expansion

The only requested target was BF6FF5. The saved project marker was `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language `x86:LE:32:default`, image base 00400000. Both typed captures retained raw HTTP bytes, status and SHA256. Their full responses and all four before/after identity records match. The count captures also match. Modification 98 was observed afresh; equality to the inherited epoch was not a prerequisite. This is observed database consistency, not a proof of exclusive access or of runtime state. Native access is closed.

`python tools/bsp.py ghidra proto BF6FF5 --lines 70` was issued with separate arguments `--lines`, `70`. Its first endpoint supplied the name, prototype, entry and body span. Its second endpoint, `get_function_signature`, unexpectedly supplied body-derived metrics, a basic-block hash and a callee label. The full exact stdout is retained in `Native_before/prototype.txt`, with the actual argument vector in `Native_scope.json`. **The whole capture is not strictly entry-only.** Root explicitly accepted the independently bounded typed/prototype subset with this disclosure. The extra fields are quarantined from semantic/ABI credit and never justify a new target query. The prototype command was not repeated. No bytes, instruction text, decompile, caller, separate target, IAT/data query or Ghidra mutation was issued.

## Exact admitted database claims

| Field | Current metadata |
|---|---|
| Function-at / containing | Both ram:00BF6FF5 |
| Name | `_atexit` |
| Prototype | `int __cdecl _atexit(_func_4879 * param_1)` |
| Return / convention / parameter | `int`; `__cdecl`; one declared `_func_4879 * param_1` |
| No-return / thunk | false / false; direct thunk target null; recursive lookup false |
| Complete body range | One range BF6FF5..BF7006 inclusive, 18 addresses |
| Instruction-at / containing | Both BF6FF5, length 4, InstructionDB |
| Code-unit-at / containing | Same instruction, defined field null |
| Override | NONE; fallthrough override false |
| Default / effective flow | Both FALL_THROUGH; call/jump/conditional/computed/terminal flags all false |
| Default / effective fallthrough | Both BF6FF9 |
| Default / effective targets | Both empty; direct-call-target metadata empty |

The function range is metadata only; its body was not requested for decoding. The declared function-pointer type was not inspected. `_atexit`, `__cdecl`, the declaration and no_return=false do not establish authentic CRT behavior, actual return, argument cleanup, callback signature or successful registration. The entry flow metadata does not recover callee effects.

## Preserved conditional boundary

The accepted CALL at CC8D3F, bytes E8B1E2F2FF, targets BF6FF5 and encodes CC8D44 as its next/return address. Encoding a return address never proves that the callee returned.

The accepted POP at CC8D44, byte 59, conditionally gives ECX=DWORD[SS:U], ESP=U+4 modulo2^32 and nextEIP=CC8D45 under ordinary successful decoded 32-bit execution with entryESP=U and normal 32-bit stack. ECX=CD9410 requires **both** returnedESP=S-4 and unchanged DWORD[SS:(S-4)]=CD9410. Return or zero argument cleanup alone is insufficient.

The accepted RET at CC8D45, byte C3, conditionally gives EIP=DWORD[SS:V], ESP=V+4 modulo2^32 under ordinary successful decoded 32-bit near-return execution with entryESP=V and normal 32-bit stack. Other GPRs/EFLAGS are preserved relative to that unknown entry. No concrete stackword, target or cleanup convention follows. Post-BF6FF5 ESP/GPRs/EFLAGS/stackword remain UNKNOWN; earlier EAX=E1768C credit ends at BF6FF5 entry. Actual return, registration and initializer completion remain unproved.

## Finite portable proof

The frozen Source epoch is `9316daef07de49f6f0637442c435e990d15e83d0`. The closed src/include/cmake/CMakeLists census has 4217 paths, plus 77 named dependencies, 4294 actual Git blob occurrences and 11 commit/tree objects. Every selected current file matches its frozen Git blob exactly; full query specifications and complete matches are retained. Text matches include existing comments and other callers; they are not target-body evidence.

Relative to the predecessor's 639ba6fa83343dcfa263676b8a0f3d9aa903c5aa epoch, 4215 Source paths match exactly. Only `include/bsp/native_render_service_base.hpp` and `src/native_render_service_base.cpp` differ through separately accepted Application commit 770c31f60f4d9876fe7df02f37eecb41ea5366b8. Their old/new actual blobs and pins are retained. This comparison does not rebase earlier evidence or transfer build credit.

Root's physical Application Source3356 manifest is retained at 1267925 bytes, SHA256 d171fd187a6609a8be42fa29f39cb495ad8c3d0e3fd34b18f7470e734d8c602a. Its overlap comprises 3346 raw-equal Source files plus 5 CRLF-only variants: 3351 LF-equal paths. The five raw variants and pins are retained; only CRLF pairs become LF. Nine named/artifact pins outside this census remain opaque and unopened. There is no sound Source delta or inherited build/runtime credit.

The accepted predecessor commit 3a0eb8914f4e40aaeba43677a9e48475d1794583, its 235172237-byte core ZIP and 296862-byte delivery ZIP are physically retained and recursively replayed. All 19 core and 3 delivery helper pins are checked; one shared helper yields 21 unique predecessor helper files. The new proof has 22 physical execution/bootstrap helpers beside each ZIP. Physical hashes are verified before imports execute, and the strict audit is active before helper execution. Replay only reads its supplied ZIP, executes verified in-memory helper bytes, writes its designated result and rejects external reads/processes/network/native loads. Retained producer code never runs during replay. Original and distinct physical-copy guards PASS with matching semantic results.

Core archive: `259858593` bytes, SHA256 `e9a5dd0ee4a443c8a28680cd243b13f839a057ae2044fdc92f3f6013369bd22f`, 8587 entries. Replay from the delivery directory with `python Tools/guarded_replay.py evidence.zip --output result.json`. The report records every helper pin. An actual-Git delivery archive follows the immutable two-output commit and proves exact current/staged/committed output equality; its pin is reported separately to avoid circular hashing.

## Separately gated continuation

The smallest candidate is exactly BF6FF5..BF6FF8, four code bytes plus context 0 instruction text at BF6FF5, after Root separately admits that packet and a fresh typed gate confirms its bounds. This is a proposal, not authorization. BF6FF9, the rest of the body, any callee/target, IAT/data, callers and decompilation remain excluded. No Source implementation, CMake, test, probe, build, game, OS or audio action is credited here.
