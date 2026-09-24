# Raw resource-registry node leaves

This packet reconstructs four complete normal bodies, 404 bytes, over the actual
resource factory tree. It provides node construction/allocation and iterator
comparison/decrement needed before raw insertion. It does not register factories,
populate procedural globals, activate B107F0 or admit a logical registry.

| Entry | Exclusive end | Bytes | Native ABI | Coverage |
|---|---|---:|---|---|
| B19530 | B19559 | 41 | ECX left8B iterator; stack right; RET4/EAX | complete, exact high24/AL result |
| B1AB40 | B1ABC9 | 137 | ECX mutable8B iterator; RET/returning validation tail | complete |
| B1ACA0 | B1AD10 | 112 | ECX actual1Ch; five stacked DWORDs; RET14/EAX this | complete normal body |
| B1AD10 | B1AD82 | 114 | five stacked DWORDs; ECX unused; RET14/EAX captured allocation | complete normal; explicit source exception projection |

The native1Ch node is links0/4/8, raw name length/data C/10, borrowed factory14,
color/nil bytes18/19 and untouched1A/1B. The tree subobject is registry+4:
opaque0/head4/count8, and sentinel links0/4/8 mean minimum/root/maximum. These
are the same actual fields consumed by existing resource-registry lookup,
rotations, iterator increment and node/subtree disposal. Factory14 is borrowed;
the leaves add no retain/release, count, metadata object or ownership policy.

`NativeResourceRegistryNodeArguments` and the allocation frame bind immutable
references to caller-owned live volatile DWORD cells. They do not cast an
aggregate onto reused scratch. View metadata and diagnostics are disjoint from
payload/argument/local backing. Incoming, nested constructor and cleanup cells
represent distinct native frame sites. Exact raw DWORD access uses MSVC Win32
MOV helpers, so existing pointer-cell iterators and live DWORD backing can both
be observed without constructing a different C++ object over them.

B19530 captures left owner for validation. A missing/mismatched owner calls the
required genuine returning BF6713 binding; no default success service exists.
After it returns, current left/right node words determine equality. The exact
result preserves the left node's high24 bits and replaces ONLY AL. It is not a
normalized DWORD bool. B1AB40 reloads the node after initial validation, publishes
intermediate ascent parents and rereads the current iterator before the final
nil check. Both invalid nil-node tails return immediately after BF6713 without
retry or a later selected-node store. Callable validation is a valid-domain
precondition; a null callback does not become a source exception.

B1ACA0 captures left/right/pair/parent before its first store. It compares the
destination C-header with the captured pair pointer before clearing C/10. It
then writes links, zeros the header, calls genuine raw41DD40 preserve1 on the
non-self path, rereads current source length, and captures current destination
length/source data/destination data in that order for copying. Native BF7680's
overlap-compatible byte effect uses host memmove; the established zero-count
host boundary omits a zero-byte call. Current pair+8 is captured before the late
incoming color BYTE load, then value14/color18/nil19 are written. Padding stays
unchanged. There is no NativeString object or automatic name cleanup.

B1AD10 allocates exactly1Ch through the existing actual CRT boundary before
reading the five incoming words. It stores allocation cleanup, arms state0,
stores placement cleanup and advances to state1. Non-null construction reads
CURRENT incoming color/pair/right/parent/left and performs the original MOV/PUSH
interleaving into the caller's separate constructor argument cells. It returns
captured allocation even if exposed cleanup cells change. Null allocation skips
construction. Fresh persistent `NativeResourceRegistryNodeAcquired` is required;
reusing an already-started acquisition is rejected before native work.

DF49F4 has three unwind states: 0->-1/no handler, 1->0/CBC6D0, 2->-1/no handler.
The try0..1/catchHigh2 record points to catch-all B1AD82. CBC6D0[17] reads CURRENT
EBP-14 then EBP-18 and calls verified RET-only401130. This placement-delete
boundary has no payload or string effect. B1AD82[21] then frees CURRENT EBP-14
through BF65AC and calls BF6885(0,0) to rethrow. The source preserves those two
volatile reads and projects free/rethrow through C++ transport. No partial name
destructor/return is added. Diagnostics preserve captured allocation and current
freed address separately; neither may be read after free. Any residual name
allocation needs explicit caller disposition. Allocation failure before arming
does not run this cleanup. Hardware faults/native FH3 are not reproduced.

Compiler boundaries total48B: B1AD82[21], CBC6D0[17], CBC6E1[10]. Fresh Ghidra
still stores only9B of B1AD82 through B1AD8A, omitting the returning-free tail
ADD/PUSH/PUSH/rethrow. CBC6E1 is undefined. Full live bytes equal PE bytes; exact
inclusive ends and last instruction lengths are in the report. Worker performs
no repairs or annotations. Until primary repairs/defines, report call rows in
those missing regions use `no_ghidra_function` with an explicit actual mnemonic;
the dispatcher row must become `tail_jump` after definition.

Strict `scripts/build.ps1` passed MSVC Win32 and both configured worker CTests.
The first build found a reserved inline-assembler operand name `offset`; changing
that parameter to `byte_offset` resolved it. Both logs are retained. No tracked
tests were added. One ignored standalone comparison compiled and passed on its
first invocation with `/MD /EHsc /std:c++20 /O2 /Gy /W4 /WX /fp:strict`, explicit
`/link /OPT:REF /MANIFEST:EMBED`, and compile-time rejection of `NDEBUG`.

The focused probe copies all404 normal bytes. CALL/JMP displacement bindings are
changed at the eight native transfers: six CALLs and two validation JMPs. The
constructor/allocator wrapper remain composed with each other as copied native
bodies. External allocation/resize/copy/validation calls use declared shared
providers: genuine CRT allocation, genuine raw name pool, host memmove and a
genuine returning CRT invalid-parameter service with an installed test handler.
Allocator/resize adapters mutate exposed cells on both sides to check native
read points. The source body is included unchanged under two provider-name
redirects for those instrumented boundaries; the separately built production
library retains its direct genuine providers. This is normal-body composition,
not an independent comparison of provider bodies or native private stack ABI.

The sequence compares allocated-node bytes after verifying actual name length,
content, borrowed value, links, color, nil and padding; only the name-data pointer
at+10 is normalized because real pool allocations differ. Existing actual
B1A260 disposes both nodes/names. A direct self-header pair checks skipped resize
and preserved value/padding. An iterator sequence checks exact EAX high24/AL,
sentinel maximum, predecessor descent/ascent and returning CRT repair/tail order.
Two counters confirm actual allocation/resize entry. A source-only exception
after completed genuine resize verifies placement cleanup/free/rethrow state2
and explicit residual-name disposal. Copied native EH is not executed. Null
allocation, initial allocator failure, empty-name/zero-copy, every arbitrary
callback alias and every validation path are not individually fixture-tested.

Evidence: `local/output/cc10_resource_registry_node_leaves/` contains fresh
live/PE byte records, full listings, build logs, exact probe command/source,
binary/object/provider identity receipt and passing output. The preceding frozen
readiness archive is pinned by SHA256
`04fe21149d6615bfd7eaba7b07c065cf5b1a1e09172fa53e80c390060c534868`.
That readiness preserves the misleading existing B1ADA0 throw-only label and
verified normal insertion frontier; changing it and implementing insertion are
outside this packet. No original game/application process was launched, and no
source interface is a native ABI/FH3 or later-startup/gameplay claim.
