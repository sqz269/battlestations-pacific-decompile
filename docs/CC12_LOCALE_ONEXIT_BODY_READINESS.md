# CC12 onexit stored body: bounded operand evidence

The initial typed gate covered every byte BF6FB9..BF6FEE. Root independently reviewed its 108 raw responses, exact coverage and flow schedule, then separately admitted 54 code bytes and context-zero text at the 14 whole instruction starts. A fresh pair of complete 54-byte typed captures surrounds the operand reads. All four metadata batches, both code/text captures, project/program identities and count bookends agree at epoch 98 and function count 64730. Native capture is closed.

The same non-thunk/no-return-false saved `__onexit` owner has one complete stored 54-byte body. Fourteen units cover it without gaps or overlap; the other 40 queried bytes are instruction interiors. Manual opcode/ModRM/relative-operand analysis and independent Capstone 5.0.7 x86/32 decoding agree with all context-zero texts:

| Address | Bytes | Instruction-local decode |
|---|---|---|
| BF6FB9 | 6A 0C | PUSH 0000000C |
| BF6FBB | 68 F0 2C E0 00 | PUSH 00E02CF0 |
| BF6FC0 | E8 3B 0C 01 00 | CALL C07C00 |
| BF6FC5 | E8 9E 4A 00 00 | CALL BFBA68 |
| BF6FCA | 83 65 FC 00 | AND dword [EBP-4],0 |
| BF6FCE | FF 75 08 | PUSH dword [EBP+8] |
| BF6FD1 | E8 FB FE FF FF | CALL BF6ED1 |
| BF6FD6 | 59 | POP ECX |
| BF6FD7 | 89 45 E4 | MOV dword [EBP-1C],EAX |
| BF6FDA | C7 45 FC FE FF FF FF | MOV dword [EBP-4],FFFFFFFE |
| BF6FE1 | E8 09 00 00 00 | CALL BF6FEF |
| BF6FE6 | 8B 45 E4 | MOV EAX,dword [EBP-1C] |
| BF6FE9 | E8 57 0C 01 00 | CALL C07C45 |
| BF6FEE | C3 | Near RET, no immediate |

All effects are conditional on the relevant instruction being reached and completing normally under the stated 32-bit operand/stack model. If the two initial PUSH instructions and first CALL complete consecutively from unknown ESP W, they write 0000000C at W-4, opaque literal 00E02CF0 at W-8, and return site BF6FC5 at W-12. This supplies no descriptor or frame interpretation.

Each of the five external CALLs introduces an unknown returned state. Actual return, stack cleanup, ESP, EBP, EAX, flags and memory preservation are unverified. POP at BF6FD6 is therefore not established to consume the earlier pushed value. The BF6FD7 EAX store and BF6FE6 reload cannot be equated across the intervening BF6FE1 call. The final BF6FE9 call also leaves EAX and ESP unknown before C3. Near RET would consume the then-current unqueried stack word and increment ESP by4 if it completes; actual destination, return/status and final stack relation remain unknown.

The EBP-relative locations and literals are instruction-local operands only. They are not established as parameters, callbacks, frame locals, state slots or descriptors. No IAT, global, data or stack contents were read. All automatically supplied target-function objects remain wholly opaque: 20 occurrences across the four typed batches, with no semantic credit. No target, caller or neighbor was followed. Saved library identity and the earlier saved prototype authenticate no CRT, callback, owner or ABI behavior.

Root's explicit coverage/admission receipt is retained as `Admission/Root_listing_gate.json`, 14586 bytes, SHA-256 `a5c32068a36dce125dee928725051691c7d66cf26c3ef5312b60a9f91fe4b4f6`. Physical 54-byte SHA-256 is `7608521a2c02ea42523f647610e04e419867cc0280d686d18b595d6e3ca2215f`. The report retains raw metadata/code/text pins, the manual decode, independent Capstone operands and every unknown external-call boundary.

This standalone Native-only proof freezes HEAD `5af77761890d02c76a548dcf5da0cf074df6147a` and the actual src/include Git identities. Nine immediate prerequisite/tool files and six actual commit/metadata-tree objects are retained. No Source census, Source file read, inherited archive traversal, artifact comparison or Source/build/runtime transfer occurred. Independently running Source implementation work has separate provenance. Four physically pinned helpers pass original and copied single-ZIP guarded core replay; Producers never execute during replay.

Only this document and `reports/cc12_locale_onexit_body_readiness.json` are committed. Exact current/staged/committed output bytes and actual delivery Git objects are retained in the sibling delivery proof. Immutable commit/archive/helper pins and all four guarded receipts are in `local/cc12_locale_onexit_body_readiness/next_worker_handoff.json`. No prototype repetition, body decompile/disassembly query, GPR repair/write, C++ change, test, build, Native probe, process-status query or game/OS/audio action occurred. No successor is admitted.
