# Logical vertex owner: remaining terminal dependency

The complete logical vertex destruction packet is **not ready for a general
actual-owner interface**. The physical-buffer, CPU-declaration, raw
unregistration and optional-synchronization dependencies now have concrete
implementations, but no nonnull object profile has been established for the
base owner's `+4C` field. Its destructor explicitly supports a nonnull
reference and calls that object's current virtual zero-count terminal.
Constructor-null is not accepted as a lifetime-wide invariant.

This is a read-only discovery packet based on main `9341b8a`. It changes only
this document and `reports/native_logical_vertex_owner_next.json`; it adds
no C++, tests, Ghidra edits, exports, ledgers, or shared build registration.
The report pins 47 native spans against fresh project-guarded Ghidra reads
and the installed PE, plus current dependency source contracts.

## Four exact roots

| Root and exclusive end | Native ABI | Status |
| --- | --- | --- |
| `B4B5D0..B4B6E8` | ECX actual logical stream, RET, no stable EAX contract | Full destruction order and two EH states established; depends on full base terminal |
| `B4BF10..B4BF30` | ECX owner, stack flags low byte, RET4, EAX original address | Full deleting wrapper established; depends on destruction and pool return |
| `B49570..B495D9` | ECX actual pool, stack actual slot, RET4 | Independently ready; no virtual dispatch or owner cleanup |
| `B62010..B62097` | ECX actual base storage, RET | Full normal/EH body established; nonnull `+4C` terminal remains unresolved |

The logical profile prefix at `D61D6C` starts with `BD30E0` and deleting
slot `B4BF10`. The object occupies `74h` bytes and its pool slot occupies
`78h`; the extra DWORD at `+74` stores the slab index. Do not initialize or
replace the whole table merely to supply a terminal dispatcher.

## What is and is not proved about `+4C`

The complete base constructor `B61E20..B61F84` zeros EBX and stores it into
actual `this+4C` at `B61E92`. Both concrete constructors that install the
logical profile call this base constructor: `B4BC00..B4BEF6` and
`B4A9B0..B4AA94`. Fresh profile cross-references identify these two stores
and the destructor's store at `B4B5EF`.

The complete reviewed logical-profile methods cover lock, unlock, flags,
count, declaration, buffer, offset, invalidation, and the three returning
leaves at `B48D30`, `B4AAA0`, and `B4AAB0`. Their direct field accesses do
not write `+4C`. The physical-pointer setter `B4AAC0..B4AAFB` writes `+58`,
and the decode setter/writer `B61D90..B61D9A` and `B61F90..B62003` write
`+50`. These are distinct ownership fields and do not explain `+4C`.

Among the complete reviewed logical-family bodies, the only direct `+4C`
accesses are the constructor's zero, the destructor's read at `B62034`,
and its post-release clear at `B62059`. Four bounded byte-pattern scans
over `.text` range `B40000..B63000` retain 9 direct-MOV, 4 immediate-MOV,
3 LEA, and 20 load candidates. Candidate bytes include other object types,
longer displacements and possible non-instruction matches. A broader raw
MOV search was used only for candidate prioritization; neither search is
complete alias, bulk-copy, external-write, or whole-program lifetime proof.

No nonnull writer, producing allocation, concrete table, deleting slot,
or allocation-return domain has therefore been admitted for `+4C`.
The field's purpose remains unnamed. It must not be relabeled as a physical
buffer, declaration, layout, COM interface, or known resource merely because
those types already have usable host lifetimes.

The required next evidence is either a concrete nonnull assignment chain
with its current terminal profile(s), or a whole-lifetime proof for every
admitted caller/alias establishing that nonnull cannot occur. The present
bounded negative search is insufficient for the latter. No generic behavior
callback or null-only substitute is proposed.

## Destruction order and the separate renderer lock

`B4B5D0` first installs `D61D6C`, saves the actual owner, and arms EH state 0.
When current mode `108D6DC` is nonzero it captures current global renderer
`F8D394`, enters the existing optional guard, and saves AL. It reads, masks
and compares current `this+60 & F000`, then arms state 1 before branching.

For dynamic flags `1000`, it captures **current renderer +19F4**, calls the
real `EnterCriticalSection`, and increments the captured section's depth
at `+18`. It reads current physical owner `this+58` and invokes full
`B4B3F0` with the actual logical pointer. After that call, it reloads global
renderer `F8D394`, decrements that renderer's `+1A0C` depth, and leaves that
renderer's `+19F4` section. Entry and exit need not use the same renderer
if an intervening call changes the global; reconstruction must not repair
this order. This lock is separate from optional synchronization's pointer
at renderer `+04` and from any pool's section at pool `+0C`.

The non-dynamic branch still reads current physical `+58`, then its count
DWORD `+04` at `B4B66B`, even though EDX is not subsequently used. This
actual read is part of the native contract.

Next, the destructor reloads the global renderer and invokes complete
`B268E0` to remove the raw logical pointer from renderer `+1AAC`. It captures
current declaration `this+68`, calls real `InterlockedDecrement` on `+04`,
and calls the captured object's **current** table slot 0 on zero. Only after
that terminal returns does it capture current physical `this+58` and apply
the same decrement/current-terminal sequence. Neither field is nulled.

