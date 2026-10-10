# Allocation-stats startup action tail primary review

Root accepts the conditional physical tail gate from worker commit
`4197702be973180574ba0ddde938797bf2580f05`. The authorized eight-byte original/live
probe at `00c86a3c` starts `POP ECX; RET`. Root independently replayed the exact
original window and decoded only those two bytes, stopping at RET; six probe
bytes remain uninterpreted.

Conditional on ordinary cdecl return from the retained `00bf65ac` free call,
POP ECX discards its pointer argument and RET consumes the original action
return word, returning to `00c07b37`. Root independently recomputed the ordinary
stack balance. This closes that physical continuation, without establishing
fault behavior or complete Native CRT/FH3 runtime equivalence.

The saved `Unwind@00c86a30` Ghidra body remains twelve bytes and ends with the
retained CALL_RETURN. `00c86a3c` remains unowned undefined one-byte DataDB; the
probe does not create or extend a function/instruction. There was no metadata
query at `00c86a3d`, so no current saved instruction there is claimed.

Root replayed 427 pin occurrences, 96 complete current/Git inputs with no
current differences, and all 237 immutable payloads in 238 ZIP entries. The
worker retained two typed replies at modification 32, nine GETs and one
read-only POST. The replacement completion worker added no Native reads or
queries; Root consumed only the already-authorized eight-byte probe window.

OS parent-frame association, free/fault behavior, FH3 identity, actual Source
startup allocation/publication ownership and runtime remain open. No C++,
build, test, Ghidra mutation, original ABI, startup or gameplay credit is added.
