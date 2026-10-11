# Independent review of the C059A8 entry

Root accepted worker commit `6058cfdc691fdb4308695ed3b1275dbfd18b97ff` after independently checking the sole admitted byte `51`. Root derived `50+1` as `PUSH ECX`, a 32-bit explicit ECX read with separate implicit ESP read/update and no arithmetic EFLAGS change. Fresh actual detailed Capstone 5.0.7 agrees with the retained worker decode. Only on successful execution with renewed actual current SS.B=1 does incoming ESP=S become S-4 modulo 2^32 and current SS:newESP receive LE32(current ECX). No saved descriptor, frame, argument role or earlier opaque-call preservation is assumed.

The accepted BFBA12 physical CALL supplies the numeric incoming edge. Saved owner metadata contains six disjoint ranges totalling 399 addresses. C059A9 and the remaining 398 addresses, all gaps, target/provider cells and prototypes remain unqueried. Saved library labels and no-return metadata do not authenticate behavior.

Root checked the complete physical delivery archive and all 480 pinned entries, compared 20 distinct objects with actual Git, verified eight positive inputs and exactly two output paths, and checked all ten current working/index/LF identities. All 170 copied Root gate artifacts equal their physical originals. The original and worker copy have distinct physical identities and identical bytes. Source subtree identities remain unchanged; their contents were not read.

Root read the complete physical pure helper and guard before execution and made another actual archive copy. The unchanged pinned helper passed its first Root strict execution: exactly one ZIP read, one exact-code-object execution and one exclusive receipt write, with no denied events. The receipt equals both worker delivery receipts. No archived producer executes and no earlier proof archive is recursively consulted.

Root gate, inspection, final review and strict replay had zero failures. The worker retained one admission-helper TypeError before phase-2 Native queries, the original failed producer/prepin/result/empty stdout/552-byte stderr, and the separate corrected invocation. It also disclosed one failed rg inspection and one conversation display truncation; those conversation-only outputs are not claimed as separate original local streams. All eleven actual Native commands and four worker strict replays passed first time. These distinctions remain in the report.

This documentation/report integration changes no Source and adds no Native query or Ghidra write. No new build, test, probe, Original/game/audio or runtime execution was performed. Entry evidence does not establish full-owner behavior, ABI compatibility or startup/gameplay equivalence.

Machine-readable pins and review: [cc12_crt_dispatch_second_direct_callee_entry_primary.json](../reports/cc12_crt_dispatch_second_direct_callee_entry_primary.json).
