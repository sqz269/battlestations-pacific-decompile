# Input vector-map full cleanup

The new `native_input_vector_map_full_cleanup.hpp/.cpp` supplies two admitted
full-path fragments and two complete whole destructors over actual owned
storage. Existing correct STL names remain unchanged. New C++ names are
behavioral hypotheses, not recovered symbols.

| Entry | New Source coverage | Original ABI |
| --- | --- | --- |
| `00559110` | ONLY full prefix through `0055918E`,127B/49 instructions | ECX tree, EDX unused; five stacked words output/first-owner/first-node/last-owner/last-node; RET14h/EAX output |
| `005599C0` | ONLY full prefix through `00559A3E`,127B/49 instructions | Same physical input/result shape |
| `0055A3F0` | Complete through `0055A423`,52B/23 instructions | ECX tree, EDX unused, no stacked inputs; RET/EAX0 |
| `0055AAB0` | Complete through `0055AAE3`,52B/23 instructions | Same physical input/result shape |

Source retains those physical input words and appends the actual
`NativeStringRawPoolContext&`. The extra reference changes the Source stack
shape; original RET sizes describe the game entries. This is not a binary
replacement claim.

The bounded readiness check is closed. The word and descriptor prefixes bind
accepted whole raw-pool subtrees `00558310` and `00558E60`, already published
with `dfbcdaee` on Main073. Both use actual28h nodes, nil byte25h, pooled8B key
at+C, mapped opaque14h/raw backing18h/end1Ch/cap20h. Their accepted complete
Source preserves backing-before-left captures, current key reads and getter
failure propagation. Descriptor payload cleanup does not add an element
loop. Final sentinel frees use accepted `singleton_lifetime_free` current
CRT. No missing provider is satisfied by a callback or noexcept storage bridge.

The prefix APIs require a nonnull actual owned12B header/head, both owners
equal to the header, first equal to the initial head.left, last equal to the
fresh head, and stable exclusive storage through selection. Their writable
8B output is private in the actual whole consumers. Native TEST/CMP/load
order is retained for these admitted branch outcomes. The diagnostic and
partial outcomes have no replacement guard, abort, fabricated diagnostic or
stub in these interfaces. The ordinary all-cases range erasers remain
incomplete in this new raw-context domain.

Each complete destructor captures head H and then H.left, passes its own
header as both owners, H.left as first, and H as last. No provider/callee
intervenes before those arguments reach selection. On valid stable owned
storage, the first owner and first-node comparisons select the full path;
the second owner and fresh-head comparisons confirm it. Subtree/getter work
starts afterward. Empty trees still use the actual nil sentinel, without a
null-head or repeated-destruction skip.

After selecting the full path, each prefix loads CURRENT head.parent as the
subtree root. On successful return it reloads head, resets head.parent,
reloads head BEFORE setting tree.count=0, writes captured head.left, reloads
head and writes head.right, then reloads head.left for the result. Output
owner is published before output node; the result node is captured before
either store. There is no direct header+0 store; admitted output aliases
follow the ordered output writes. The whole consumers' private outputs keep
opaque header+0 unchanged. The two prefixes share only the proven ordered
selection/reset/output implementation; each calls its named actual subtree
service directly.

Each complete destructor uses private8B output and, after the prefix returns,
reloads CURRENT header.head for free. It then zeros+4 followed by+8 and
returns DWORD0. It preserves opaque+0 and has no local EH/catch, rollback or
cleanup retry. A pooled getter failure propagates through current C++ with
earlier subtree effects retained; later resets, sentinel free, header clears
and return do not run.

Read-only tools verified `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Complete typed metadata at modification456
retains two200B/two-range ordinary erasers and two52B/one-range whole
destructors, all non-thunk/non-no-return. The full inspected envelopes have
506 identical installed-PE/live bytes;504 defined bytes and198 instruction
starts match independent complete decoding. The two one-byte gaps at
`0055918F`/`00559A3F` are retained outside typed bodies. Source-owned spans
have358 identical bytes and144 starts. The prefix pair agrees in115 bytes
outside twelve external rel32 bytes and all normalized instructions. The
whole pair agrees in44 bytes outside eight external rel32 bytes and all
normalized instructions. Sharing selection/reset operations is therefore
backed by complete spans and target roles, rather than a generic tree model.

The following complete native call evidence includes unselected diagnostics
and excluded partial code. Only the admitted full paths and whole wrappers
are reconstructed here; observing a partial call does not bind its Source.
All branches to partial code are conditional/internal, not external tail
calls. No local EH registration/action appears in the inspected bodies.

```text
0055912A CALL 00BF6713
00559146 CALL 00BF6713
0055915A CALL 00558310
0055919A CALL 00BF6713
005591A9 CALL 00552D40
005591B7 CALL 00557CA0
005599DA CALL 00BF6713
005599F6 CALL 00BF6713
00559A0A CALL 00558E60
00559A4A CALL 00BF6713
00559A59 CALL 00552DB0
00559A67 CALL 00558770
0055A406 CALL 00559110
0055A40F CALL 00BF65AC
0055AAC6 CALL 005599C0
0055AACF CALL 00BF65AC
```

Existing real AV/full-plus-partial fixture/projection domains remain intact.
No larger parent, other map or future unwind action is supplied. Finite
unique ownership, admitted reached storage and accepted actual pool/current
CRT services are required. Added context/by-value/private Source cells and
normalized dead EDX do not establish original stack/register/flag/privateframe,
heap/CRT/FH3/SEH/fault/exception, binary ABI, runtime or gameplay identity.

During the short FIFO registration lease, the clean worker merged current
Main `fdaa95a514a7fa1ed6c62c2967ba3d94f2a9f940`. The append-only startup
conflict with its previously accepted fill registration was resolved after
merge failure by `tools/merge_resolve.py` registry union; merge commit
`88d657a26b825220f7870cf20351739111c081c0` retains both. One bsp_core Source
registration was appended, then the lease was released before building.
`scripts/build.ps1` compiled Win32 Release including the new Source; the
three existing suites passed in7.81s. No new tests or Ghidra mutation were
performed. These suites do not newly differential-test these entries against
the game. Transfer-check results are recorded in the JSON report.
