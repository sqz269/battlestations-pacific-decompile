# Compact type-ID publication Source fragment

This adds an ordinary void C++ fragment for the retained, unowned Native span
`00CD867A..00CD868D` (20 bytes). It calls the supplied genuine counter provider,
captures old `next_id_04`, stores the wrapping increment to the same returned
counter, then publishes the old value to the borrowed current `0109042C` word.
Both writes remain volatile and ordered when those words alias; the final
aliased word then contains the old value.

The baseline is current main `46640e9f773a282e190edd115aa54d25b9db87bd`,
Root's Source632 checkpoint. All selected existing Source paths match that
baseline. The only owned tracked files are
`include/bsp/native_compact_type_id_publication_fragment.hpp`,
`src/native_compact_type_id_publication_fragment.cpp`, this document, and
`reports/cc12_compact_type_id_publication_source.json`.
Complete artifacts are under `local/cc12_compact_type_id_publication_source/`.

## Borrowed contract and alias behavior

`publish_native_compact_type_id_fragment_00cd867a` takes the existing
`TypeIdCounterLifetime&` for the SAME process/lifetime domain and a
caller-owned `volatile uint32_t&` bound to actual current `0109042C`.
The genuine `get_006fac20()` runs once; no private counter, copied singleton
value, default ID, new domain, guard, descriptor name or storage is introduced.

The returned counter is accessed through a volatile-qualified pointer. The
old word is read once. Unsigned 32-bit addition wraps modulo 2^32; separate
full expressions store its increment to `counter->next_id_04`, then store the
captured old value to the current-word reference. There is no `restrict`,
disjointness requirement, alias branch, or last-store shortcut.

The counter member and target word MAY alias. Neither may alias private Source
objects, local variables or reference bindings. Supplied bindings, backing and
lifetimes must remain valid and stable through the operation. The provider's
existing access contract remains required. No fallback for invalid storage,
new synchronization, or exception translation is added.

The interface returns void. Its C++ calling convention and compiler-selected
argument/result registers are not the original fragment's ABI. The genuine
Source provider's abstract contract does not prove the Native getter ABI,
original EH behavior, or runtime storage-alias facts.

## Retained evidence boundary

The Root-reviewed Compact predecessor packet established four whole instructions
at `00CD867A`, `00CD867F`, `00CD8682`, and `00CD8685`: call `006FAC20`, read
ECX from `[EAX+4]`, calculate EDX as ECX+1, and write EDX to `[EAX+4]`.
The separately accepted six-byte `00CD8688` store writes ECX to `0109042C`.
This establishes the selected order on normal continuation, without an owning
function, entry, descriptor, or complete initializer.

This packet uses that retained evidence only. Epoch-10 typed metadata remains
historical and was not refreshed. Initial `brief` health was the only Ghidra
contact. No original bytes, metadata, prototype, body, callee, data, name,
handler, table, or new semantic window were opened. Java CodeSource remains
unattested. The prior SkinModel fragment supplied the existing Source-interface
model; the Compact behavior is supported by its own reviewed physical evidence.

## Current ordinary MSVC x86 evidence

The candidate and unchanged `light_type_bootstrap.cpp`,
`sound_lifetime_access.cpp`, and `singleton_lifetime.cpp` all compile with the
current generated main `bsp_core` Release Win32 settings. The complete actual
project and command tlog are frozen. Flags include `/W4 /WX /O2 /Ob2 /Oy-
/EHsc /MD /fp:strict /std:c++17 /TP`, the generated definitions and include
settings, with Source/include/output paths relocated for isolation.
`/Oy-` already exists in the actual commands; no extra audit variant is claimed.
`/Bv` and `/sourceDependencies` preserve compiler identity and full include data.

The compiler is MSVC 19.51.36244.0, toolset 14.51.36231, Hostx64/x86.
The generated project has 1,914 `ClCompile` items; Source632 is the Root checkpoint
label, not this item count. The new candidate is not registered in that project.

All four whole COFF objects have complete section contents, indexed symbol and
auxiliary records, relocations, and full disassembly retained. The candidate is
27 Source bytes, symbol index `0B` in section 4. Its sole `REL32` relocation at
section offset `07` names index `0C`, the genuine counter getter; the freshly
compiled definition is index `3C`, section 13 of `light_type_bootstrap.obj`.

| Source code offset | Verified operation |
| --- | --- |
| `06` | Call genuine `get_006fac20` |
| `0B` | Read old returned-counter word at +4 |
| `0E` | Calculate wrapping increment |
| `11` | Store increment to returned-counter word |
| `14` | Load borrowed destination address |
| `17` | Store captured old value to destination |

Both stores remain unconditional after the call/read, so an alias still receives
the increment followed by the old value. EDX holds the old value in this Source
object; this is not a Native register claim. The Source function's prologue and
epilogue make it 27 bytes, distinct from the retained 20-byte Native fragment.

The four indexes cover 5/28/16/148 sections, 15/91/56/478 symbol-table records
including auxiliaries, and 1/44/17/306 relocations. The complete section-level
closure traversal records 86 sections and 216 edges. All five immediate external
BSP calls from the counter resolve to actual compiled lifetime/allocation
providers, and its internal EH records are retained. The traversal separately
records 72 external-frontier edges and five resolved weak-alias boundary records.
Downstream actual-manager creation/registration Source frontiers and CRT/Win32
imports remain explicit. The corresponding existing Source files are pinned;
no full-program linking or runtime binding is claimed.

Complete current Source and compiler dependencies are copied, with baseline Git
versions for every unchanged project input. Compiler/tool binaries are pinned,
and full external SDK headers are copied. Prior camera, SkinModel, and Compact
boundary receipts remain unchanged.

## Integration remains separate

No full initializer, owning function, descriptor layout/name/parents, production
backing/caller, CRT record, Native ABI/EH, startup, or runtime behavior is admitted.
Current `0109042C` remains Compact-specific; no other resource family is substituted.
CMake, ledgers, providers, and existing Source are unchanged. Root owns later
normal registration, build, and fragment-ledger review.

There were no GPR mutations, POST/scripts, restart, tests, probes, links, runtime
execution, or new Native analysis. Compilation and Source object inspection do
not establish a drop-in Native replacement or game validation.
