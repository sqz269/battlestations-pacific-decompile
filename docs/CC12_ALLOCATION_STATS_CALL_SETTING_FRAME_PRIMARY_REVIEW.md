# Allocation-stats CallSettingFrame primary review

Root accepts the ordinary action-entry frame equation from worker commit
`5351070b0b29468a2404d5d532b68e33a232da77`: `__CallSettingFrame@12`
at `00c07b10` sets action EBP to its raw second argument plus twelve bytes.
It saves its own frame, restores it after the action, reinstalls the action-return
EBP for the second notification, and finishes with `RET 0Ch`.

Root independently replayed 448 pin occurrences, 88 complete Git/current inputs,
238 immutable payloads in 239 ZIP entries, and all 105 authorized original-image
code bytes and 50 decoded instructions. None of the audited current Source inputs
differed. The saved metadata spans modification epochs 30 and 32; it is retained
evidence, not a fresh assertion that every response was captured in one epoch.

The actual notification entry is `00c16870`, whose nine-byte prefix preserves
EBX/ECX and jumps to `00c16884`. The admitted twenty-byte suffix stores the code,
EAX and EBP at `00e16838`, `00e16834` and `00e1683c`, respectively, preserves the
ordinary caller registers/flags, and returns with `RET 4`. Those destination
addresses were identified by the instructions; their data were not read.
The intervening `[00c16879,00c16884)` is eleven bytes and was excluded. The
worker's correction of the earlier five-byte prose estimate is preserved.

The existing C++ `___NLG_Notify` leaf models a separate stack-code entry at
`00c16879`; it does not establish the register-code entry at `00c16870`.
Upstream `00c069a2` still calls the unopened SEH prolog at `00c07c00`, so its
post-prolog EBP argument coordinates and association with the OS registration
remain unresolved. The startup action's physical epilogue, fault behavior and
complete Native FH3 composition also remain unresolved.

No C++ Source, build, ABI, startup or gameplay credit is added by this review.
No Ghidra annotation, listing repair, flow change or project mutation occurred.
See `CC12_ALLOCATION_STATS_CALL_SETTING_FRAME_GATE.md` and
`reports/cc12_allocation_stats_call_setting_frame_primary_review.json` for the
full retained worker evidence and independent Root replay.
