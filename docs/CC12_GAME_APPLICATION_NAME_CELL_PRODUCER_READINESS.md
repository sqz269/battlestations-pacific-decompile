# Actual Application and duplicated-name producers: bounded readiness

The existing duplicate/free primitives support a small, retained **name-cell
Source component**. Current startup supplies neither that canonical cell nor a
complete actual Application owner. A combined factory for
`GameNativeGameApplicationFrame` is therefore **not ready**: its Application+14
reference cannot be obtained from current projected startup storage by casting,
adding a detached pointer, or using the Game global in its place.

This is an evidence-only continuation after retained-frame Source commit
81ef1068d44682ea170cbd5afe2ec9420c2ba134. Its separate integration and build work
does not establish startup wiring. This packet changes only this document and
the paired JSON report, with zero Source or Original credit. It inspects exact
producer/caller slices, constructor shells and relevant current Source; it does
not repeat the Game caller-frame audit or sweep Application initialization.

## Native Application identity and initialization

At 008F8410 WinMain forms `ECX = ESP+20h` and calls 00737970. After pushing the
two Initialize arguments, 008F8425 uses `ESP+28h`, the **same address**. The loop,
shutdown and direct destructor calls at 008F8432/843B/8444 again use `ESP+20h`.
This is the stack Application slot identified as 1Ch in the earlier WinMain
layout audit; no heap allocation of an Application occurs in this caller slice.
The Game's separate 71A0h allocation is not this object. The exact stack-slot
identity is refreshed here; the earlier full stack-local extent analysis is not.

Three ECX-receiver, no-stack-argument constructors return the same receiver:

| Body | Established stores/calls |
| --- | --- |
| 00BEA810, 145 bytes | Install native table identity D68BC4. Obtain manager/lock, enter and increment its recursion word when nonnull. Publish receiver to E1AE90 at BEA866. Get the manager again, **reload current E1AE90** at BEA871 and register that current pointer through BD0C30. Leave the captured lock. |
| 00BEA970, 22 bytes | Call BEA810, install D68BC8, clear byte +4, return receiver. |
| 00737970, 42 bytes | Call BEA970, install CFEAB0, clear dwords +8/+C/+10/+14, clear bytes +19/+1A, set byte +18 to 1, return receiver. |

The native numeric table writes are not callable Source tables. The producer
chain includes real publication, manager registration and lock behavior; its
last field stores alone are not a complete constructor. No whole-object zero
is observed. Bytes +5..+7 and +1B have no established preimage in these stores.
The +4 write is a **byte**, not a subsystem-pointer initialization.

The Source `NativeApplicationPointerVectorStorage` establishes the 12-byte
subobject at Application+8 as `{data, signed count, signed capacity}`. It supplies
neither the containing owner nor its +14 Game pointer. Direct native destructor
7379A0 passes that +8 subobject to 735F30 with zero, frees its current data, and
calls BEA990. The caller then drains the manager. This audit does not expand
those child destructor bodies, supply Application tables, or claim complete
Application retirement/FH3 behavior.

## Native duplicated-name producer and lifetime

The selected slices retain this schedule:

| Site | Established behavior |
| --- | --- |
| 73D4C2 | Capture Init's second stack argument into EBP. |
| 73D933..73D93C | Move that retained input to ECX and call existing 438E40. |
| 73D941..73D94A | Reload the Application receiver for the parser, store the returned duplicate from EAX to E1AE78 at D945, then call 73CE20. No old-cell read/free precedes this store in the producer slice. |
| 73CE41 | Parser reads the **current E1AE78** value. Current Source parser instead accepts an explicit `std::string` argument. |
| 73E17E | The already audited Game caller later borrows the current cell after its Game allocation/zeroing. That fragment is not re-exported or re-audited here. |
| 73E45C..73E473 | Reload current E1AE78 into EAX; compare with EDI, skip when equal, otherwise free captured EAX through BF6989 and only after return store EDI to E1AE78. |
| 73E474 | Continue with Application+14 for Game entry. Duplicate retirement occurs inside Init, before the later main loop. |

The free/clear slice literally uses EDI. The prologue's `XOR EDI,EDI` at 73D433
and prior normal-path audit establish its zero-register convention. This packet
preserves those exact operands; it does not re-audit all intervening Init paths
or callee register contracts. The proposed Source composition is restricted to
that normal zero-register domain. In particular, it must free the **current
cell's captured value**, not a cached original allocation, and clear only after
that free returns. A null current value skips both free and store.

The current Ghidra direct-xref inventory for E1AE78 contains five sites: D945
write, CE41 read, E17E read, E45C read and E46E write. This is a direct-reference
inventory, not proof that indirect/escaped access or external mutation is absent.
A cell-owning API must qualify any replacement value's ownership and lifetime.

