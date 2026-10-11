# CRT unwind-dispatch direct-target entry: primary review

Root accepts the sole admitted byte at `00C07623`: `55`, `PUSH EBP`.
For current entry ESP=T and EBP=B, completion writes current B to
`DWORD SS:[T-4]` and sets ESP=T-4 modulo 2^32. EBP and flags are unchanged.

Root independently checked the manual register index and actual detailed
Capstone operand, and manually decoded the retained caller E8 at `00C07A65`:
`E8 B9 FB FF FF`, displacement -1095, numeric target `00C07623`, return
`00C07A6A`. The positive 228-byte caller comes from the exact immediate Git
report selector, not a fresh Native query or an earlier archive.

Root also inspected a concrete default route from the first opaque call's
returned current Q0/F0 through eight argument pushes, with no intervening
call or EBP write. Conditional on its actual branch predicates, entry
T=Q0-24h, B=F0; this PUSH yields ESP=Q0-28h and stores current F0 there.
Each memory argument retains its instruction-time value and aliases.
This supplies no preserved original frame or eventual callee-return contract.

Root checked all 87 archive entries, 20 actual finite Git objects, eight
positive input blobs, exactly two worker output blobs/index entries and
the distinct physical archive copies. The complete 26,898-byte replay body
was read; the unchanged pinned auditor was already reviewed. Root's separate
strict replay passed with one archive read, one pinned code execution, one
exclusive receipt write and no denied event. Root's gate producer, raw
preimages and disclosed two offline directory/launch preparation failures
match the worker archive. There were no worker Native/helper failures and no
Root review/decoder/replay failure.

The saved `FindHandler` owner and prototype remain metadata: one 854-byte
range `00C07623..00C07978`, of which only the first byte was read. All three
typed captures preserve bsp.gpr, `/battlestationspacific.exe`, epoch 98 and
count 64,730. The remaining 853 bytes, handler selection, saved types,
Original FH3 and ABI, original frame, ownership and runtime remain unproved.

Worker commit: `9722731b58a2628d60c501484ec255b5d160b326`.
Worker frozen commit: `797b72c27038d2d146d99e22a227a63c85f766d9`.
Archive SHA-256: `e16df5501844ce63e3c1dead75a961baa9b6126d0facfb4349878612f7a494d7`.
Exact pins and receipts are in the matching PRIMARY report and Root-owned
`local/cc12_application_crt_unwind_dispatch_direct_target_entry_primary/`.
Source subtree IDs describe the worker's frozen revision only. This packet
changed no Source and ran no build, test, probe, game, OS audio or Ghidra write.
