# Ordinary retirement actions: entry metadata and exact operands

Two saved eight-byte body ranges, `00CC7250..00CC7257` and `00CC7258..00CC725F`, are covered by four instruction units of lengths 3, 5, 3 and 5. The exact sixteen bytes are `8B 4D E8 E9 D8 B1 74 FF 8D 4D EC E9 80 AC 74 FF`. Independent manual decoding, Capstone 5.0.7 and the four context-zero listing texts agree:

| Address | Physical instruction | Instruction-local effect |
| --- | --- | --- |
| `00CC7250` | `MOV ECX,[EBP-18h]` | Read one DWORD from the computed address into ECX. |
| `00CC7253` | `JMP 00412430` | Transfer control; no return-address push. |
| `00CC7258` | `LEA ECX,[EBP-14h]` | Form the address in ECX without reading operand memory. |
| `00CC725B` | `JMP 00411EE0` | Transfer control; no return-address push. |

Before their targets, both instruction pairs preserve EBP, ESP and arithmetic flags. This describes ordinary successful instruction-local behavior; no stack contents or execution were observed. Saved `CALL_RETURN` overrides at the physical E9 jumps produce effective `CALL_TERMINATOR` metadata and remain distinct from those opcodes. The saved `Unwind@` names, non-thunk/non-noreturn flags and body ranges are analysis metadata, not Original FH3 or complete-funclet authentication. Complete automatic target-function objects remain opaque.

The retained [ordinary retirement contract](CC12_APPLICATION_ORDINARY_RETIREMENT_READINESS.md) stores incoming receiver `A` at `[ESP_R+8h]` at BEA8CC and forms its guard at `ESP_G+Ch` at BEA8E8, with captured section at `ESP_G+10h`. Therefore matching the first action's address to that receiver spill requires `EBP=ESP_R+20h`; matching the second action's address to that guard requires `EBP=ESP_G+20h`, modulo 2^32. A common binding requires the two store-time stack pointers to coincide. Obtaining the original receiver value additionally requires the spill contents to remain suitable. Actual FH3 EBP setup, invocation, storage lifetime and aliasing remain unproved.

The existing [target-body contract](CC12_APPLICATION_TARGETS_BODY_PRIMARY.md) already establishes the reset target's receiver-table write and the guard target's captured-section/decrement/import-call schedule. Those are prior bounded contracts, not freshly queried targets. The guard's nonnull return still requires its external callee to remove the one four-byte argument; IAT contents, provider behavior and actual OS execution were not read or established here.

Root reviewed the initial two-site metadata before admitting all sixteen metadata sites, then reviewed that coverage before admitting the sixteen bytes and four texts. All 50 typed responses across four batches close unchanged at project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, epoch 98 and count 64,730. Two code reads agree exactly. All eight count calls agree. Full typed raw/base64 responses, complete CLI code outputs, the exact context-zero response and all command schedules are retained; code outputs are CLI formatted text, not raw HTTP captures.

The narrow ZIP pins all seven helper bodies, seven query-tool sources, three bounded prerequisite doc/report pairs, target configuration, both Root admissions, current captures and actual frozen Git evidence. Original and distinct-copy replays pass under a guard that permits only the supplied ZIP and denies post-preload imports. Replay manually redecodes the sixteen bytes and checks the recorded Capstone result; it does not run Capstone, Native code or a game process. External receipts verify fourteen prerequisite Git/physical inputs, frozen `src`/`include` identities, and exactly two staged/final output objects. No Source census or recursive old archive is included.

No action target, IAT, stack, data, neighboring instruction, prototype, decompilation or disassembly query was made. There was no Ghidra repair, Source composition, C++ change, test, build, probe, process, game, OS or audio action. This packet supports a separately reviewed Source policy only through its bounded argument-preparation and conditional slot relationships; actual EH frame, invocation, ABI and cleanup authority remain open.

Evidence: [machine-readable report](../reports/cc12_application_ordinary_retirement_action_entry_metadata.json).
