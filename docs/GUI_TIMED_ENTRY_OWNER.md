# Canonical GUI timed-entry allocation and retirement

Addresses: `AA6F30`, `AA77B0`, `AA8B00`, `AD3A80`, `AA87B0`, `AA9730`.
Names describe hypotheses; new C++ interfaces do not replace native widget,
virtual-table or exception ABI. The existing source CRT allocation service is
used only for newly allocated, provenance-tracked backing and entries.

`GuiTimedEntryOwner` binds the existing owner's three `pointers_88_90` words as
data, signed count and signed capacity. Its maps hold allocation provenance and
entry lifetime phase, never a second live array or widget state. The initial
binding requires an empty header. It cannot adopt arbitrary native pointers.
The owner keeps the original live `D7A24C` constructor constant reference.

| Routine | ABI | Coverage |
| --- | --- | --- |
| AA6F30..AA6F8E | ECX header, signed capacity stack, RET4 | Complete valid-allocation reserve: signed minimum1, exact capacity*4, live count/backing copy, free before pointer/capacity publication |
| AA77B0..AA77FF | ECX header, signed size stack, RET4 | Nonnegative size: reserve if needed, zero growth slots, decrement count on shrink without deleting elements |
| AA8B00..AA8B7E | ECX widget, signed index stack, EAX entry, RET4 | Nonnegative nonoverflowing index, all three resize sites and original captured-slot publication before current-header reload |
| AD3A80..AD3B40 | ECX index, EDX borrowed widget, EAX entry, RET | Both alpha indices and inline index2 producer using actual 14h storage; other indices null |
| AA87B0 | ECX widget, float seconds stack, RET4 | Fragment AA884D..AA892C only: after child traversal, current timed-entry update/delete loop before listener tail |
| AA9730 | ECX widget, no arguments, RET at AA99B8 | Fragment AA97F6..AA9950 only: actual timed-entry drain, negative-capacity repair, final backing retirement |

Array backing and entries share `singleton_lifetime_allocate/free`: source
malloc/new-handler/retry semantics with equal native/host byte counts. Original
static CRT heap, handler state and native exception identity remain boundaries.
Allocation callbacks can alter the live header. Reserve reloads count/backing
after allocation; get-or-create stores to its original captured slot only if
that same allocation remains valid. Failed host metadata allocation is a C++
transport boundary. Later domain exceptions retain allocations and partial
state, without inventing native rollback or resumable exception frames.

The exact entry profiles and numerical update rules are in
`docs/GUI_TIMED_ENTRY_TYPES.md`. The borrowed layout token resolves through the
same owner runtime. Flags0 resets the profile and retains storage; explicit
flags1 storage return consumes the provenance before free. Shrinking a header
does not delete orphaned entries; their allocation remains tracked until an
explicit return. Destruction of an owner with such outstanding allocations
terminates, rather than freeing unknown live native work automatically.

The update fragment x87-spills the original seconds at the native call boundary,
calls actual AD39A0, and reloads the current slot after callbacks. A false result
deletes that current entry and clears the captured slot only after return.
Count stays live between iterations. This is not the widget's child traversal,
elapsed-time store, listener dispatch or complete current40 implementation.

The destruction fragment drains entries, then implements capacity<0 reserve1
and final resize0/free. It leaves the native backing and capacity words intact
after free. Separate retirement metadata prevents replay; repeated direct drain
is rejected and the widget lifecycle skips an already completed entry phase.
Negative count, wrapped allocation sizes and indexes outside physical allocated
objects remain explicit boundaries. No negative-index writes or overflowed
small allocation are made to mimic native memory corruption.

The existing Text scalar base tail calls this owner at its established entry
phase. Page retirement calls it after child retirement. Text's former blanket
nonzero-header rejection now checks actual allocation ownership. Pending clip
work and active timed update/drain reject scene release and scalar deletion
before destructive work. This does not establish the remaining generic native
base child-list/string/SEH ABI or a suspended outer page-loader implementation.

Validation is recorded in `reports/gui_timed_entry_owner.json` and the combined
batch report. A focused local fixture uses actual canonical Group/Model owners
and the real source allocator, exercising delayed alpha, inactive Section gate,
flags0 storage, negative-capacity repair and one-time retirement. It is separate
from native-byte execution, rendered UI and gameplay validation.

## 2026-09-12 timed and clip ownership integration

The combined library fixture now verifies producer indices, actual alpha dispatch,
countdown/deletion, the non-Section gate, flags0 storage, negative-capacity repair
and one-time header retirement. Active allocation/producer calls also block
owner destruction; the currently executing entry cannot be deleted by its widget
callback. A retained flags0 entry requires D5D204 before later final free.
AD3A60..AD3A7E is now defined from matching disk/Ghidra bytes and proves that
base deleting profile. These are host ownership guards, not native failure or
arbitrary reentrancy parity. Per-owner preflights do not establish atomic tree
retirement or protect every ancestor's borrowed continuation.
See reports/orch5_timed_clip_batch.json for combined build and source-fixture evidence.
