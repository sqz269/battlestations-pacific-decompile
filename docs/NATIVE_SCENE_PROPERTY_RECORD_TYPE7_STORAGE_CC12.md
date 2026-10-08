# Native property-record Type7 raw storage, CC12

Source admission is **1 for the bounded current-provider raw storage domain**.
The whole initializer `[008EF270,008EF2AF)` is 63 bytes/19 instructions, all
literal bytes, with no CALL or relocation. The existing CPP/HPP are unchanged. The provisional raw-storage name and
evidence are saved in the configured Ghidra project; the affected export is refreshed.

ECX supplies a fresh writable unowned 56-byte root; EDX is unused padding.
The actual stable readable 12-byte input pointer at `T+4` supplies three opaque
DWORDs copied inline to `+0C/+10/+14`; the pointer is not retained. `RET 4`
returns the actual root in EAX, zero ECX and the middle word in EDX. It writes
45 bytes and preserves 11. Nonvolatiles, ES and DF are preserved. Defined XOR
flags satisfy `flags & 8C5 = 44`; undefined AF is unasserted. Byte `+2C=1`
establishes no ownership. Quiet-NaN/negative-zero encodings remain raw bits.

## Complete code and sole-process evidence

Root generated three fresh TUs in `local/t7p2`, with 184 actual consumed headers
and seven libraries. No BSP archive, old object or accepted process/stage was
replayed. The exact all-files seal includes every pycache and retained failure:
**491 files**, SHA-256 `e2be95c994eda25c018e7fff0ab27f2269cf4cec973eafb283bdf25b2d6c786e`.
Its Source0 summary is immutable history before this external Source1 decision.
All 24674 prior immutable pins and 28 exact prior
families remained unchanged.

Independent raw readers derived 35 logical mapped symbols and 33 physical TU
bodies, 243 logical relocation checks over 237 actual operand sites, six alias
rechecks and 18 complete external helpers: **51 whole gated code spans**.
The actual Main contains 3,359 bytes/877 instructions; Root manually reviewed
its complete code, 18 phase-dominance pairs and 51 guard-failure edges.
Both ordinary wrappers and the 29-byte/nine-instruction EH handler are retained
but dynamically unexecuted. Ordinary Original has 66 bytes including five
CFG-proven unreachable trailing INT3 bytes, all retained in its full gate.
Stack43, cookie14, cold delete16 -> delete5 -> free6, std import thunks and
every normal referenced helper are covered. The GS-failure frontier stays
named, unexpanded and unadmitted; cold coverage is not execution proof.

The sole recorded process calls Source with DF1 and unchanged Original RX
with DF0. Two actual canonical 56-byte roots are allocated and freed; there
are no child allocations. Both full roots, both 44-byte guarded inputs and
both 116-byte captures decode independently. The actual pointer stack DWORD
is captured before PUSHFD can overwrite its dead slot. Full EAX/ECX/EDX,
nonvolatiles, ES, DF, stack guards and all live-buffer bookends agree.
The gated probe saves all six blobs before freeing roots and releasing RX.

The three malloc/free/_callnewh bindings use frozen UCRT bytes and original
generation NT-path/FileID/hash associations. Before target calls, the probe
checks MEM_IMAGE/AllocationBase, mapped-versus-held-handle NT path, FileID,
full file SHA, live I386 PE tuple, named export/GetProcAddress/IAT and adjusted
live prefixes. After frees, it repeats IAT, held-handle FileID/size, mapped-file
SHA and live prefix checks; MEM_IMAGE, mapped NT path and GetProcAddress are
not repeated. The decoder opens only frozen DLL copies and creates no new
live identity attestation. Serialized prefixes are expected adjusted bytes
the probe compared with memory, not a separate live-memory dump.

## Current build and lifetime boundary

Main Win32 build at `a4ba85f1f6cc5c140034dd6d9eaaead6dcf80d76` passed all three existing checks with
4,041 Source/header/CMake input hashes unchanged. No new tests were added.
The [lifetime audit](CC12_TYPE7_RECURSIVE_LIFETIME_DOMAIN_AUDIT.md) confirms
tag7 takes the release default: it clears `+04/+0C/+28/+08`, leaves `+10/+14`
and `+2C` untouched and frees no stored child. Scalar flags concern only the
actual root's disposition in the existing initialized context contract.

Bag publication, connected raw clone, native vector/declaration/float
semantics, class/vtable ownership, arbitrary root lifecycle, private Original
CRT/EH, drop-in ABI, game startup and gameplay remain unadmitted.