Both E1AE78 and E1AE90 are initially loader-zero: they lie in `.data` at offsets
12E78h/12E90h, beyond its 10000h raw extent but within its 297EDCh virtual extent.
Static Ghidra zeros match PE zero-fill. There are **no four on-disk bytes** at
either cell to compare, and this is not a running-game value observation. It
supports explicit zero initialization of the sole corresponding Source cells;
it does not justify zeroing the stack Application's unobserved bytes.

## What current Source actually owns

`GameStartupHost::application_construct` initializes `ApplicationFrameState`
and sets `constructed_`; its comment explicitly leaves subsystem fields
unreconstructed. That state is a two-boolean projection, not 1Ch storage.
`application_destruct` only clears the Source constructed flag. The host's
member inventory contains no actual Application owner or retained E1AE78 cell.
Current platform creation passes the `GameStartupHost` pointer as a projected
identity. Its C++ object layout does not establish a raw Application+14 field.

`ApplicationFrameService` merely borrows an identity and separate frame fields,
checks identity equality and calls the projected frame routine. It neither owns
nor constructs native Application storage. `game_hosts_text.cpp` also explicitly
states that this process builds no E1AE90 owner for the native Lua-owner lookup.
The renderer Application service and the 12-byte shader vector component are
separate providers, not evidence of the missing 1Ch owner.

At current phase 3, `run_initialize_phases` calls
`parse_command_line_0073ce20(mode)` directly. Its comment describes the native
duplicate, but no duplicate call, publication or retained-cell retirement is
implemented there. `BootstrapHost::command_line()` returns a `std::string` value;
it is another projected input boundary. The separately supplied retained Game
frame borrows genuine Application+14 and duplicate-cell references and creates
neither producer.

The existing `duplicate_native_string_00438e40` is the actual 57-byte/30-instruction
ECX/RET0 Source entry with the two call operands rebound to canonical allocation
and standard memcpy. Its header requires live readable nonnull NUL-terminated
input for nonnull duplication and matching `singleton_lifetime_free` after last
use. Its current Source allocator uses `std::malloc` through the canonical
nonnull-or-throw service; matching Source free uses `std::free`. Native BF6989
is a jump to the existing free entry BF65AC. Original private CRT/EH behavior,
OOM/new-handler failures and arbitrary faults are not newly admitted here.
The existing primitive report retains older reopened coverage flags alongside
the later `complete_helper_hold_resolution` and `Root_dup57p2` Source-1
publication. This audit uses that explicit later qualification and current
Source/byte identity; it does not replay the historical fixture or broaden it.

## Smallest legitimate next components

**Name side: feasible bounded Source work, not implemented here.** A stable
owner can contain the **sole canonical** `char* volatile` cell initialized to
the proven loader-zero value, expose that same cell by reference, and provide
explicit one-shot production and retirement. Production invokes the existing
duplicate once at the D93C/D945 point and publishes only its returned result.
No extra duplicate, old-value cleanup or mirrored parser/frame cell is needed.
Parser and retained Game-frame consumers must read this same current cell at
their own native points. Retirement captures the current value once and, when
nonnull, calls matching canonical free before clearing the cell. The original
input remains borrowed. Do not automatically retire from the owner destructor
or infer Original failure cleanup; retain pending state until explicit resolution.
Any admitted replacement in the mutable cell must already satisfy the same
allocation/ownership domain and account for the replaced allocation externally.
An alternative binder may borrow an already genuine retained cell, but none is
currently supplied by the inspected startup owner.

Installing that owner would require explicit startup producer/consumer/lifetime
wiring. An unused new cell beside the current `mode` path would establish no
actual producer. The existing duplicate primitive supports only its qualified
domain; a future binder needs its own Source failure policy without claiming
Original allocation-failure/FH3 equivalence.

**Application side: current-source binding remains blocked on a real owner.**
A later borrow view may expose the actual already-constructed owner's +14 cell
and retain its lifetime. Current Source has no such owner to lend. Completing
the Application producer requires its actual stack/storage lifetime, complete
constructor chain/current publication and registration, genuine callable tables,
all reached consumers and qualified retirement. A detached `void*` field, cast
of `GameStartupHost`, numeric table stamp, partial constructor or new full-Game
context defaults cannot satisfy those obligations. No new combined frame factory
is recommended until that owner exists and the already required Game contexts
are fully composed.

## Verification boundary

The report preserves 14 native spans: 488 code bytes equal to the installed PE,
plus eight static loader-zero bytes, with 151 fully decoded instructions. Five
whole function bodies and the free thunk are retained; remaining code spans are
explicit caller/dataflow slices. Seven valid instruction-context snapshots and
the direct-xref inventory are preserved. Source hashes/excerpts and earlier
report identities are pinned. One non-instruction context query was discarded;
an initial raw-byte comparison of a zero-fill cell was corrected using its PE
section extents. No Source, GPR, ledger, build, test or probe changed; no native
execution, Original EH, startup completion or gameplay validation is claimed.
