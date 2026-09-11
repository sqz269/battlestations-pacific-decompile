# Native hardware-layout tree keys and lookup

Five complete functions now operate on the actual borrowed Win32 key, pair,
tree-header and node storage. They supply the key/lookup dependency of the
hardware-layout factory identified in
`NATIVE_HARDWARE_LAYOUT_CONSTRUCTION_NEXT.md`. Descriptive names are hypotheses.

| Address | Original ABI | Reconstructed behavior |
| --- | --- | --- |
| `B20BF0` | ECX left key; stack right; RET4; AL bool | Capture right count, then left count. Signed counts descend; equal positive counts compare unsigned pointer DWORDs ascending. Equal nonpositive counts compare equal without touching the pointer words. |
| `B23020` | ECX tree; stack key; RET4; EAX node | Lower-bound from captured head/root, using node key `+0Ch`, links `+0/+8`, and sentinel byte `+25h`. |
| `B28220` | ECX tree; stack output/key; RET8; EAX output | Lower-bound first, then null-owner validation and current-head capture. Reverse comparison rejects nonmatching candidates. Store output owner before node. |
| `B282B0` | ECX pair; stack key/value-address; RET8; EAX pair | Forward key-word copy with current source count reloaded after each store; copy current count, then read the value address and publish value. |
| `B25EF0` | ECX destination; stack source; RET4; EAX destination | Same current-count forward copy, then source value `+14h`, read after destination count publication. |

The key is 14h bytes: four raw stream-owner identities and signed count `+10h`.
The pair is 18h bytes with borrowed owner value `+14h`. The actual 28h node
places that pair at `+0Ch`; no owner reference changes or allocation occur here.
Unused key words and node padding remain untouched. The native loops add no
four-element clamp. Every accessed word needs valid backing storage; the new
interfaces do not promise safe behavior for arbitrary corrupt counts.

Assembly inspection resolved hidden ECX inputs, AL/EAX returns and exact store
order. In particular, overlapping pair storage can change the source count or
the value before its later load. A captured array or bulk memory copy would
lose that behavior. Volatile scalar DWORD accesses retain the observed loads
and stores. The comparator captures its counts once, unlike the copy loops.

The null-owner check in `B28220` follows `B23020`, which already dereferences the
tree. The callback site is preserved, but an ordinary null Win32 owner faults
before reaching it. Its returning/throwing handler path was not exercised and
is outside this fixture's valid-storage domain; no synthetic lower-bound
replacement was introduced to make the path reachable.

One private differential fixture executes all five complete original bodies
(359 bytes verified against live Ghidra and the installed PE), with no code
patches, substituted dependencies, absolute relocations, or entry/exit slices.
Relative calls connect the original comparator, lower-bound and find bodies.
Five loaded postimages match their preimages. The source side links all five
providers from the actual primary `bsp_core.lib`.

Three paired phases matched 862 DWORDs: 102 comparator pairs, 16 pair-copy
operations (including self/forward/backward overlap, changing counts and a
value address alias), and 60 tree operations across empty/populated trees,
hits/misses and output aliased to the actual tree header. This is fixture
evidence, not proof for every possible memory fault or concurrent mutation.

The strict MSVC Win32 build and both existing CTests passed. No tracked tests
were added. The APIs use new C++ calling conventions and are not binary
replacements. Full tree insertion, factory construction, live renderer profile
and game behavior remain separate work. Exact artifacts, hashes, ABI records,
saved annotations and validation limits are in
`reports/native_hardware_layout_tree_key_audit.json`.
