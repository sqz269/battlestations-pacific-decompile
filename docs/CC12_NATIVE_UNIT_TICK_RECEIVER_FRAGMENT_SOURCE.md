# Unit tick receiver registration fragment

`register_native_unit_tick_receiver_0087b699` reconstructs the qualified business
fragment `0087B699..0087B728` of `0087B670`. The full original parent is 212 bytes /
46 instructions; this selected span is 143 bytes / 24 instructions. Original PE,
fresh live Ghidra bytes, fresh saved listing and independent instruction starts
agree throughout. The descriptive names remain hypotheses.

The ordinary Source C++ interface borrows an actual receiver whose parent
construction has already returned, with aligned live backing through `+38B`.
Its embedded node is that same receiver's `+310`, its payload is that same
receiver, and its raw group word is zero. It invokes the existing admitted
`construct_native_tick_registration_00875890` with the genuine registry/manager
publication references, fixed pending tail and current timer reference.
That provider's actual section, neighboring links and lifetime requirements
continue to apply. No independent cells, replacement callback or projection
receiver is supplied.

After the provider returns, exactly fifteen volatile DWORD stores follow:

| Offset | Value |
| --- | --- |
| 00, 10, 24, 170, 1E4, 310 | D0DF70, D0DF54, D0DF4C, D0DF48, D0DF40, D0DF28 |
| 348, 34C, 350 | Zero |
| 35C | FFFFFFFF |
| 364 | Captured current D0E12C word |
| 374 | 3F |
| 380, 384, 388 | Zero |

The six profile words are opaque data, with no callable Source-table promise.
The borrowed D0E12C word is read after the three first zero stores and before
the 35C store. Native MOVSS performs a bit transfer; Source uses a DWORD copy
without float arithmetic. The original/live selected cell currently contains
`CD CC C7 42`; this observation is not a copied default or substitute cell.
The required capture order is preserved even for overlapping borrowed fields.
No whole receiver initialization or holes sanitation occurs.

Original parent ABI is ECX receiver plus one flag stack argument and RET4,
with an FS/FH3 frame and saved EBX/ESI/EDI. This fragment supplies a new ordinary
void C++ interface. It excludes the 0077EED0 parent call, the later C4 class-id
store, native stack/frame restoration and native exception cleanup. A throwing
tick provider escapes with its partial state and without these later stores;
there is no new cleanup, retry or rollback. Hardware faults, Native SEH/FH3,
XMM0/register/flag identity and drop-in ABI compatibility remain unproved.

The emitted complete Source root is 181 bytes / 34 instructions with one
indexed REL32 to the real tick-registration provider. It has one positive
Core definition and no production application-map consumer. The normal build
passes the three existing checks; no new test or runtime invocation of this
fragment is claimed. Full unit receiver, parent cleanup and production owner
remain open. See [primary review](CC12_UNIT_TICK_FRAGMENT_PRIMARY_REVIEW.md).
