# October 18 reconciliation lane

Packet: the 96 rows in coordinator `local/codex_tail_R.txt`. Owner: agent/oct18-rec.
No game hosts or original installation are modified; this is reconstruction accounting
and bounded source work. Descriptive names remain hypotheses. Build evidence does not
establish original ABI compatibility or game validation.

## Progress board correction

Baseline at origin/main 21b82decf: 203 remaining (all segments), 19,994 game functions
done, 20,197 live game targets. The history parser remembered the first fragment at an
address and discarded every later function record. This hid whole bodies already
recorded in the ledger, especially naked MSVC definitions the source-name regex misses.
`first_ledger_records` now promotes a fragment when the first whole-function record
appears, using that function record's commit date and author. No duplicate implementation
or duplicate function record is required. A focused synthetic history checks promotion,
its timestamp, a fragment that stays partial, and an already-complete function.

## Compiler and library exclusions

The following original scalar-deleting destructors are compiler-generated wrappers:
00403610, 00403C60, 004041F0, 004042A0, 00405FC0, 00406210, 00406240, 00406270,
00406290, 004062F0. Exports show an ECX object, stack deletion-flags byte, conditional
`_free` under flag bit 0, and the returned object. The SAP variants perform member
cleanup or call the existing destructor first. This exclusion applies to those
generated deleting entries, not to the in-house Dyn physics implementation.

007512B0 and 007512F0 are telemetry library scalar-deleting entries: RTTI identifies
`iostdnet::ZMemoryPooledObject` and `iometrics::ZMetric::IStoredData`; both restore the
telemetry base vtable and conditionally call TELEMETRY_iostdnet_00A51E70. They are
outside the game-code contract even though RTTI tags obscured that classification.
Evidence: per-address `exports/bsp/functions/<address>/decompiled.c` and assembly;
existing inventory comments and direct callee identities. No Ghidra annotations changed.

## Missing Dyn/Mit bodies

`src/oct18_rec.cpp` / `include/bsp/oct18_rec.hpp` supply thirteen complete entries:

| Address | Body and original ABI |
|---|---|
| 00403C40 | Empty virtual method; ECX receiver, one stack word, RET 4. |
| 00404540, 00404550 | Empty terrain virtual methods; ECX receiver, three stack words, RET 0Ch. |
| 00405AA0 | Empty triangle virtual method; ECX receiver, RET. |
| 00405BF0 | Triangle support point; ECX shape; float output/direction and float transform on stack, RET 0Ch. |
| 00405D80 | Double-input/output support point; same stack contract and float transform, RET 0Ch. |
| 00407750, 00407BB0 | Reclaim empty pool pages; ECX pool, RET. Explicit host free binding replaces CRT call. |
| 00408300 | Store ECX dword to [EAX], RET; explicit pointer/value C++ interface. |
| 00409D00 | Load [EAX] to EAX, RET; explicit pointer C++ interface. |
| 00408E00 | Free non-null [EAX], RET; explicit header/free-binding interface. |
| 0040AB80 | Clear three dwords at ECX, leave EAX=receiver, RET. |
| 0040AB90 | Free non-null [ECX], RET; explicit header/free-binding interface. |

The empty virtual methods are real in-house entries, not inferred library exclusions.
The pool routines remove only pages whose free-stack depth is 128, move the last
table entry into the gap, rewrite all 128 moved slots' page-index trailers, retry
that slot, then find the first page with nonzero depth. They do not acquire locks,
shrink the page table or clear freed vector headers. Layout is the existing proven
`cc13_dyn2::MitPoolAllocator36x128` header, also shared by the 20-byte pool.

The two triangle routines select the greatest projected vertex, then transform that
vertex. Source retains all original x87 instructions and float spills. The double
variant narrows direction components to float and widens float-spilled outputs;
replacing it with double arithmetic would change its behavior. JBE branches retain
original equal/unordered selection. Named inline assembly was decoded from PE bytes,
cross-checked with both complete pseudocode exports and their assembly; no absolute
data or call targets occur. EDX dummy parameters adapt thiscall's stack layout.
No original game integration or runtime/ABI equivalence is claimed.
MSVC Win32 compilation and both existing CTest groups pass. Extracting the two
triangle symbols from the resulting COFF object confirms exact whole-body PE byte
identity: 393 bytes at 00405BF0 and 469 bytes at 00405D80. This is static proof;
the compiled bodies have not been used by a running game.
A focused manifested Win32 probe also passes both pool layouts through the same
mixed-page case: two empty pages including the moved page, one partial page,
all 128 trailer indices rewritten, first-free hint recomputed, capacity/lock retained.
Thirteen naming records and Ghidra plate comments were updated under the annotation
write lock; prior values are in `local/ghidra-annotations-20261018T230059Z.json` and
the project was saved. Existing class/virtual-slot names were retained; only the
two previously generic single-word register helpers received semantic names.

## Existing bodies reconciled without rewriting

00408A78 is Ghidra's split continuation of the already complete logical 004089E0
growth routine. `finish_growth` in `src/native_legacy_sbo_string.cpp` covers its copy,
old-buffer release and header/terminator commits. It is not independently callable:
original EBX preserve-count and parent EBP/EH frame are live. The new record explicitly
points to that parent coverage; existing fragment and original function boundaries stay.

008F81F0 is `run_win_main` in `src/winmain_startup.cpp`, the existing full startup
control flow through injected platform/application services. Original stdcall four
arguments are unused; EAX=0, RET 10h. The documented safe std::string path buffer and
host-bound COM/language/EH interfaces remain limitations; this is source reconstruction.

00AD2750 already has a complete raw-storage getter in `src/native_gui_frame_box_factory.cpp`.
Its ledger record was introduced only by a merge and therefore absent from the board's
non-merge history. Refreshing that existing record with reconciliation evidence gives
it reviewable provenance without duplicating its body. Original ECX box, signed16 stack
index, EAX record, RET 4; checked-invalid-parameter and post-callback pointer reloads remain.
