# Native input scale maps

Addresses: 0055B400 0055B490 00444BE0 006A5AA0 0055B300 0055B060 00554CD0 00554EC0 00558570 005555B0 00554F50 0055A290 00443D60 00444910 006A4DC0

The new source API in `native_input_scale_maps.hpp` uses actual Win32, twelve-byte map headers and native-size nodes. Allocation passes native/host sizes to `singleton_lifetime_allocate`; node/header release calls `singleton_lifetime_free`. `NativeStringStorage` owns the pooled string copies. Returned inner trees and float cells are borrowed. These are new C++ interfaces, not binary replacement calling conventions.

| Address | Original ABI | Coverage | Evidence and behavior |
|---|---|---|---|
| 0055B400 | ECX destination, source pointer stack, EAX destination, RET4 | complete normal populated copy | 0055B300 calls 0055B060, which recursively copies the 20h outer nodes. Each node constructor 0055A290 calls 00558570; the latter calls 005555B0 to recursively copy 18h inner nodes. Preserve signed keys, inner value bits, both colors, shape, count, and recompute copied head extrema. |
| 0055B490 | ECX map, RET | complete normal populated destruction | Saved body calls 0055B230 full range then frees head. The source releases every inner node and its head before the outer node and finally the outer head. |
| 00444BE0 | ECX raw string-CI map, native string pointer stack, EAX float*, RET4 | complete normal valid-tree lookup/insert | 00443D60 is lower bound; 00444910/004442A0 allocate and rebalance a 1Ch node. Existing case-insensitive match preserves the float and node identity. Missing string deep-copies into temporary and node; temporary is then released. New value bits are zero. |
| 006A5AA0 | ECX outer map, signed integer pointer stack, EAX inner tree*, RET4 | complete normal valid-tree lookup/insert | Signed lower-bound search returns node+10h when present. Missing path initializes 18h inner head with nil+15h, then 006A4DC0/006A3FB0 insert a 20h outer node and maintain count, extrema, colors. |

The outer node has key at +0Ch and nested tree at +10h, with color/nil at +1Ch/+1Dh. The inner node has signed key +0Ch, scalar bits +10h, color/nil +14h/+15h. The string map node stores its native 8-byte string at +0Ch and float bits at +14h, color/nil +18h/+19h. Device+78h is a different integer-only layout (nil+11h).

The original copy constructors do not write the opaque dword at map+0. The source leaves destination+0 and freshly allocated outer-node nested +10h untouched. Allocator preimages therefore remain an explicit boundary. The outer destructor, like 0055B490, leaves the freed head pointer in the dead map header. The 006A5AA0 missing path creates a default inner head, copies it into a pair head, then copies the pair into an inserted node head; it releases both temporaries afterward. Valid source trees, non-aliased destination, native allocator behavior, CRT invalid-parameter/SEH identity, private stack preimages, malformed trees, and gameplay behavior are outside demonstrated parity. No game validation is claimed.

The string index is shared by many subsystems. This packet reconstructs its native raw-header semantics for the owned keyboard use; other call sites are excluded from this packet's host-call attribution. Primary integration owns wiring and native fixture validation.
