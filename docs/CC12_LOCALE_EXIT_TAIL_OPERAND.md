# CC12 locale exit tail: bounded operand evidence

The fresh gate passed at every byte BF7000..BF7006 before the exact seven code bytes and five context-zero instruction texts were read. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, identity, epoch 98 and function count 64730 remained unchanged. All seven full typed metadata pairs, both code captures and all five text pairs match. The containing BF6FF5 `_atexit` owner remains non-thunk/no-return false with one complete 18-byte body BF6FF5..BF7006. The capture is closed.

| Address | Physical bytes | Manual and Capstone x86/32 decode |
|---|---|---|
| BF7000 | 1B C0 | SBB EAX,EAX |
| BF7002 | F7 D8 | NEG EAX |
| BF7004 | 59 | POP ECX |
| BF7005 | 48 | DEC EAX |
| BF7006 | C3 | RET, near, no immediate |

These five complete units exactly occupy the seven-byte tail. Manual opcode/ModRM analysis and independent Capstone 5.0.7 decoding agree with Ghidra text. Physical byte SHA-256 is `95149f52065d2fc5f2cd1aa3af9f7cdabd58e180dd31ce3456f066bc6284f13f`. C3 supplies physical evidence for the near RET at BF7006.

Let C be the unknown CF bit and S the unknown ESP at BF7000. Only if the units are reached in order and complete normally under the stated 32-bit operand/stack model, SBB produces EAX=-C, NEG produces C, and DEC produces C-1 modulo 2^32. Thus EAX would be FFFFFFFF for C=0 or zero for C=1. POP ECX reads the unqueried dword at S; near RET reads the unqueried dword at S+4 as its destination. Together they would leave ESP=S+8 modulo 2^32. The decoded RET has no immediate stack adjustment. The report records instruction-local arithmetic flags and explicit preconditions.

The separately accepted preceding BF6FFE NEG can be composed only conditionally: if it actually executes on R and its CF reaches this tail unchanged, C=(R!=0), yielding FFFFFFFF for R=0 and zero for R!=0. R, actual CF, preceding callee return/cleanup, tail reach, both consumed words and the return destination remain unknown. No actual return, success/failure status, callback acceptance, registration, CRT authenticity, initialized locale owner or Source ABI/runtime behavior follows. In particular, the old CC8D44 POP still lacks actual returned-ESP and unchanged-word evidence. No target, caller, neighbor, data, pointer, stack or prototype was queried.

Root received the actual fresh gate, closed-capture summary and decoded-byte pins before sealing. `Native_gate_summary.json` is 7156 bytes, SHA-256 `ddeae1075e5c7a3eb2eb19c4f7f7003a89774a8566494319b44e442baa9ce999`. No additional Native read or automatic successor admission follows this packet.

Predecessor `7307485545258c317d9d0990e032472b39c0bfd2`, both accepted archives, all 34 predecessor helpers and ten pinned Root proof artifacts are physically retained. The 36-helper current core passes original and physically copied recursive replay under the active one-supplied-ZIP guard; Producers never execute during replay. Inherited target-function objects stay wholly opaque. Fresh captures are site-only; the whole inherited proof is not.

Current Source epoch dd1ce37f retains 4217 closed paths, 4310 actual Git blob occurrences, 93 named dependencies and 11 commit/tree objects. All Source paths equal predecessor54f0 and acceptedf203. Original575 remains separate with only the accepted full-shell header/TU pair differing. Root physical Source retains 4212 raw-equal paths and five separately pinned CRLF-only variants, equal after that exact normalization. There is no Source evidence rebase or artifact/build/runtime credit transfer.

Only this document and `reports/cc12_locale_exit_tail_operand.json` are committed. Exact current/staged/committed output bytes and actual delivery Git objects are retained in the sibling delivery proof. Immutable archive/helper/commit pins are in `local/cc12_locale_exit_tail_operand/next_worker_handoff.json`. No GPR repair/mutation, C++ change, tests, build, Native probe executable, process/OS/audio/game action or broader Native query occurred.
