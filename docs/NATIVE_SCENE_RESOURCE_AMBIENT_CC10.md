# Raw scene-resource ambient setter

The setter operates on actual3Ch resource storage and actual98h ambient owners.
It publishes only resource+10 and writes raw resource identities into borrowed
backlinks. Host companion pointers, logical SceneResource objects, automatic
admission, replacement counts and extra reference credits are not introduced.

| Range | Bytes | Source entry | Coverage |
| --- | ---: | --- | --- |
| B825D0..B82628 | 89 | `set_native_scene_resource_ambient_00b825d0` | Complete normal body |
| B7BD50..B7BD5F | 16 | `remove_native_ambient_scene_00b7bd50` | Complete valid-array wrapper |
| B7BF90..B7BFC9 | 58 | `append_native_ambient_scene_00b7bf90` | Complete normal body |

All three take native ECX=actual owner and one stacked identity word, RET4.
Remove returns its low-byte result; upper EAX is outside the new source ABI.
Last instructions are B82626/B7BD5D/B7BFC7, each three bytes.

| Body | Site | Native callee | Source binding |
| --- | --- | --- | --- |
| B825D0 | B825DB | B7BD50 | Raw remove wrapper |
| B825D0 | B825F7 | current IAT CE221C | Borrowed current increment import |
| B825D0 | B82605 | current IAT CE2220 | Borrowed current decrement import |
| B825D0 | B82615 | captured old, current virtual0 | Same canonical actual-count registry |
| B825D0 | B82620 | B7BF90 | Raw append wrapper |
| B7BD50 | B7BD58 | B7B620 | Existing borrowed-key erase leaf |
| B7BF90 | B7BFAB | B7B390 | Existing borrowed-key reserve leaf |

The initial current resource+10 is removed before the setter captures either
the current requested argument or current old+10. The requested pointer is
then captured before old+10. If they differ, publish requested first, increment
captured requested through current CE221C, then decrement captured old through
a fresh current CE2220 load. Only a zero return resolves old in the same
canonical registry and invokes its genuine current terminal binding. Callback
replacement owners require their own genuine canonical binding. No lookup or
synthetic decrement occurs for the nonzero case.

After those callbacks the setter rereads current resource+10 for append.
Same-pointer input still removes and appends, but performs no reference-count
operation. Null input releases old as reached and skips final append if +10
remains null. The existing actual ambient companion supplies BD30E0/current4
B7C7E0; arbitrary profiles do not gain a default callback here.

NativeSceneResourceAmbientFrame owns one volatile scene-argument word: native
remove and append reuse the same pushed stack slot. The setter writes the raw
resource identity immediately before each reached wrapper. A reserve callback
may replace that word before append reads it. The requested ambient argument
is a separate caller-owned volatile cell. Context, imports, registry and frame
remain stable; requested and scene cells are distinct, and context/frame do
not overlap native owner/array storage. Original private-stack ABI is separate.

The genuine12B SystemAmbientBacklinks layout is pointer/count/capacity with
four-byte opaque pointer keys. Remove supports nonnegative count<=capacity,
accessible nonoverflowing backing disjoint from its descriptor, and no
asynchronous mutation. In this domain native begin<end equals count>0. Empty
input does not read the key. The wrapper snapshots the opaque pointer word only
for nonempty input, then reuses B7B620. No callback/write occurs between the
native key-read point and this leaf; no SceneResource object is dereferenced.
Its first match swaps in captured last key if needed and decrements count.

Append captures capacity before testing current count equality. Equality grows
using modulo32 doubling, interpreted signed, with values<=1 replaced by1.
B7B390 captures that minimum before allocation. Its existing genuine provider
then reads current count/begin for copies, captures current old begin for free,
and publishes captured new begin/capacity after free returns. The omitted
Ghidra fallthrough B7B3E1..B7B3E9 was inspected; the provider already contains it.
Growth requires valid nonnegative storage/count/capacity and a successful
disjoint CRT allocation. Native new's exceptional ABI is not reconstructed.

After reserve, append rereads count then begin and computes destination modulo32.
Only a nonzero destination reads the current scene argument and writes it.
Then it increments current count modulo32, even if the destination was zero.
No scene key retain, dereference or destruction is performed. Existing array
providers retain their valid-domain and asynchronous-observation boundaries;
no library container implementation was added or newly claimed.

There is no native setter unwind/rollback handler. If reserve allocation fails
after publication, the published pointer and preceding imported count changes
remain. Missing import/identity bindings are explicit source errors at the
reached operation; they do not cause a fabricated success or rollback.

Strict MSVC Win32 build and both existing CTests passed. All163 owned bytes,
plus95B reserve and103B erase evidence, match the installed PE. The four direct
call rows are mechanically checked; the two current imports and current
virtual0 are explicitly indirect. Ghidra was read-only.

One ignored probe compares three original/source pairs: same-pointer,
null-request and a callback path. The callback pair changes current resource+10,
requested argument, and decrement import after the captured requested retain;
old reaches zero through its genuine canonical ambient companion. Final current
+10 append grows through the real allocation provider, whose probe callback
replaces current backing and the scene argument. Comparisons cover resource3Ch
and final ambient98h bytes with actual pointers normalized, raw key identities,
counts, capacity, registry retirement, and event order. Original wrappers use
the same genuine array leaves. The original eight-byte virtual0 dispatch
sequence is bridged to the same canonical terminal provider; no native private
stack, arbitrary profile, or original unwind equivalence is claimed.

Separate source checks verify append allocation failure preserves publication
and retained count, and a computed-null destination skips a PAGE_NOACCESS
argument while incrementing count. Allocator mutation is an ignored-probe seam
which really allocates/frees storage; production has no new callback API.
Assertions are active, with NDEBUG rejected at compile time. Exact `/MD`,
`/fp:strict`, `/MANIFEST:EMBED` command and logs are retained in the report.

Resource construction/lifetime, its raw28h registry, lighting views, graph
admission, native CRT/FH3/outer ABI and gameplay remain separate work.
