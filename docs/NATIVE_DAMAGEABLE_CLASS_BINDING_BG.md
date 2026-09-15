# Damageable-class model-point binding

Addresses: 00879AD0, 00879A70, 00876DA0, 008772B0, 0074D190, 0085DC80, 00438E10, 0095F500, 00877FE0, 0087CA80, 00C9667E

The complete 00879AD0 binder now consumes the actual class+50 game resource. It
writes crew-marker matrices into the class frame tree, binds explosion positions
to the class effect rows, appends indexed fake-explosion points, and copies the
current effect positions to their secondary positions. It uses the complete
named-group lookup from BF and existing raw SBO strings, allocation, checked-tree
successor, and camera-vector kernels. No copied class or resource registry is
introduced.

| Address | Native span | Source behavior |
| --- | ---: | --- |
| 00879AD0 | 2354 bytes | Complete three-stage binder and final effect-position copy. |
| 00879A70 | 46 bytes | Construct an output SBO string from point-item category+28h. |
| 00876DA0 | 123 bytes | Find the first 30h effect row whose category+4/index+8 match. |
| 008772B0 | 99 bytes | Adapt the existing checked-tree successor to nil byte+55h. |
| 0074D190 | 116 bytes | Grow a counted Vec3 array, copy with x87, free old storage, publish new begin/capacity. |
| 0085DC80 | 532 bytes | Complete native in-place matrix schedule, added beside the older pure arithmetic interface. |

The five newly completed binder/helper bodies total 2738 bytes; the matrix
interface adds the full 532-byte native schedule, for 3270 bytes covered here.
The binder span includes six unreachable alignment bytes after 87A348. They
remain undefined in Ghidra. The existing 438E10 null-safe comparison is exposed
from its canonical implementation; the keyboard iterator wrappers reuse their
existing successor algorithm with their existing default CRT behavior.

## Behavior and producers

The crew stage walks the actual checked tree at class+60h, looking up `emberke`
with each node's index at +Ch. It uses the first three points to construct a
matrix, preserves every inline x87 subtraction/cross-product spill, invokes the
native orthonormalizer, then publishes all 16 words at node+10h. A missing crew
group has no invented fallback. Returning validation preserves captured iterator
and point-index values while reloading the current storage at the native sites.

Node producer 877FE0 allocates 58h, writes its three links, copies 18 DWORDs at
+Ch, and writes color+54h/nil+55h. Lua reader 87CA80 zeros a 30h temporary at
87CE00, selects class+18h at 87CE3A, appends it at 87CE48, then writes the row's
index+8 at 87CF60 and category+4 at 87CF8E. The Points-item constructor/reader
71ABB0/71B3E0 were established in BF. These producers are evidence references,
not additional implementations in this batch.

The explosion stage first copies and destroys every item's category even when
the name does not match. `explosion` matching is case-sensitive and counted;
category matching uses the current null-terminated 15-entry table at E08138
and the original null-safe CRT comparison. Unknown categories map to FFFFFFFF.
Only the first matching effect row is updated. The fake-explosion stage starts
at index 1 and stops at the first absent index. Its array preserves prior
entries, grows with the native signed/wrapping capacity arithmetic, and publishes
count after the point. Finally, each effect row's +Ch position is copied to +18h
through ordered x87 loads/stores, including NaN quieting.

## Native arithmetic and cleanup

The native 85DC80 interface retains the complete x87/SSE sequence over the
actual 16-word matrix. It borrows the real CRT binding and the constant cells
at D0D0A0 and D7A208. Length, normalization and cross product use the existing
canonical camera kernels. The original `SUBSS -0.0, component` operations stay
arithmetic, preserving behavior that the older sign-negation model omitted.
The older pure arithmetic API remains available to its existing consumers;
the new class binder calls the native interface.

Four returning-free gaps in 879AD0, two in its vehicle caller 95F500, and one in
74D190 were repaired under leases and the Ghidra write lock. Bytes and callee
no-return flags are unchanged. Exports were refreshed. The vehicle caller's
2518-byte body remains a repaired reference; it is not reconstructed here.

The newly defined C9667E handler selects FuncInfo DC8900 and the three unwind
rows at DC88E8. All states unwind to -1: states 0 and 1 destroy the category/name
temporary at EBP-A4h; state 2 destroys EBP-28h. Existing Unwind names are retained.
The source arms each completed string only after construction returns, disarms
before normal cleanup, and uses the canonical destructor during C++ unwinding.

## Verification and remaining work

MSVC Win32 `/O2 /fp:strict /W4 /WX /EHsc` and both existing CTests pass. One ignored
fixture executes the complete original binder and helpers with explicit bridges
to the canonical source dependencies. Three original/source binder pairs cover
empty input, all populated stages with two tree nodes, and returning validation
that repairs point storage. They exercise heap category copies, known/case-folded/
unknown categories, duplicate first-match effect rows, two fake-point growths,
an index gap, and signaling-NaN/signed-zero/subnormal transfers. Three separate
matrix pairs exercise normal, parallel-reference and zero-forward paths. Matrix
and class images, FP state and finite sqrt-handler records agree. Both sides of
each comparison are retained separately.

The CRT fixture handler records the finite exception records and retains the
provided result/control word. Both callers use the same canonical vector/CRT
kernels and that explicit fixture policy; those dependencies are not independent
original-code oracles. The original 4B3FC0 comparator is executed separately.
Native FH3, throwing cleanup, unmasked hardware exceptions, malformed/private-stack
aliases, actual Lua/resource parsing and game execution are unvalidated. This is
a new C++ interface, not a binary replacement. Full 95F500/82FE30 binding, vehicle
entry activation and executable admission remain open. No workers were dispatched.
