# Independent review of the C26B57 entry

Root accepted worker commit `e56b396b1de3ce1a040c325a4887e54d48dfcf30` after independently checking the four admitted bytes `8B 4C 24 04`. They encode `MOV ECX,SS:[ESP+4]` with default 32-bit operand/address sizes, no prefix and no implicit stack operation. Root independently split ModR/M 4C and SIB 24, derived signed displacement +4 and default current SS, then compared fresh actual detailed Capstone 5.0.7 output with the retained worker output. On successful completion, the instruction reads the current memory dword into ECX and leaves ESP and EFLAGS unchanged.

The explicit ESP calculation does not establish SS.B. Composition with the accepted C05B6A near CALL requires its renewed current SS.B=1 predicate, successful execution, the same descriptor and no intervening changes. Under those conditions, pre-CALL ESP=T becomes T-4 and this MOV reads current SS:T. No argument identity, prototype or library service is recovered. Saved `__set_error_mode` is only metadata.

Root authenticated the physical delivery archive and all 154 pinned entries, compared 20 distinct objects with actual Git, verified eight positive inputs and exactly two output paths, and checked all ten current working/index/LF identities. The 43 copied Root gate artifacts still equal their physical originals. Source subtrees were checked by identity only.

The full physical pure helper and audit guard were read before execution. Root made another physical archive copy and executed the unchanged pinned helper under the reviewed guard. The first execution passed with exactly one ZIP read, one exact-code-object execution and one exclusive receipt write, `denied=[]`. The receipt equals both worker delivery receipts. Archived producers are inert evidence. Root added no Native query and wrote nothing to Ghidra.

Root gate, inspection, final review and strict replay each had zero failures. The worker reported zero Native, producer, decoder, seal and replay failures and disclosed three conversation display truncations. The retained records do not invent separate original local streams for conversation-only output.

Only C26B57..C26B5A is physically established here. C26B5B and the other 66 owner bytes remain unqueried. This documentation/report integration changes no Source. No fresh build, test, probe, Original/game/audio or runtime execution was performed; the prior Source validation remains separate from this bounded entry evidence.

Machine-readable pins and review: [cc12_crt_banner_first_direct_target_entry_primary.json](../reports/cc12_crt_banner_first_direct_target_entry_primary.json).
