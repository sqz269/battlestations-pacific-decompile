# Borrowed locale recycling helpers: primary acceptance

The slot helper borrows the actually initialized locale pool and an actual
allocated slot. It enters the pool OS section before current depth, slot,
table and block reads; preserves wrapping DWORD/WORD updates, signed division
by28 and the unsigned minimum update; then decrements current depth and leaves.
It introduces no bounds/null guard, rollback, unlock RAII or physical free.

The scalar wrapper invokes existing payload cleanup first with the actual raw
string context. Only normal payload return permits the later flags-byte read
and optional slot return. Both normalized interfaces remain dormant; stable
Win32 import bindings are a factoring precondition, not runtime binding proof.

The new document explicitly corrects two earlier proposal labels: the real
allocator header is allocator_list.hpp (unused by these helpers), and the
existing string cells are01090AA8/01090AA4/01090AA0. Historical evidence is
preserved; no fabricated cells or new allocation domain were introduced.

Root reviewed the copied guarded worker proof and actual Git blobs, then
verified its own merged Source epoch: 3356 selected build inputs,
4210 src/include paths and 4217
actual Git paths. The fresh normal Win32 build passed all three existing checks;
no new test or probe was added. The prior accepted Root artifact pins match
the baseline exactly. All 174331 existing Core/App code sections
remain byte/relocation exact; 10 sections are added only in
the owned translation unit. 0 old whole objects
change; 1 objects are added. All80 App whole objects
remain exact. Root selected method payloads match the separate worker build;
relocations match under the retained positive function-symbol bijection for
path-dependent anonymous namespace names, where applicable. Positive archive
definitions are unique. The new methods
have no external Core/App code callers and no game-map symbols. Linked game
text remains exact, but whole artifact hashes need not be identical.

These are Source, build and static artifact observations. Canonical owner and
factory lifetime, Original ABI/unwind behavior, runtime publication/quiescence,
application activation, startup and gameplay are still unproved.

Evidence: reports/cc12_locale_helpers_primary.json and local/cc12_locale_helpers_primary/Root_compiled_review.json.
