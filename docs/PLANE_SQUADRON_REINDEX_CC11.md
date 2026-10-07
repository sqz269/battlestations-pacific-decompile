# Actual squadron reindex entry

`native_plane_squadron_reindex_007ed260` preserves the entire native x86 body:
`007ED260..007ED375` end exclusive, 277 bytes and 83 instructions. Its compiled
MSVC Win32 function is byte-identical to the installed PE and saved Ghidra
program, SHA256 `34c211366b9eae72404ec5d797ff97716b371b4a96e986c57c5c2ea5d1f4887d`.
There are no calls, absolute globals, x87 operations or COFF relocations.
The two original LEA displacement encodings and near JMP are emitted explicitly
because the assembler otherwise selects shorter equivalent encodings.

The raw entry takes the actual squadron in ECX, has no stack arguments and
returns with RET. EAX, EDX and flags are native scratch; EBX, ESI, EDI, EBP and
ESP are restored. The new C++ declaration uses fastcall only to place the
actual receiver in ECX; it does not establish a callable original class table,
constructor, higher caller, world service or lifetime policy.

The first pass reads the current signed count at +3CC, stores successive
ordinals in each current plane's +9D8, reloads that member, reads its old +9D0
index and marks the corresponding local slot. The loop rechecks the actual
count. The second pass assigns leader index zero, then scans odd and even
candidates with signed comparisons. Negative slot words are already assigned;
zero and positive words are available. Adjacent candidates retain the old
positive index's parity before selection. Every represented count/member
reload and index/ordinal store remains in its original instruction order.

The five-word local table does not bound the scans. After the second-pass EDI
push, slot five is the actual return address at ESP+24, and slot six is the
following caller word at ESP+28. Negative following words can continue the
scan to later caller storage. The raw implementation retains that physical
stack behavior. The detached array algorithm in `src/plane_formation.cpp`
clamps candidates and cannot replace this entry for that domain. It remains
available under its existing separate interface; no adoption is inferred.

Ordinary storage requires a live, stable, coherent squadron count and member
cells, writable live plane fields, disjoint backing, and old indices 0..4.
The connected fixture uses nonpositive counts and positive counts up to five.
Invalid indices can overwrite the original frame, and malformed members can
fault; no new guards, defaults, capacity policy or synchronization are added.
Caller storage must remain readable through each signed scan. Actual class
admission, faults, concurrent mutation, larger counts and gameplay remain
unvalidated. Byte identity alone does not establish their safety or semantics.

One unique component family compares the unchanged, unrelocated original
277-byte body with the new entry on the SAME live squadron and five plane
objects. It restores the initial backing between runs and uses one actual
assembly CALL site and one C++ invocation site. The controlled caller words
are -1, -1 and zero, so the five-member even scan reads beyond the first caller
word instead of receiving an invented local-array default. The odd scan reads
the actual nonnegative executable return address. The vectors `[0,1,2,3,4]`
and `[0,2,1,4,3]` exercise those distinct paths. Nonpositive counts skip members,
and a one-member case demonstrates the leader's unconditional zero assignment.

The final strict `/W4 /WX /fp:strict /O2 /Gy /MD` Win32 run passed 32 checks
across five cases. It compares complete squadron/plane backing and guards,
expected actual publications, volatile registers and flags, preserved
nonvolatile registers, exact stack restoration and genuine following words.
There are two fresh TUs, two project includes, 213 actual host headers, seven
searched host libraries and four selected Hostx64/x86 tools pinned before and
after. No BSP support library or bridge is required. The manifested executable
and whole COFF receipts are in `local/cc11_primary_squadron_reindex_run03/`.

Attempt 01 is retained: its fixture read the return word after PUSHFD overwrote
that location and compared two outer stack depths. Corrected attempt 02 passed
the same cases but the assembler shortened three original encodings. Attempt
03 preserves the original encodings, passes the same family, and establishes
complete code identity. Counts are not combined across attempts. No old fixture
family or tracked test was added or replayed.

The caller-domain audit separately qualifies actual incoming saved registers
and message-pointer words. The controlled component frame does not close those
callers, promotion/removal, class dispatch, arena ownership or game behavior.
The primary integration receipt records the full repository build, existing
CTest results, saved annotations, exports and index refresh when completed.

## Primary integration

Complete raw 277-byte/83-instruction entry, Source code byte-identical, no relocations/bridges. Signed slot scans retain actual return/following caller words, fresh member/count loads and exact publications. One unchanged-original controlled real-frame family passes 32 checks/five cases, full storage/registers/flags/stack. Source array algorithm remains separate. Actual original caller frames/class/fault/concurrency/world/game remain unbound.

Main build `fffe59921` passed the full Win32 build and all three existing CTests. The independent primary fixture used 2 fresh TUs; its pinned receipt is `local/cc11_primary_squadron_reindex_run03/inputs_after.json`. Saved annotation, refreshed export and snapshot/index evidence follow in the report.
