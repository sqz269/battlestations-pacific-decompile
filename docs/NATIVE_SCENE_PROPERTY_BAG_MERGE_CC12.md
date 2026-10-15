# Whole ordinary scene-property bag merge (CC12)

`merge_native_scene_property_bag_008f54f0` reconstructs the complete
`[008F54F0,008F566F)` body: 383 bytes, 131 instructions, fifteen call sites and
fourteen distinct callees. Its descriptive name is a hypothesis. The new
ordinary interface takes actual destination/source bags, the complete keep
DWORD and the existing `NativeScenePropertyCloneContext`; it returns no value.
Original ECX, RET8, stack/register/FS and exception transport remain separate.

Every call allocates a real twelve-byte iterator through the current canonical
heap, binds its first word to source bag `+4`, and calls the existing raw begin
routine. A nonnull empty source still allocates, begins and frees its iterator.
There is no count shortcut, null-source repair or allocation-failure recovery.

For each live node, the existing iterator key getter supplies its key or its
existing empty-string role. An actual aligned eight-byte query header goes
through the current raw string constructor: zero fields, length, raw resize and
copy. Lookup uses destination `+4` and the distinct canonical property/query
fallback cells from the existing insertion context. A hit's mapped record at
node `+8` is physically captured before the raw query destructor returns its
storage. Only after that return does the merge take its hit/miss branch.
No model host, callback, new fallback cell, new pool or provider wrapper is used.

A hit whose destination tag is six recursively merges the actual child bags
before inspecting keep, passing the same full DWORD and context. There is no
source-tag guard. Other hits call whole `008F0700` only when keep's low byte is
zero; a nonzero high part alone does not suppress assignment. Assignment keeps
its literal payload release/read order and untouched metadata rules.

A miss calls whole `008F4F60`, then physically rereads iterator `+4`, node `+8`
and source record `+34`, and stores that ordinal to the clone again. This
post-clone read/store is retained even though clone already copied the ordinal.
Only then does merge call the iterator key getter again and full `008F33F0`.
Insertion owns publication, owner backlinks and zero-ordinal stamping; merge
does not deduplicate or repair ordinals. The real iterator advances after each
branch and is freed once on normal completion.

The admitted domain consists of actual raw61-produced `0x114` bags, consistent
maps/counts/64 bucket heads, real live `0x14` nodes, raw-produced nonnull `0x38`
records, and canonical owning pooled keys. Source/destination owning graphs are
finite, acyclic, unshared and disjoint, with compatible matching tags on hits.
Source storage and frames survive all destination releases. Self, ancestor,
descendant, shared payload, mutation and reentry cases are outside this domain.
The source-tag and alias restrictions are preconditions, not added branches.

Keys are stable closed NUL-free ASCII, including empty, with nonoverflowing
lengths and current C locale. Canonical processes, fallback cells, pool
generations/membership and return gates remain live through retirement. All
whole clone and assignment contracts apply, including positive array spans,
clone strides and already-owning destination arrays. At every Type1 conversion
the x87 stack is empty, all six masks are set, PC is 24/53/64, RC is valid and no
unmasked exception is pending. Production adds no FP conversion or state reset.
Require DF zero, sufficient recursion stack and successful current providers;
invalid memory, overflow, OOM, provider throws and concurrent trim are excluded.

The first strict Win32 compile passed unchanged. Complete COFF inspection and
independent review account for all nine emitted functions: 776 instruction
bytes, 223 instructions and seventeen relocations, with no table, padding or
FP/SSE instruction. The main body is 718 bytes/198 instructions. Its sixteen
calls include three compiler-duplicated iterator-next sites and the security
cookie check. The mapped capture, query return, Type6-before-keep branch and
post-clone physical reread/store order are present in actual emitted code.

The required worker build passed both existing tests. Its current archive
contains the registered whole assignment and storage leaves; the new merge TU
was compiled separately pending integrator registration. The focused executable
linked that object against frozen dependencies from this completed build.
All 718 bytes and seventeen relocations match the actual linked merge body.
Five unused helpers have direct linker discard records. Three inline context
accessors have byte-identical selected probe COMDAT definitions with positive
discard records; those are distinguished from direct own-object discard lines.
The executable contains an embedded `asInvoker` manifest.

After independent prelaunch review, one actual Source process made three
top-level calls using real Type0/Type6 graphs. Keep `0x100` overwrote ordinary,
empty-key and nested hit values. Keep `0x101` preserved those hits while still
recursing and inserting the missing child. Both copied a zero-ordinal source
miss and let full insertion stamp destination ordinal three/counter four; the
nonzero child ordinal remained one. The third call used an actual empty source.
All nineteen source bag/node/key/record spans, hit identities and unassigned
record bytes remained intact. Same-context recursive cleanup completed, with
source storage rechecked after each destination retirement. Exit was zero,
stderr was empty, and all twenty selected file pins/stamps stayed unchanged.

This is a selected ordinary Source check, not all payload arms, an FP matrix,
an independent per-free trace or a leak counter. Full transitive provider and
loaded-module audits were not repeated. No Original Native target was run.
Historical CRT, Original ABI/SEH/EH/OOM, class dispatch and game behavior remain
outside this evidence. Shared registration, metadata, Ghidra annotations and
the Main registered-build gate belong to the integrator.
