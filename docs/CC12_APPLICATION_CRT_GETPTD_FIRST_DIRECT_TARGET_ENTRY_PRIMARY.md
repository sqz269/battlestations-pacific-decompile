# Root review: CRT direct target exact entry

Root accepts the exact one-byte entry at `00C05070`, selected by the admitted physical call at `00C051C7`. Worker commit: `65b1445b785dc80960e8e51af764a568c793f306`.

Root independently checked both complete typed responses before byte admission: exact one-byte InstructionDB aliases, saved non-thunk/no-return-false owner, complete fifty-byte ownership range, ordinary fallthrough, project/program and epoch 98/count 64,730. Paired `56` reads, context-zero text, independent opcode-family decoding and actual Root Capstone 5.0.7 agree on `PUSH ESI`.

For normal local execution in the captured 32-bit environment, current ESI is pushed through SS after decrementing ESP by four. ESI and flags remain unchanged by this instruction; memory aliasing and faults remain possible. `C05071` and the remaining saved body are unread. No routine behavior, TLS interpretation, return/argument type, frame binding or original ABI is established.

Root reviewed the full pure parser and verified all 79 manifest members, retained producers/preimages, eight frozen actual Git payloads, twelve positive immediate paths and exact working/index/committed bytes for two outputs. The stricter Root audit passed with one supplied ZIP read, one receipt write, one pinned helper execution and no additional audited effects. Stored Capstone records are replay evidence; Root's separate review actually ran Capstone. Prototype interpretation stops at four metadata prefix lines; the automatic companion suffix remains opaque.

The companion report pins Root's gate, audit and actual Git/decoder receipts. This acceptance grants static entry evidence only. Source, build, original ABI, startup and gameplay remain unvalidated. Source IDs carry frozen worker revision identity only, without content census or equality to current Root Source.
