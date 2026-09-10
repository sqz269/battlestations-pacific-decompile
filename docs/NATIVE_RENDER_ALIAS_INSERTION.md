# Actual renderer alias insertion

`append_native_render_alias_004d2660` reconstructs the complete 50-byte native
single-insertion body. `insert_native_render_alias_range_004d26a0` reconstructs
the complete 276-byte range body, including its parent-bound catch at
`004D2746`. Both operate on the actual embedded list: preserved word at +0,
sentinel at +4 and count at +8. Nodes are the established 16-byte next/previous
and pooled-string storage. They reuse the same concrete sized pool, ordinary
node allocation, count guard and checked iterator/erase implementations.

The original single insertion takes ECX=list and one source-string-header
pointer on the stack, returns with `RET4`, and has no semantic return. It
captures the destination sentinel and its previous node, allocates/copies the
new node, grows the count, writes captured sentinel.previous, then reloads
new-node.previous to publish its next link. A count-growth exception leaves
the successfully constructed but unlinked node allocated: the original has
no local cleanup covering that call. This implementation preserves that behavior.

The range original takes ECX=destination and seven stack DWORDs, returning
with `RET1Ch`. Six words are three iterator pairs, passed by value: insertion
position, working source and source end. The seventh word is never read; the
new C++ interface does not assign it invented semantics. The three copied
pairs and saved initial source remain actual mutable working storage.

The normal loop captures the insertion node once. Each iteration validates
captured source owner against the current end owner and tests captured source
node against current end node. Its subsequent owner/sentinel checks remain
separate; returning invalid-parameter handlers continue at the original next
operation. It allocates/copies before count growth and link publication.
The source owner for the advance check is captured at `004D2727`, before either
link write. After that check it reads the captured source node's current next
pointer, reloads the working source owner, and publishes the advanced node.

The catch compares the saved initial iterator with the current working source.
If they differ it captures the current insertion argument pair anew, repeatedly
restores that pair, obtains its previous node, erases that node using the source
end argument as output storage, then advances the saved source. It stops when
the saved and working sources compare equal and rethrows. The rollback count
therefore follows completed source advances, not merely linked destination
nodes. A rollback exception can supersede the initial exception.

Original FuncInfo `00D8EB30`, try map `00D8EB1C`, catch descriptor `00D8EAFC`,
and the two empty unwind states at `00D8EB0C` establish this catch. Its native
rethrow calls `00BF6885(0,0)`. `Catch_All@004D2746` remains a parent fragment,
not a separately callable C++ function or an additional complete-body count.

Independent assembly review found no discrepancy in the implementation. One
focused private fixture executes original single insertion followed by the
entire original range body and catch with mapped native metadata under the
actual host `__CxxFrameHandler3`. Concrete shared node/count/iterator/erase
helpers supply its already-reconstructed boundaries. A real allocation hook
changes the source sentinel while copying the second node; the actual CRT
invalid-parameter handler then throws after that node is linked. Both versions
remove the newest node, retain the earlier copied node and preexisting alias,
preserve the source mutation, and rethrow the same marker. All twelve actual
node/string/sentinel allocations are eventually released by concrete cleanup.
The 257-word normalized transcripts match. The strict MSVC Win32 build and
both existing CTests pass. No new tracked test suite was added.

This fixture specifically verifies partial rollback and the preceding single
insertion. It does not inject the count-limit error into the original range
body; that owning error transport has a separate focused native fixture.
These new C++ interfaces require valid actual graphs and shared allocation
domains, and retain the declared host exception/runtime boundaries. They do
not establish original binary ABI compatibility, full renderer ownership,
or gameplay validation. See `reports/native_render_alias_insertion_audit.json`
and `reports/native_render_alias_insertion_verification.json` for exact spans,
source snapshots, hook preimages and fixture provenance.
