# Native award registry and game payload-list lifetime (R126)

Addresses: `00487110`, `00489CA0`, `004C7DD0`, `00501FA0`, `006B9120`,
`006B9380`; consumed library branch `006B91F0`; parent `004DCF90`.

## Result

Six complete normal bodies (767 bytes) now close two more game-destructor
bindings. `4DD037` destroys the award registry using the game's existing raw
string-pool context and retained `operation.awards`. `4DD134` clears the four
game payload lists using retained `operation.containers`. The parent refuses
diagnostic retirement while an award child remains running or failed.
Names describe recovered behavior and remain hypotheses, not recovered symbols.

| Entry | Bytes | Recovered behavior |
| --- | ---: | --- |
| 487110 / 489CA0 | 72 / 5 | Reset `{opaque0,head4,count8}` sentinel links/count; capture next before each node free; compare current head; free current sentinel and clear4. 489CA0 is a JMP thunk. |
| 501FA0 | 256 | Free current record buffer48; clear48/4C/50; destroy list30; release string headers28,1C,14,C,0 without clearing headers. Each string data/size is captured before the getter/return pair. |
| 6B9120 | 149 | 6Ch tree node with nil69. Recurse right, then capture left, destroy value14 and keyC/10, free node, follow captured left. |
| 6B9380 | 149 | Destroy the current full tree10, free current head14, clear14/18; then destroy vector4..8 strings, free current begin4 and clear4/8/C. Preserve0/10/1C. |
| 4C7DD0 | 136 | For four0Ch lists beginning game7134, free each node+8 payload before the node pass. Read next after payload free and compare current head. Then reset current sentinel/count and free nodes using captured next. Sentinels survive for later array destruction. |

List, record and registry entries use ECX owner and RET. Subtree cleanup uses ECX
tree, one stack node and RET4. The 201-byte `6B91F0` library reference uses ECX
tree, five stack arguments and RET14. Its source adapter accepts only the current
full range reached by the registry destructor; general partial erase remains
unimplemented. No generic STL replacement or invented record class is supplied.

`NativeGameProfileLifetimeContext` supplies the same actual raw string domain
already used by game cleanup. The award operation borrows that context and records
separate registry/node/record unwind states. Failure retains the graph and refuses
replay; explicit diagnostic retirement performs no free or rollback.

## Layout and analysis corrections

`6B8C30` allocates6Ch nodes with color68/nil69. `50FC30` returns node14 for the
value. `506270` calls `502110` on record28, initializing its Unlock string and
Index list; `4870F0` allocates its0Ch self-linked sentinel. The registry constructor
`6B9450` reads `Achievements.lua` and the game publishes its20h object atE19900.
The actual constructor/parser is outside this packet's implementation scope.

The map count is registry18. Registry1C is a separate parsed-row counter, incremented
at instruction `6B9C53` and preserved by the destructor. The older award-grant note
called1C the map size and cited6B9C56, the instruction's final byte. Its correction
is appended in `docs/AWARD_GRANT.md`.

Seven returning-call gaps totaling139 bytes were repaired. Five interior gaps total
50 bytes. The other89 bytes are truncated tails:12 bytes after487147 and77 bytes
after6B93C3. Functions now end at RET487157 and RET6B9414. Ghidra removed the
489CA0 thunk while recreating its target; the verified five-byte jump and its
original automatic name were restored. Prior target metadata and every repair
are recorded. Final flow audits have no returning-call gaps; one unreachable
padding byte after6B926C RET remains untouched.

The final call audit found that the newly decoded diagnostic call4C7E18 was still
outside Ghidra's stored body membership despite appearing in its linear listing.
Recreating4C7DD0 through4C7E57 preserved the name/comment and included that call.
All36 direct call rows now pass the live containing-function/listing checks.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. All968 copied bytes
(767 normal bodies and201 library-reference bytes) match live Ghidra and the
installed PE. The 79 paired cases match989 observations and4,796,352 normalized
bytes against the combined nested source cleanup. They use real raw string
construction/pool/manager services, the existing432050 string-range destructor,
and two actual parent defaults. Record/registry graphs are explicit fixtures.

Coverage includes empty/one/three-node trees with left/right branches, five-string
masks, small-return gate0/1,149/150-byte string allocation boundaries, list/thunk
cleanup, null payloads, and four-list payload-before-node ordering. Callbacks change
tree/list heads, counts, next/left links and string lengths to check precisely which
values are captured and which are reloaded. A returning post-payload-free diagnostic
is exercised after a callback temporarily makes the current node the sentinel.

One source-only failure after tree cleanup retains exactly the sentinel and string
vector allocations, retains the context, and rejects replay without further calls.
Three guards reject missing/foreign contexts and an actual partial tree range.
All remaining fixture allocations are explicitly drained.

The separate controlled parent comparison passes52 paired cases,3,537 snapshots,
141,402,076 normalized bytes and four failure/replay cases. It checks retained
award/container identity; the nested fixture supplies the concrete body evidence.

## Remaining work

Bind the remaining parent dependencies and the raw game owner into the application.
The ordinary executable still does not reach these bodies, so this packet does not
repeat an unrelated application run. Real registry construction/session ownership,
general partial erase, native exception cleanup, private-stack aliases, arbitrary
invalid graphs, concurrency, binary ABI compatibility and gameplay remain open.
