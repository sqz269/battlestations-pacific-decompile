# Complete ordinary property-library insertion/replacement

`insert_or_replace_native_property_library_entry_008f29a0` reconstructs the
complete ordinary body at `[008F29A0,008F2A4C)`: 172 bytes, 65 instructions,
four direct calls and one Original virtual call. The last `RET 8` begins at
`008F2A49`. The descriptive name remains a hypothesis. This is a new
context-bearing MSVC Win32 C++ API. Original takes ECX=actual108h map at a
raw10Ch library+4, stack=query8h/incoming bag word, and returns the current
count in EAX with `RET 8` on both paths. Its transport is not supplied here.

The prior read-only readiness artifact is pinned in the report. Its missing
providers have since landed on Main: distinct actual38h E17540 outer-node
pool, distinct retained E18560 empty role, complete raw004689D0/00468ED0
readers and complete outer008F4C50/008F56A0 lifetime. The accepted immediate
Source providers are pinned, not re-proved.
The original complete stored listing has SHA-256
`1be95e92228d34635ff90b6f451dab1d728e11d6494449df893ab11df9034376`.

## Concrete context and admitted ownership

`NativePropertyLibraryInsertContext` retains the same concrete
`GameNativeStringProcess` used to construct its existing
`NativePropertyTreeLibraryLifetimeContext(actual_strings,actual_physical)`.
The lookup receives that process's distinct E18560 and E17654 roles. Key
resize uses `lifetime().bags().keys()`, old-bag retirement uses the same
`bags()` context, and allocation uses its distinct `library_pool()` E17540.
The inner E175B0 pool and common E188B4 list remain those of the canonical
physical process. No raw pool/callback/provider graph can be injected.

The context starts nothing. Explicitly complete both CC8A30 inner and CC8A70
outer pool startups before constructing it, and retain all processes,
providers, pools, string manager/pool/gate generation and shared list through
their borrowers. A nonzero CRT registration status does not undo successful
pool construction. Externally serialize common-list mutations: separate
startup mutexes do not provide that synchronization.

Use a genuine raw10Ch root initialized by whole008F5670, its actual108h map,
coherent64 heads/count and unique genuine14h outer slots with intact
allocator+10 membership. Incoming null is admitted. Nonnull incoming bags
must be genuine same-current-heap114h roots admitted by whole raw61
008F41A0/wholeclone008F41F0 and existing complete producers. Their owning
graphs must be finite, unshared, acyclic and disjoint from old ownership,
query/key, node and map storage. A phase DWORD alone is not provenance.
Successful publication transfers incoming root/graph ownership into the map.
Retire each bag, then its outer pooled key, then its E17540 slot before pool
page teardown. No application loader/parser is required or implemented.

All inherited raw lookup, bag lifetime, allocator and resize limits remain:
stable closed NUL-free ASCII keys with consistent lengths, null data only
when empty, current CRT C locale, finite correctly bucketed chains, valid
nonaliasing storage, successful providers, no overflow/fault/concurrent
mutation/trim/reentry, sufficient recursion stack, and the same live string
return gate/generation. No new null-allocation recovery or ownership-alias
shortcut is added.

## Complete body schedule

| Original sites | Source behavior |
| --- | --- |
| 29AF | Call whole00468ED0 with a nonaliasing local bucket and both retained empty roles. Historical consumed-stack-slot identity is not claimed. |
| 29BA..29D7 hit | Physically read node+8. If nonnull, directly call the accepted fixed bag scalar008F59E0(old,1,same bags context); physically zero+8 only after return. Publish incoming, then physically reload map+4 count. No same-pointer shortcut. |
| 29E0..29FF miss | Allocate with whole004E7C00 on the distinct E17540. Successful fresh slot zeros only key0/4; preserve allocator+10. The Original failed-allocation branch continues toward a fault; ordinary admission requires success. |
| 2A01..2A21 | Retain node==query identity branch even though ordinary admission is disjoint. Otherwise physically read current query length, resize actual raw node key with preserve=true, physically reread query length after return, and if nonzero physically read current destination length, source data then destination data. Copy that current destination length, without strlen or an added terminator byte. |
| 2A29..2A43 | Physically publish incoming at node+8, read current head at lookup bucket, store node+C, publish the head, increment map+4 in memory, then physically reload count for return. |

Inline x86 physical helpers retain the provider/publication schedule under
optimization. Original's numeric virtual0 phase dispatch is replaced only
for admitted real bags by the already accepted fixed scalar Source helper;
no numeric phase value is called as a Source vtable. The interface does not
construct a `NativeString` object over raw8h storage or fabricate bag graphs.

## Validation and evidence boundary

The first strict compile uses MSVC x86 19.51.36244.0 with
`/std:c++17 /MD /O2 /Ob2 /Oy- /W4 /WX /EHsc /Gy /GL- /fp:strict /c`.
Its original tool result and compiler log are preserved under
`local/WholePropertyLibraryInsertSource01/`. Complete COFF inspection covers
every code section, every decoded byte, all symbols and all relocations;
the machine schedule is recorded in the report. Dependency and Source pins
allow the integrator to verify the reviewed family unchanged.

The required repository `./scripts/build.ps1` runs separately in this
worktree; its original handle is preserved to natural completion. The new
TU is intentionally unregistered here because shared CMake/registration is
owned by the integrator. This ordinary strict TU compile and existing-target
build do not establish registered integration or Native execution.

No new test, target/probe runtime, Ghidra write, ledger, shared header, CMake
or application wiring is part of this packet. Original register/ECX/RET8,
class/vtable/FS/SEH, historical/private CRT, exceptional/fault behavior and
gameplay remain unproven. The report records the final build and independent
review status; the context-bearing C++ interface is not a drop-in binary.
