# Actual scene class-name lookup

Ordinary raw Source covers all 277 bytes and 94 instructions at
00469690..004697A4. The native descriptive hypothesis
`BSP_SceneDatabase_ClassNameToId` is retained. Native ECX receives the scene
database, one stacked DWORD supplies the class-name pointer, RET4 returns
the class-id DWORD in EAX or zero. This Source adds explicit frame/pool/fallback
bindings and is not a binary replacement interface.

Publish database+34h in the actual iterator table word, then call the complete
raw Begin provider before reading the current input cell and constructing the
actual8B key header. Capture current iterator.node, header.data and header.length
in that order after construction. Keep the latter two captured values throughout.
No temporary header, iterator or private slot is zeroed by the caller.

For each captured node, read its +4 text pointer, falling back to the distinct
actual NUL role00E18560 if null. A null cached query pointer takes the native
byte-by-byte unsigned-wrapping length scan and matches only an empty text;
both-null matches directly. A nonnull query and null text compare cached length
with zero. Both nonnull use the actual current SDK `_stricmp`. A hit reads
node+8 then the pointed DWORD before release, retaining the result across it.
A miss calls complete raw Next, ignores its return and reloads current node.

Both normal returns release only the captured data pointer, using captured
length+1 modulo32. Null data skips getter and size work. A nonnull release
resolves the current raw pool publication before BD1510; no pool cache,
header reread/reset, automatic destruction or retry is introduced.

The entire listing initializes the FH3 state to-1 and only stores-1 again at
0046970B/00469772: it never arms state0. Handler C61B38 selects FuncInfoD891AC,
maxstate1/mapD891A4: state 0 unwinds to -1 through the 8-byte cleanup C61B30.
That action's existence does not justify inventing an outer Source cleanup
guard. No private action coverage is claimed by this packet. Implicit aliases
with native frame-state words and original EH/SEH/fault transport are unproved.

Entry ESP=P gives header8 at P-20h, iterator12 at P-18h and the live input cell
at P+4. The corresponding backing regions are distinct; binding metadata is
stable/disjoint from payload. Reached actual storage/aliases, C strings, pool
lifetimes, fallback and current CRT policy must be valid. No class-map
representation, count consistency, ASCII clamp, ownership or game proof is
inferred. All277 live/installed-identical bytes and independent whole94-op Source review passed. The normal MSVC Release Win32 build passed all three existing CTests in 7.99s; fourteen typed transfer rows passed. No new tests were added. Receipt: reports/goal_oct15_scene_lookup_primary_review.json.
