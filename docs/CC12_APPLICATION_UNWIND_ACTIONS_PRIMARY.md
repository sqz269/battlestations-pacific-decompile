# Application unwind action operands: primary acceptance

Root accepts two complete eight-byte bodies at CC7230 and CC7238. The first
loads ECX from [EBP-18h] and jumps to 412430. The second forms ECX=EBP-14h
without a memory read and jumps to 411EE0. Neither adjusts the stack, pushes a
return address or changes arithmetic flags before its target. Both default
JUMPs retain saved CALL_RETURN overrides; no repair or callee read follows.

Matching the retained constructor slots requires EBP=ESP_R+20h for its saved
original receiver and EBP=ESP_G+20h for its guard record. A single binding
requires the two store-time stack pointers to coincide. The constructor's
complete retained epoch-79 body has no EBP operand; actual FH3 frame setup,
invocation, target cleanup and return behavior remain unproved.

Root copied 525 finite pins, passed guarded relocation replay, checked 8,476
actual current/prior Git occurrences and both closed Source trees, and verified
all 433 prior map pins unchanged. All 4,210 frozen Source paths match the worker
commit and current Root. The 16 fresh code bytes were captured at epoch 92;
constructor, handler, descriptor and map epochs remain distinct.

Correct Unwind@ names and stored undefined(void) prototypes describe saved
analysis metadata. Descriptive target names do not prove effects. A separate
metadata-only target packet may refresh bounds at 412430 and 411EE0; no target
body, descriptor suffix or handler expansion is admitted here.

Root appended two locked evidence comments, retained correct EH names and old
comments/ledger evidence, saved the project and refreshed six export files.
Four complete typed responses differ only by modification 92 to 95; bodies,
listing, flow, prototypes and no-return flags remain unchanged. Indexed callee
summaries are not instruction-level evidence, including the old BF6B43 edge.

No C++, build, test, ABI or runtime credit is added. Evidence is in
reports/cc12_application_unwind_actions_primary.json.
