# CC12 locale exit target after-PUSH metadata

BF6FF9 is currently an exact 5-byte InstructionDB unit inside the database function at BF6FF5. Its default/effective flow class is UNCONDITIONAL_CALL, with edge address BF6FB9 and fallthrough metadata BF6FFE. This packet supplies database metadata only: no bytes, mnemonic, operands or new architectural transfer contract were recovered.

## Capture and qualification

The saved identity was verified as `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86:LE:32:default, image base 00400000. The bounded sequence was typed-before at BF6FF9, count-before, count-after, typed-after at BF6FF9. Raw typed responses and all four identity records match, as do count bytes. Actual modification 98/count 64730 were observed afresh; no inherited epoch was assumed. This is observed database consistency, not proof of exclusive access or runtime state. Native access is closed.

The typed endpoint also returned a target-site function label/flags object under `direct_call_targets[*].function_at_target`. **The whole capture is not strictly site-only.** That entire object stays raw in the pinned captures and is omitted from the qualified report. Root accepted only independently bounded BF6FF9/containing-function metadata and the returned edge addresses. Target-site behavior, ABI, ownership and successor admission receive no credit. No separate query of BF6FB9 was made, and no target-site value was used to expand scope. Raw response equality is an integrity check, not validation of those target claims.

The older full prototype capture remains separately qualified as not strictly entry-only, with its unsolicited statistics/hash/callee label raw and opaque. This packet issued no prototype, signature, instruction-context, text, byte, decompile/disassembly, caller, adjacent-site, IAT/data, pointer or target query. It performed no Ghidra mutation or repair.

## Qualified database observations

| Field | Current metadata |
|---|---|
| Instruction-at / containing | Both BF6FF9, length 5, InstructionDB |
| Code-unit-at / containing | Same instruction; kind instruction; defined null |
| Function-at-query | null |
| Containing function | BF6FF5, database name `_atexit` |
| Owner body metadata | One complete range BF6FF5..BF7006 inclusive, 18 addresses |
| Owner no-return / thunk | false / false; direct thunk target null; recursive lookup false |
| Flow override | NONE; fallthrough override false |
| Default / effective flow | UNCONDITIONAL_CALL; call true; jump/conditional/computed/terminal false |
| Default / effective fallthrough | BF6FFE |
| Default / effective target address | BF6FB9 |
| Edge origin flags | Present in both default and effective targets |

The target address is a database edge claim, not a decoded operand or observed transfer. Fallthrough metadata does not prove actual return or continuation. The owning function's name and flags do not establish CRT authenticity, a signature, callback policy or cleanup convention. No claim about target-site function ownership is made.

## Preserved conditional state boundary

The accepted BF6FF5 bytes FF742404 decode as `PUSH dword ptr [ESP + 0x4]`. With unknown entry ESP=W, entry memory M and K=DWORD M[SS:((W+4) modulo 2^32)], successful ordinary decoded 32-bit execution with a normal 32-bit stack uses the pre-decrement ESP for the source, then gives ESP=W-4, DWORD[SS:(W-4)]=K and nextEIP=BF6FF9. Other GPRs/EFLAGS are preserved relative to that unknown entry. No stack data were captured, no concrete callback/word is identified, and earlier EAX=E1768C is not imported into this independent callee-entry slice.

The prior CALL/POP/RET remain conditional. In particular, ECX=CD9410 after the prior POP needs **both** returnedESP=S-4 and unchanged DWORD[SS:(S-4)]=CD9410. Current metadata discharges neither hypothesis. W/K are not equated with previous caller, POP-entry U or RET-entry V symbols. No new BF6FF9 transfer, post-callee ESP/GPR/EFLAGS/stackword effect, actual return, registration or initializer completion is established.

## Frozen Source, metadata and portable proof

Source is frozen at `c939bad77217caff6a7bc8dab22481e0394ba6b6`: 4217 closed src/include/cmake/CMakeLists paths and 77 named dependencies, 4294 actual blob occurrences and 11 frozen commit/tree objects. All selected physical inputs equal their Git blobs. Complete queries and 3 bounded Source excerpts are retained. The BF6FF9/BF6FFE/BF6FB9 Source query has zero matches in this finite corpus; that is not a global implementation-absence claim.

Compared with the predecessor's febb7a6be871e15092832d58ad68a5717e246f48 epoch, 4215 Source paths match. The only differences are `include/bsp/native_render_service_base.hpp` and `src/native_render_service_base.cpp`. Five additional actual Git objects prove those two current blobs equal accepted worker edc97a7f6aad6b4382d7000ef1c56790dec45761. That worker's whole Source epoch differs from current Root and is not equated with it. Two actual commit/root-tree objects separately prove Root acceptance 3c3c469d85f9c6b3c85116991df346b95028bd74 and current c939 have identical closed Source objects across all 4217 paths.

The accepted Main transition from 8249586cd also includes eight docs/reports paths and `config/reconstruction/00be0000.jsonl`. The latter is accepted reconstruction metadata from Root 3c3c, with exactly two added lines and zero removals; it is separate from the two C++ Source changes. The exact 3295-byte zero-context Git diff, SHA256 fc98ef0b2e457ac7710475b6b1ba824105d17e4c3a8e2f1f27347990143b70e7, its added-line pins and old/new blob IDs are retained. Two actual reconstruction tree objects establish membership without reading or archiving the whole shard. Replay derives all 11 changed paths from actual before/after trees. The initial docs/reports-only remainder check rejected this shard; the retained comparison records its correct metadata classification explicitly.

Root's current Source3356 Base manifest is physically retained at 1267920 bytes, SHA256 c491d001e5c2cd1a1539b028503c1975b4fe1ec10991d47ab7061f18fa1179c2. Its overlap comprises 3346 raw-equal Source files plus 5 retained CRLF-only variants: 3351 LF-equal files. Only CRLF pairs become LF, with differing raw bytes/pins preserved. Nine named/artifact pins stay opaque and unopened. Older e89 and d171 manifests remain physical historical evidence at their own epochs. No sound Source delta, evidence rebase or historical build/runtime transfer is credited.

The predecessor's accepted 8249586cd commit, 284915822-byte core ZIP, 660307-byte actual-output delivery ZIP and all 24 original helpers are physically retained. Their full transitive ZIP/helper graph is recursively replayed. Original and distinct physical-copy core guards PASS. All 26 current helpers must remain in `Tools/` beside the ZIP. Their physical hashes are verified before imports execute; the audit is active before any helper code runs, only verified in-memory helper code executes, and replay reads only its supplied ZIP and writes only its result. External files/processes/network/native loads are denied. No retained producer runs during replay.

Core archive: 310139070 bytes, SHA256 `25fe84a41549404cd9551432fc242712fc98257110dcb97d00c6da0e93b59211`, 8612 entries. Run `python Tools/guarded_replay.py evidence.zip --output result.json`. The exact-two-output actual-Git delivery archive follows the immutable commit and proves current/staged/committed byte equality; its pin is separately reported. No C++/CMake change, build, test, probe, game, OS or audio execution is credited.

## Separately gated continuation

A possible next packet is a fresh BF6FF9 typed gate followed by exactly BF6FF9..BF6FFD inclusive, five code bytes and context 0 text at BF6FF9, **only after separate Root admission**. This proposal follows the qualified unit length, not target-site metadata. No successor operand/body read is authorized now. BF6FFE, BF6FB9, remaining body, callers, prototypes and all IAT/data/pointer reads remain excluded; full-callee effects and actual return/registration/completion remain unknown.
