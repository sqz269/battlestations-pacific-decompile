# SkinModel type-ID publication Source fragment

This adds an ordinary void C++ fragment for the retained, unowned Native span
`00CD8516..00CD8529` (20 bytes). It calls the supplied genuine counter provider,
reads the old `next_id_04`, writes the wrapping increment to that same returned
counter, and finally publishes the old value to the borrowed current
`01090344` word. Both writes remain volatile and ordered when those two words
alias; the final aliased word then contains the old value.

Baseline: `80a0c1753964aeb0e1c482ab11b747383f3d0f55`. The owned files are
`include/bsp/native_skin_model_type_id_publication_fragment.hpp`,
`src/native_skin_model_type_id_publication_fragment.cpp`, this document, and
`reports/cc12_skin_model_type_id_publication_source.json`.
Receipts and the frozen bundle are under
`local/cc12_skin_model_type_id_publication_source/`.

## Borrowed Source contract

`publish_native_skin_model_type_id_fragment_00cd8516` accepts a
`TypeIdCounterLifetime&` and `volatile uint32_t&`. The caller supplies the SAME
genuine existing counter lifetime and the actual current SkinModel word.
The function calls the real `get_006fac20()` method once. It introduces no
private counter, copied ID, alternate provider, descriptor, guard, or backing.

The returned storage is accessed through a volatile-qualified pointer. The
old value is captured once; unsigned 32-bit addition wraps modulo 2^32. Separate
full expressions write the increment to `counter->next_id_04` and the captured
old value to the borrowed target. There is no `restrict` contract, disjointness
precondition, equality shortcut, or last-write elimination.

The two public storage words may alias. Neither may alias private Source
objects, local variables, or reference bindings. The caller retains valid,
stable provider/storage bindings and lifetimes through the operation and
satisfies the provider's existing access contract. No fallback is added for an
invalid provider result, and no new synchronization or exception handling is
introduced. Provider effects and failures retain the existing Source semantics.

The new interface returns void. Its compiler-selected argument and result
registers are not claims about the original instruction fragment's ABI.
The genuine Source provider's abstract contract also does not prove a Native
getter ABI or establish whether the original runtime storage words alias.

## Retained Native evidence

The predecessor packet retained and compared exactly `[00CD8515,00CD8524)`.
Its verified-start suffix is `CALL 006FAC20; MOV ECX,[EAX+4]; LEA EDX,[ECX+1];
MOV [EAX+4],EDX`, spanning 14 bytes at `00CD8516..00CD8523`.
The separately accepted six-byte store at `00CD8524` is
`MOV [01090344],ECX`. These retained observations establish local publication
order on normal continuation, without establishing owning function or entry.

Root authorized this ordinary Source fragment after reviewing those packets.
No original bytes, metadata, body, prototype, callee, data, table, handler, or
additional scope were opened here. Initial `brief` health was the only Ghidra
contact. The retained modification-5 metadata is historical: Root's subsequent
annotation changed the parent program. It was not refreshed or treated as
current Ghidra state in this Source packet. Java CodeSource remains unattested.

## Ordinary MSVC x86 object evidence

The candidate and the unchanged `light_type_bootstrap.cpp`,
`sound_lifetime_access.cpp`, and `singleton_lifetime.cpp` compile successfully
with the actual generated main `bsp_core` Release Win32 settings. The complete
generated project and command tlog are retained. Flags include `/W4 /WX /O2
/Ob2 /Oy- /EHsc /MD /fp:strict /std:c++17 /TP`, the original definitions and
include settings, with paths relocated for the isolated objects. `/Bv` and
`/sourceDependencies` retain compiler identity and complete include receipts.
`/Oy-` already appears in the actual generated provider commands; it is not an
extra audit configuration. No separate flag-adjusted object is claimed.

The compiler is MSVC 19.51.36244.0, toolset 14.51.36231, Hostx64/x86.
Whole COFF objects, complete disassembly, all section contents, exact indexed
symbol and auxiliary records, and every relocation are retained for all four
objects. The candidate is 27 Source bytes in section 4, symbol table index
`0B`; its sole relocation is `REL32` at section offset `07`, to symbol index
`0C`, the real `TypeIdCounterLifetime::get_006fac20` declaration. The genuine
definition is symbol index `3C`, section 13, in the freshly compiled counter
provider object.

The candidate's selected code proves the Source ordering directly:

| Source code offset | Operation |
| --- | --- |
| `06` | Call the genuine counter provider |
| `0B` | Read old `[returned counter+4]` |
| `0E` | Calculate the wrapping increment |
| `11` | Store increment to `[returned counter+4]` |
| `14` | Load the borrowed target address |
| `17` | Store old value to the target |

Both stores are unconditional after the call/read. The compiler uses EDX for
the retained old value; that is ordinary Source code generation only. The
27-byte Source function includes its own prologue/epilogue and is not the
20-byte original fragment or a drop-in replacement.

The whole-object indexes contain, respectively, 5/28/16/148 sections,
15/91/56/478 symbol-table records including auxiliaries, and 1/44/17/306
relocations. A complete section-level dependency traversal reaches 86 sections
and 216 relocation edges. All five immediate external BSP calls from the real
counter definition resolve to actual compiled lifetime/allocation providers;
its internal EH handler is also retained. Downstream actual-manager creation
and registration remain explicitly listed Source frontiers, alongside CRT and
Win32 imports. Their complete existing Source files are pinned for review.
This enumerates the closure and its boundaries; it is not a whole-program link
or runtime-binding proof. Complete compiler dependency files and SDK headers
are pinned and copied, with baseline Git copies for unchanged project Source.

## Held integration gates

No guard, descriptor name/parents/layout, initializer, original owning function,
backing owner, production caller, CRT record, or original return-register
contract is admitted. `01090344` remains the SkinModel current word; adjacent
or similarly named resource families are not substituted.

CMake, ledgers, existing providers, and prior camera/SkinModel artifacts remain
unchanged. There was no GPR mutation, POST/script endpoint, restart, link, test,
probe, runtime execution, or game validation. Root owns later normal source
registration, build, and fragment-ledger review. Original function naming
remains gated until an owner exists.
