# Locale table backing cleanup: primary acceptance

Root accepts the bounded 00435560 leaf. It loads the current member pointer
once and returns directly if null, without accessing count or capacity. A
nonnull captured pointer is passed unchanged to BF6989. On a normally returning
caller-cleaned free, the original POP ECX consumes its argument before RET.
The leaf makes no member writes or idempotent reset. Free target effects remain
unread; no meaningful whole-function return value is established.

The stored body contains 13 bytes across two ranges; the authorized 14-byte
hull includes the original unowned POP ECX byte at 43556C. The call retains its
saved CALL_RETURN/effective CALL_TERMINATOR override. The ordinary return graph
is recorded from original bytes without repairing listing or ownership. The
constructor-action null-table/capacity-32 outcome is conditional on actual
invocation with the required frame. FH3 frame setup, other actions, complete
owning unwind, ABI replacement and activation remain unproved.

Root copied and replayed the guarded ZIP, checked 4,233 actual frozen Git blobs,
13 commit/tree objects and all 4,217 closed Source paths. Current Root Source
matches this worker epoch. The 14 fresh native bytes/seven instructions were
captured at modification 83 before later independent annotations. Inherited
PE, constructor and lifecycle evidence receives no new artifact credit.

Root separately recorded provisional descriptive name BSP_TableBacking_FreeIfPresent
and appended evidence under the Ghidra write lock. Old name/comments are
journaled; the project is saved and three affected exports are refreshed.
Four complete before/after metadata responses differ only by the expected name
and modification 85 to 88. Prototype, body, listing, flow and no-return remain
unchanged. No C++, build, test, probe, OS or game result is added.

Next bounded evidence gate is 00402F70 section cleanup: metadata first and
complete hull at most 128 bytes admitted only after exact range ownership.
403970 unlink and actual FH3 frame setup remain separate dependencies.

Evidence: reports/cc12_locale_table_cleanup_primary.json.
