# Actual hidden-entity map lookup

Addresses: `00468140`, `00468CD0`.

The new ordinary C++ in `src/native_scene_record_map_lookup.cpp` walks actual
tree storage. It reuses the accepted concrete string-header comparator and
returning CRT report body; it introduces no host/comparator callback, copied
container, STL replacement, allocator or unresolved-call substitute.

| Routine | Body | Coverage | Native ABI |
| --- | --- | --- | --- |
| `STL_MapNativeStringPtr_LowerBound` | `00468140..00468192`, 83 bytes | complete, qualified Source/provider domain | ECX map; stack actual8h key-header pointer; `RET 4`; EAX node |
| `STL_MapNativeStringPtr_Find` | `00468CD0..00468D35`, 102 bytes | complete, qualified Source/provider domain | ECX map; stack hidden iterator output/key-header pointer; `RET 8`; EAX output |

These established library names remain unchanged. Current indexed ledgers had
no reconstruction for either candidate. `scene_record_map.cpp` provides an
older vector/string projection which explicitly omits the tree; it does not
implement these actual-header bodies. Existing host consumers remain separate.

The public APIs use `__fastcall` with an explicitly unused EDX word so that
ECX, the native stacked words and pointer result have their physical roles:

```cpp
void* __fastcall lower_bound_native_scene_record_map_00468140(
    void* actual_map, std::uint32_t unused_edx, const void* actual_key_header);
void* __fastcall find_native_scene_record_map_00468cd0(
    void* actual_map, std::uint32_t unused_edx, void* actual_iterator_output,
    const void* actual_key_header);
```

The layout authority is the producer and the existing bounded reference
`docs/SCENE_RECORD_MAP.md`. The insertion producer at `0046AD70` publishes a
first node into head+4 and takes left/right fields +0/+8 when linking later
nodes. The constructor listing `00469F50..00469FBF` writes links +0/+4/+8,
constructs the native length/data pair at +0Ch/+10h, places its mapped word at
+14h and writes node+19h=0. Its allocator requests 1Ch bytes. The insertion
descent at `0046B340` and both lookup listings read exactly these offsets.
No new node/map/iterator structure or conflicting field declaration is added.
Lookup only needs map+4 head, head+4 root, node left/right, node+19h sentinel
and the existing actual8h key header; it never reads mapped value, proxy, size
or color. SceneDatabase callers supply their map subobject at +18h.

Lower bound captures the initial head once and begins at its root. It tests
node+19h before reading any key/header data. The inlined comparison reads node
key length first, then query length. Empty is less than nonempty regardless of
data pointers. Two nonempty headers read query data before node-key data and
compare C strings through CRT `__stricmp`; there is no length tie-break or
length-bounded byte comparison. The accepted concrete
`less_native_string_headers_00443d00` body has that exact read order and Boolean
rule, so Source reuses it for the native inlined predicate. The native lower
bound directly calls CRT at `00468170`; it does **not** call `00443D00`.

If node sorts before the query, descent reads CURRENT right+8 after comparison.
Otherwise it remembers node and reads CURRENT left+0. A nonzero sentinel ends
the loop. The remembered candidate, or the captured initial head when none
exists, is returned. No allocation, normalization, owner check, null-header
guard, empty-tree special replacement or map-head reread is introduced.

Find calls the exact lower bound first, then performs the native null-map CRT
report check. This order is retained even though normal valid backing must
already have supported the descent. A returning report continues. It compares
the found node with CURRENT map+4. If unequal, it compares query key against
node+0Ch using the accepted concrete comparator. Either end or a true trailing
comparison selects fallback by rereading CURRENT map+4, after possible CRT
effects. Otherwise the original lower-bound node is retained. Owner and node
are both captured before output+0 owner, then output+4 node stores. Output may
alias map, node or key storage; publication does not reread a field overwritten
by its first store. Find returns the exact output address.

| Native call site | Native target | Concrete Source composition |
| --- | --- | --- |
| `00468170` | `00BF7FBF` | Accepted `less_native_string_headers_00443d00` realizes the identical inlined length/C-string rule using current CRT; caller `ADD ESP,8` |
| `00468CDD` | `00468140` | Same-packet complete actual lower bound; child `RET 4` |
| `00468CEC` | `00BF6713` | Existing `invoke_native_invalid_parameter_00bf6713`; no invented report callback, no noreturn/noexcept promise |
| `00468D01` | `00443D00` | Existing actual two-header comparator, query before node key; child `RET 8` |

All callee bodies were read first. Native CRT `00BF7FBF` dispatches to ASCII or
locale comparison, or reports invalid nonnull-argument requirements; accepted
Source policy uses the current SDK CRT. The owned caller bodies and both full
listings settle stack cleanup, sentinel branching, rereads and ordered output.
The original bytes were read live and are retained in the companion report.

All live direct xrefs were checked. Lower bound has one caller, find itself.
Find has four callers, whose containing-function boundaries and argument setup
were read: `004691D4` in `004691B0`, `004694A8` in `00469480`, `0046D96F` in
`0046D930`, and `0046DC55` in `0046DC10`. Each supplies SceneDatabase+18h,
a local eight-byte iterator output and a temporary native name header.
The callers consume owner/node and sentinel equality, so Source preserves the
iterator result rather than replacing it with mapped-record lookup or Boolean.
No caller/provider files are edited in this packet.

Live clients verify project `bsp`, program `/battlestationspacific.exe`, x86
language and image base before each batch; the configured project file is
`C:/Users/sqz269/bsp.gpr`. Locked evidence annotation preserves prior comments
and names, logs old values, saves the project and refreshes affected exports.

The companion report records verification. After the shared CMake holder
released its lease, this packet unqueued and appended only its unique source
registration, without claiming an exclusive shared-registry lease. A registered
build is required before a build-tested claim. No new tests are added. Source inherits
the concrete current-CRT locale/handler/provider domains and requires valid
taken backing. It does not expose native private argument slots or private
iterator temporaries, reproduce native binary/SEH/fault-address identity, or
establish direct native execution, application adoption or game validation.
