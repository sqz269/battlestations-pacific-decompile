# CC12 locale exit tail: complete typed metadata gate

Exact typed queries cover every stored-body byte BF7000..BF7006. All seven before/after raw pairs match; project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, identity, epoch 98 and count 64730 remain unchanged. Every byte has the same containing BF6FF5 `_atexit` owner, non-thunk and no-return false, with exactly one 18-byte range BF6FF5..BF7006. Function-at-query is null at all seven bytes.

| Instruction/code unit | Length | Default and effective flow | Fallthrough |
|---|---:|---|---|
| BF7000..BF7001 | 2 | FALL_THROUGH | BF7002 |
| BF7002..BF7003 | 2 | FALL_THROUGH | BF7004 |
| BF7004 | 1 | FALL_THROUGH | BF7005 |
| BF7005 | 1 | FALL_THROUGH | BF7006 |
| BF7006 | 1 | TERMINATOR | null |

Both instruction/code-unit at-query fields are null at interior bytes BF7001 and BF7003, while their containing units remain the two-byte units shown above; instruction-flow is null at those interior queries. At all five starts, override is NONE, fallthrough override is false, and default/effective targets and direct-call targets are empty. The units exactly cover the seven-byte tail and none crosses its boundaries.

These are stored listing and flow metadata facts. No code, instruction text, mnemonic, operand, data or stack effects were read or inferred. TERMINATOR does not identify a return opcode or prove actual execution, termination, return or cleanup. Prior NEG/CALL effects are not extrapolated into the tail. Incoming EAX, W/K, post-callee ESP, callee return/cleanup, stack preservation, initialized locale ownership, CRT authenticity and actual Source ABI/runtime remain unknown.

The actual gate summary and all fourteen raw-capture pins were sent to Root before sealing. The summary is `local/cc12_locale_exit_tail_metadata/Native_gate_summary.json`, 17781 bytes, SHA-256 `9e40482f42cf97345a891bd8c5f2097318f66e6d40222159a7f9855d34f471c2`. Any operand reads require a separate Root review and are outside this packet. No successor or operand scope is automatically admitted.

Accepted predecessor `b5d57f94aa74078a310f1dc8c6d8fb65bb3bbdf4`, its exact core/delivery archives, all 32 predecessor helper bodies and nine pinned Root proof artifacts are physically retained. The current 34-helper archive passes original and copied recursive replay under the active one-supplied-ZIP guard. Producers are never executed during replay. Inherited target-function objects remain wholly opaque; fresh tail queries are site-only, while the complete inherited proof is not strictly site-only.

Current Source epoch54f0 contains 4217 closed paths, 4306 actual Git blobs, 89 named dependencies and 11 commit/tree objects. All Source paths equal predecessor bf866 and accepted f203. Original575 remains separate with only the accepted full-shell header/TU pair differing. Root physical Source retains 4212 raw-equal paths and five pinned CRLF-only variants, equal after that exact normalization. No Source evidence rebase or build/runtime transfer occurs.

Only this document and `reports/cc12_locale_exit_tail_metadata.json` are committed. Immutable delivery and helper/archive pins are in `local/cc12_locale_exit_tail_metadata/next_worker_handoff.json`. No GPR repair/mutation, C++ change, tests, build, Native probe executable, process/OS/audio/game action or broader Native query occurred.
