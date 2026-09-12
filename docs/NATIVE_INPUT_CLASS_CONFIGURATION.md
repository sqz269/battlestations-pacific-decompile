# Native input class configuration

Addresses: `00A917E0` (packet `orch4_native_input_class_configuration_af`).

`configure_native_input_class_00a917e0` configures the existing actual F8h
backend allocation. It creates no manager, device, publication or parallel
container. Source is in `native_input_class_configuration.hpp/.cpp`.

| Entry | Original interface | Coverage |
|---|---|---|
| A917E0 | ECX backend; stack DWORD class, requested count, optional actual10h identifier-vector pointer; no EDX input; no consumed result; RET0Ch | Complete game body A917E0..A91899 in the stated storage domain |

There are two exits: RET0Ch at A9186D..A9186F and A91897..A91899; end exclusive
A9189A. The owned listing has no missing starts, omitted instructions or tails.
The new C++ ABI is not a binary replacement. No x87 operations or owning EH
state occur in this game body. Source exceptions propagate without rollback.

## Storage and order

Existing A91570 calls the three A914C0 class constructors, then stores zero to
each requested-count word at A915F9. A914C0 initializes the six vector pointer
fields; A90DC0 later frees accepted-ID storage before active-pointer storage.
The class record is backend+68h+class*24h: requested count+0, actual10h active
header+4, actual10h accepted-ID header+14h. Each header is an untouched DWORD
followed by begin, end and capacity. Fixed device slots remain at backend+4.

1. A917FB writes requested count before any check, erase or allocation.
2. A917FE captures active end, then compares current begin. After a returning
   invalid-parameter handler, A9180C captures begin and compares current end.
   A91824 erases this captured range. A handler can change the header between
   these reads; this is not an unconditional `end=begin` substitution.
3. A9182C..A91852 repeats that sequence for the accepted-ID vector.
4. A91857 tests the optional vector pointer after both erases. Nonnull assigns
   its complete DWORD sequence, independently of requested count. In particular,
   a source alias of the destination has already been erased before assignment.
5. Null source appends exactly requested-count copies of FFFFFFFF. TEST/JBE at
   A91874/A91876 tests zero, not a signed positivity bound; the count stays raw
   DWORD bits. A91887 appends one word each iteration.

No active device is selected here. Active pointers are borrowed: clearing them
does not decrement a reference or delete a device. There is no dirty+D4 store,
callback+D8 call, publication change, SDK invocation or mouse/rumble write.
Later A918A0/A91620 performs activity-driven activation against these filters.

## Recognized library boundary

| Native site | Callee | Contract |
|---|---|---|
| A91806, A91814, A91835, A91842 | BF6713 | Actual returning CRT invalid-parameter service |
| A91824 | A90D00 | Checked active-pointer range erase; ECX actual header, five stack DWORDs (output iterator, first owner, first pointer, last owner, last pointer); RET14h |
| A91852 | 4685A0 | Same checked DWORD range erase signature; RET14h |
| A91862 | 4F60E0 | Actual10h DWORD-vector assignment; ECX destination, one stack source pointer; EAX destination ignored, RET4 |
| A91887 | 442190 | Append pointed DWORD; ECX accepted-ID header, one stack pointer; RET4 |

These are stateless source storage contracts, not imported/reconstructed STL
bodies or new library naming claims. Erase uses the captured iterators, current
end and real `memmove_s`, preserving capacity. Append reuses the existing
`insert_input_active_pointer_storage` contract on DWORD bits. Assignment reuses
capacity when sufficient; otherwise it frees the old buffer, clears the three
header pointers, allocates in `singleton_lifetime_allocate/free`'s domain, then
reloads the source endpoints for copying. No second ownership or typed vector
is involved. Invalid parameter and copy calls really may return.

Supply complete readable DWORD ranges, valid iterator owners and correctly
sized backing allocations, including after callbacks. Source allocation/error
transport and the reused 3FFFFFFF-element limit are explicit host contracts;
original CRT heap, new-handler frame aliases, STL instruction trace, exception
type identity, arbitrary malformed storage and FH3/SEH equivalence are not
claimed. The full nonnull branch is implemented, although all four observed
native callers pass null.

Read-only dependency qualification: 4F60E0's export omits 4F61C4..4F61C6 after
free at4F61BF; bytes `83 C4 04` prove ADD ESP,4, then body continues through
RET4 at4F6202..4F6204 (exclusive4F6205). 441860 similarly omits
441961..441963 after free44195C, with the same verified bytes; full final
RET10h is441A11..441A13 (exclusive441A14). These unowned library omissions
were reported to the integrator. The worker did not mutate Ghidra or claim
those library routines.

## Application bridge

All incoming sites were read. OnInitOnce (4DD5B0) reloads F8BBF4 before each of
4DD6B4, 4DD6C5 and 4DD6D6 and passes `(0,1,null)`, `(1,1,null)`, `(2,1,null)`.
They follow Xbox-compatibility application at4DD6A3 in the first-time arm.
RebindPrimaryInput (67C970) calls at67C9C6 with `(2,1,null)` after optionally
removing the captured first active gamepad through A90EE0. It then writes
E1987C=1. Current source `game_entry.cpp` exposes `on_init_once(true)` as a
required service; this packet does not implement its unrelated Lua/UI work.

The primary bridge must call this API on the current canonical F8BBF4 for each
native call, retaining those reloads. It must not seed alternate class counts
at A982D0 construction or substitute a semantic InputFocusBackendState.

## Verification

Verified bsp.gpr /battlestationspacific.exe for the read-only analysis/export
batches. `verify-seeds` matched all8 native seeds. Win32 Release build and both
existing CTests passed. `reports/native_input_class_configuration.json` carries
all8 owned direct CALL sites and all4 incoming CALL sites for the mechanical
audit. No permanent tests were added.

One ignored manifested actual-storage probe passed: requested count precedes
the real returning CRT handler; captured erase retains a post-handler suffix;
three wildcard words, three explicit IDs, capacity reuse and self-alias clearing
work; fixed slots, dirty and callback words remain untouched; existing class
destructors return all vector fields to zero. Runner:
`local/run_native_input_class_configuration_probe.ps1 [-PrimaryWorktree path]`.
This is storage/source validation only; no SDK/window/poll/force/gameplay or
original binary ABI validation was performed.