State 0 is restored before conditional normal optional-guard leave.
State -1 is stored before the normal call to the full base destructor.
The physical and declaration releases cannot be pre-captured together:
the first terminal may change the later field or global renderer.

## Full base destructor and EH evidence

`B62010` installs base profile `D62B68`, captures current `this+4C`, then
arms its own state 0. If the captured pointer is nonnull it decrements its
actual count at `+04`. On zero it loads that captured object's current
table and calls slot 0. Only after this returns does `B62059` clear the
owner's `+4C` field. A throwing terminal leaves the field uncleared.

It next captures current `this+50`. If nonnull it calls `BF65AC`; after that
call returns, the hidden continuation at `B62070` clears the owner's `+50`.
The saved Ghidra listing omits this continuation because of the old free
flow annotation. The full 135-byte PE/Ghidra preimage proves the returning
tail. State -1 then precedes `BD30F0`, which writes only profile `CEB130`.

Logical FuncInfo `DF83DC` is 36 bytes, magic `19930522`, max state 2, and
unwind map `DF83CC`: state 1 goes to 0 through `CBF928` (saved guard at
frame `-14` to complete `B21110`), then state 0 goes to -1 through `CBF920`
(saved owner at frame `-18` to `B62010`). Handler `CBF930` is 10 bytes.
There is **no EH state or unwind action for the separate +19F4 lock**.

Base FuncInfo `DFA2EC` is 36 bytes, magic `19930522`, max state 1, and map
`DFA2E4` sends state 0 to -1 through `CC14A0`. That funclet loads saved
owner at frame `-10`, jumps to five-byte thunk `B48D70`, and reaches
`BD30F0`; handler `CC14A8` is 10 bytes. Neither map contains a catch or a
retry of a failed virtual terminal. Both have null ESTypeList and EHFlags 1.
Second-exception behavior should be retained from FH3 and checked in the
eventual native caller comparison; this discovery does not execute it.

## Available contracts and future ownership

| Dependency | Concrete available route and supplied storage |
| --- | --- |
| Current declaration `+68` | Profile `D61D1C`: slot0 `BD30E0` reads current slot4 `B48CA0`; full CPU owner uses actual pool `108FD38` and borrowed type-size table `D61CC0`. Files `native_vertex_declaration_owner.hpp/.cpp`. |
| Current physical vertex `+58` | Private `D61E34` / pooled `D61E7C`: slot0 `BD30E0`, deleting `B4BB40` / `B4C230`, full destructor `B4BAB0`; pooled return `B49500` uses actual pool `108FDE0`. Files `native_physical_buffer_owner.hpp/.cpp`; source and integrated audit read from primary checkout `d8326d2`. |
| Raw unregister | Full `B268E0/B25300` and `B4B3F0/B4B2E0` in `native_render_buffer_unregistration.hpp/.cpp`; actual array and global references, no pointee lifetime. |
| Optional guard | Full `B33AD0/B33B00/B21110` in `native_renderer_synchronization_actual.hpp/.cpp`; borrowed globals at `108D6DC`. |
| Separate `+19F4` lock | Actual inline 1Ch tracked section, real Win32 Enter/Leave, captured-entry/current-exit ordering above; no additional lifetime provider. |
| Base scalar release | Existing `singleton_lifetime_free` for the established `BF65AC` allocation domain; preserve the post-return `+50` clear. |
| Additional `+4C` owner | Unresolved producer/profile/terminal/allocation domain; blocks general full destruction. |

The independently ready leaf `B49570` returns a slot to actual pool
`108FE18`: enter captured `pool+0C`, increment depth `+24`, read current
slot slab index `+74`, current slab table `pool+28`, and that slab pointer.
Its signed wrapped `(slot-slab)/78h` quotient is truncated to WORD and
stored at `slab+F00 + current WORD[slab+F40]*2`. The WORD count is reloaded
and incremented after that store, retaining aliasing and 16-bit wrap.
Unsigned slab index may lower current `pool+34`; the same captured section
depth is decremented and left. There are no virtual calls or EH actions.

If split out, a leaf packet should own only `B49570` and
`include/bsp/native_logical_vertex_slot_return.hpp`,
`src/native_logical_vertex_slot_return.cpp`,
`docs/NATIVE_LOGICAL_VERTEX_SLOT_RETURN.md`, and
`reports/native_logical_vertex_slot_return_audit.json`. Remove that address
from the eventual owner packet's independent work list after integration.

A later full owner packet should own roots `B4B5D0`, `B4BF10`, `B49570`,
`B62010`, and its four new files `include/bsp/native_logical_vertex_owner.hpp`,
`src/native_logical_vertex_owner.cpp`, `docs/NATIVE_LOGICAL_VERTEX_OWNER.md`,
and `reports/native_logical_vertex_owner_audit.json`. Its inline EH support
addresses are enumerated above. Dependency files remain with their existing
owners; hardware-layout tree work and logical-index roots do not overlap.
Do not assign that full packet as ready until the `+4C` dependency is closed.

The discovery validates native bytes and currently supplied dependency
source, not new C++ execution, a complete owner lifetime, binary ABI
replacement, or game behavior. Existing dependency fixture results are
reported with their source audits; they were not rerun for this document.
