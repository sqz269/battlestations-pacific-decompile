# Input bit/sensitivity-map full cleanup

The new `native_input_bit_sensitivity_map_full_cleanup.hpp/.cpp` supplies two admitted
full-path fragments and two complete whole destructors over actual owned
storage. Existing correct STL names remain unchanged. New C++ names are
behavioral hypotheses, not recovered symbols.

| Entry | New Source coverage | Original ABI |
| --- | --- | --- |
| `00559280` | ONLY full prefix through `005592FE`,127B/49 instructions | ECX tree, EDX unused; five stacked words output/first-owner/first-node/last-owner/last-node; RET14h/EAX output |
| `00559A90` | ONLY full prefix through `00559B0E`,127B/49 instructions | Same physical input/result shape |
| `0055A430` | Complete through `0055A463`,52B/23 instructions | ECX tree, EDX unused, no stacked inputs; RET/EAX0 |
| `0055AAF0` | Complete through `0055AB23`,52B/23 instructions | Same physical input/result shape |

Source retains those physical input words and appends the actual
`NativeStringRawPoolContext&`. The extra reference changes the Source stack
shape; original RET sizes describe the game entries. This is not a binary
replacement claim.

The bounded readiness check is closed. The prefixes bind accepted whole
raw-pool subtrees `00558420` and `00558ED0`, published with root `68920b203`
in Main073. Both use actual2Ch nodes, nil29h, pooled8B key at+C and raw
backing1Ch/end20h/cap24h. Their payload schedules remain DISTINCT: the bit
leaf captures left, clears count14h, then captures/frees backing; sensitivity
captures backing BEFORE left and preserves14h/18h. Both keep current pooled
key reads/getter/return and current CRT node frees. Each new prefix calls its
named accepted subtree directly; sharing metadata operations does not merge
these payload semantics. Getter failures propagate without a noexcept
NativeStringStorage bridge, provider or callback.

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
`/battlestationspacific.exe`. Complete typed metadata at modification481
retains two200B/two-range ordinary erasers and two52B/one-range whole
destructors, all non-thunk/non-no-return. The full inspected envelopes have
506 identical installed-PE/live bytes;504 defined bytes and198 instruction
starts match independent complete decoding. The two one-byte gaps at
`005592FF`/`00559B0F` are retained outside typed bodies. Source-owned spans
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
0055929A CALL 00BF6713
005592b6 CALL 00BF6713
005592CA CALL 00558420
0055930A CALL 00BF6713
00559319 CALL 00552E90
00559327 CALL 00557F90
00559AAA CALL 00BF6713
00559AC6 CALL 00BF6713
00559ADA CALL 00558ED0
00559B1A CALL 00BF6713
00559B29 CALL 00552770
00559B37 CALL 00558a60
0055A446 CALL 00559280
0055A44F CALL 00BF65AC
0055AB06 CALL 00559A90
0055AB0F CALL 00BF65AC
```

Existing real AV/full-plus-partial fixture/projection domains remain intact.
No larger parent, other map or future unwind action is supplied. Finite
unique ownership, admitted reached storage and accepted actual pool/current
CRT services are required. Added context/by-value/private Source cells and
normalized dead EDX do not establish original stack/register/flag/privateframe,
heap/CRT/FH3/SEH/fault/exception, binary ABI, runtime or gameplay identity.

During its short FIFO registration turn, the clean worker merged current
Main `2897ae4ba02f5d2aef1a8b737cda91df9c7a790d`. After the append-only startup
merge failed, `tools/merge_resolve.py` resolved it by registry union; merge
commit `a45f40bc6f90fd452748e6e7968ac4acf6b1ebde` retained existing lines.
One bsp_core Source registration was appended, then the lease was released
before building. No queue jump occurred. The previous accepted bee75c0f
Source/header/doc/report hashes remain unchanged.

`scripts/build.ps1` compiled Win32 Release including this Source; the three
existing suites passed in7.64s. All16 JSON transfer rows and16 Markdown
claims passed with zero failed/unchecked claims. No new tests or Ghidra
mutations were performed. Existing suites do not newly differential-test
these entries against game code. The preexisting unrelated LNK4006 warning
remains.
