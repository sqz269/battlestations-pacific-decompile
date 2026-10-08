# Type5 storage and recursive lifetime domains — CC12

This Source-only audit finds a conditional owned-string connection. Successful Type5 construction stores a new current-canonical string copy at `+0x1C` for nonnull borrowed input, or null for null input. Tag5 release frees a nonnull child before clearing its pointer, then clears the inline scalar word and common record fields. Scalar flags separately govern root disposal. Byte `+0x2C=1` is written for either input case, ignored by release and left unchanged; it supplies no ownership or liveness rule.

The actual Source-derived entry is **`008E F2B0`**, whole `[008EF2B0,008EF2EF)`, 63 bytes/19 instructions. This audit adds Source/reconstruction credit 0. Type5's producer validation and the standalone duplicate's Source0 hold remain separate Root decisions; independently qualified current Type2/reference consumers retain their existing domains. No active `local/t7p2`, sealed fixture, old text reader/helper/stage, Native, Ghidra, compiler or installed provider was queried or executed.

## Constructor fields and string ownership

[The current literal constructor](../src/native_scene_property_record_type5_storage.cpp#L14) takes fresh/unowned root56 in ECX, unused EDX and three physical stack DWORDs: borrowed nullable NUL text, `word_18_bits`, `word_08_bits`, followed by `RET 12` and full EAX root. The third word is loaded before PUSH ESI; the second/text slots are read at their actual post-PUSH offsets. Before the duplicate call it stores phase identity `CE89D4`, tag5 and the exact opaque words at `+0x08/+0x18`. The sole CALL binds `duplicate_native_string_00438e40`; its full EAX is stored at `+0x1C`. Then `+0x20/+0x24/+0x34` are zeroed and marker byte 1 is written.

It writes 33 bytes and preserves 23: `[0C,18)`, `[28,2C)` and `[2D,34)`, including owner-candidate DWORD `+0x30`. The caller retains root allocation ownership. Text through the first NUL, root and frame must be disjoint/nonwrapping and stable; current providers require DF0. A failed duplicate can leave partial writes. This connection concerns successful complete construction only, without reentry, failure/naked-frame unwind, class or numeric interpretation. See [the declaration](../include/bsp/native_scene_property_record_type5_storage.hpp#L17).

The actual [raw duplicate implementation](../src/native_string_duplicate.cpp#L25) uses ECX text/no stack arguments/plain RET0. Null returns null without providers. Nonnull scans through NUL, obtains exactly length-plus-one bytes from the real current size adapter/canonical allocator, and copies that many bytes with `memcpy` (lines 35–59). Thus empty but nonnull text produces an owned one-byte copy. No copy flag selects borrowing: the input pointer remains borrowed and is never retained as the child. Later record release has no dependency on the original input span.

The child must remain live, exclusive, undisposed and an actual matching current allocation base. A borrowed input, canonical permanent empty authority, pooled-key buffer/interior, shared allocation or previously freed copy cannot be substituted into `+0x1C`. The raw duplicate's [contract](../include/bsp/native_string_duplicate.hpp#L9) requires one current-free disposition and explicitly keeps the older plain-CDECL API separate.

## Tag5 disposition and root flags

The six lifetime definitions are in [native_scene_property_bag_storage.cpp](../src/native_scene_property_bag_storage.cpp#L218), declared by [native_scene_property_recursive_lifetime.hpp](../include/bsp/native_scene_property_recursive_lifetime.hpp#L34). `release_native_scene_property_record_value_008f0640` performs the complete tag5 arm in lines 273–281:

1. Read the child DWORD at `+0x1C`.
2. If nonnull, call `singleton_lifetime_free(child)`.
3. After that call returns, clear `+0x1C` inside the same nonnull branch. A null pointer was already zero and takes no free/clear call path.
4. Always clear `+0x18`, then leave the switch.
5. Apply common ordered DWORD resets `+0x04=0`, `+0x0C=0`, `+0x28=0`, `+0x08=0` (lines 299–302).

The provider is [the actual `std::free` wrapper](../src/singleton_lifetime.cpp#L65). No retired string bytes are read. The marker `+0x2C` and preserved owner-candidate field `+0x30` are neither read nor cleared. Constructor-zeroed `+0x20/+0x24/+0x34` remain zero; constructor-preserved `+0x10/+0x14` remain untouched. The array-block helper is selected by tags 8–11, not tag5; its interior-array contract supplies no alternate Type5 disposal rule.

[Record destruction](../src/native_scene_property_bag_storage.cpp#L254) first publishes phase identity then invokes value release. [Scalar record deletion](../src/native_scene_property_bag_storage.cpp#L245) always destroys the record; low flags bit zero therefore still frees its owned string and discards both opaque words, while retaining the root in its existing domain. Low bit one additionally frees the actual exclusive current-owned root allocation base. It never frees the `+0x18/+0x1C` interior payload as a root. Returned root bits were captured before disposal and authorize no retired memory read.

| State or operation | Compatibility with tag5 release |
| --- | --- |
| Successful null-text construction | No child disposition; scalar/common resets still occur. |
| Successful nonnull copied child, still live/exclusive/undisposed | Structurally compatible under the existing record/context/current-heap contract, with one child disposition. |
| Borrowed string placed directly in `+0x1C` | Incompatible with owning release; any nonnull pointer is freed. Marker 1 cannot protect it. |
| Child already explicitly freed during raw storage cleanup | Incompatible unchanged: the stale pointer would be freed again. This audit adds no repair transition. |
| Scalar flags0 | Retains root, while disposing the child and scalar words. |
| Scalar flags1 | Requires the actual owned current allocation base and exactly one root disposition after child release. |

Raw fixture cleanup (explicit child-before-root free) and recursive release are alternative ownership schedules. Applying both unchanged is invalid. Source structural compatibility does not grant producer/class admission or permit fabricated record/root storage.

## Current consumers and genuine context

The Type2 constructor, raw copy55 and replace48 Source each call the same physical raw duplicate symbol. Their accepted reports currently grant their own Source1 qualified successful current-provider operations. This does **not** promote standalone duplicate57 from its historical complete-helper Source0 hold, and that hold does not revoke these independently compiled/gated consumer domains. No old family was replayed to make this distinction.

[Copy55's header](../include/bsp/native_reference_payload_copy.hpp#L13) and [replace48's header](../include/bsp/native_reference_payload_replace.hpp#L14) describe an actual eight-byte scalar/string payload. That physical shape corresponds to Type5 `+0x18/+0x1C`, while neither operation touches record `+0x08` or establishes a class. Their required old-copy ownership, disjoint borrowed text/payload/old storage and no use-after-free conditions remain essential. The compatible shape is not evidence that a whole Type5 clone/class caller currently binds them. Type2 owns its string at a different record field; it is separate qualification evidence for the same current duplicate provider.

The ordinary lifetime interface requires its applicable genuine context. Its constructor captures `actual_strings.raw_context()` and the physical process's property pool getter (bag Source lines 212–216). [The context declaration](../include/bsp/native_scene_property_recursive_lifetime.hpp#L16) accepts canonical processes with private constructors, rather than a raw pool or callback. Explicit `CC8A30` startup must return before capture; the same `E175B0` pool and `E188B4` allocator-list generation must be used. [The physical process declaration](../include/bsp/game_native_physical_pool.hpp#L33) states that a nonzero registration status still leaves initialized storage without its CRT callback; it is not permission to fabricate or skip ownership/teardown obligations. All key/record/map payload lifetimes must finish before page teardown, with canonical process/list resources outliving borrowers.

The actual [raw string context](../include/bsp/native_string.hpp#L34) borrows real pool-publication, return-gate and manager cells. A semantic `PooledStringStorage` or convenient buffer is not a substitute for that layout/domain. Whole map clear uses genuine unique 20-byte property-pool nodes with real key headers, saved successors, owned current record roots and real key destruction/slot return (bag Source lines 220–240). Its finite unshared acyclic graph/no-alias/no-concurrent-trim/reentry contract remains required (lifetime header lines 34–49). Type5's partial constructor, preserved `+0x30` or marker 1 does not establish this publication, an owning parent or a class lifetime.

## Clone and publication remain dependencies

[The Reference copy specification](../src/scene_property_bag_merge.cpp#L71) describes payload/kind fields and names the historical constructor. It does not call it. [Semantic assignment](../src/scene_property_bag_merge.cpp#L190) copies C++ text and retains its destination kind; [the model clone](../src/scene_property_bag.cpp#L363) copies semantic values in a different representation from native56 storage. Neither reconstructs the raw whole clone producer.

The merge Source calls `host.clone_record` and `host.insert_record` (lines 274/276); [both declarations](../include/bsp/scene_property_bag_merge.hpp#L162) remain virtual host dependencies. Bounded searches find the Type5 constructor only at its definition/declaration, and no concrete merge-host clone binding. Another `insert_record` call in `scene_record_map.cpp` is a named, unexpanded interface frontier; its spelling is not proof of this raw record publication. The admitted empty-bag clone declaration accepts only count0/all heads empty. No connected Type5-containing raw clone, owner stamping or publication is established by this audit.

Next prerequisites are independent Type5 producer validation, an evidenced complete raw clone/caller/provider binding with the genuine source words and stable text, actual owner/ordinal behavior and real record/key/node publication. Integration execution, native class/destructor, original private-CRT/EH, World/game behavior and parsing/lookup remain separate evidence domains.

## Frozen evidence and recovery

Worktree baseline: `ba56c06f1b13186fb0b9c4d3e4d1be8a7b981604`; current Source revalidated at Main `86da7c2800ad6bd959031c09ae86a5db7f81a387`. The justified strict set is 25 Source/header files: base13 domain inputs, duplicate CPP/HPP, three current consumer pairs and four canonical/property-key declarations. Frozen metadata is generation history, not a forever Main pin.

The initial snapshot stopped when Main appended two permanent empty-string authorities to `game_native_string_process.hpp`; its helper/log/22 partial copies remain unchanged. Only that header differs from the baseline; established raw-context/private canonical fields are unchanged. The next selector failed on a braced `case 5:` label. Its helper/log/41 completed input copies remain unchanged. A new post-only helper selected the actual brace-bearing case and revalidated current Main Source without replaying either failed helper, an old family, compiler or Native process. No new empty-authority qualification is claimed.

| Input | SHA-256 |
| --- | --- |
| Type5 CPP | `bf0267a1f2362148b39599a3b9c3ae9ddefd01b6d26c6da0ede9e410f91407ce` |
| Type5 HPP | `e013cde2dbb55bed44deba372d6fd7ed7162c5f770d0ef561a8dee0a047677e4` |
| Bag/lifetime CPP | `5a0ee52ea81c2eed2cff928bd7d1b4eaa93e6b46e87fad54f114fc7825678e1f` |
| Lifetime HPP | `1cffe704d444d8511d8a0e448f2c05b8e11e9c1b3040e5f8bdf3406630fae6e1` |
| Raw duplicate CPP | `48c86a1ad7763861d099d909a7a8b314b3258ee9b5508f702f093a64188946e0` |
| Raw duplicate HPP | `b101c630eba5844d1f5d21530aef1200c0969b0139fb72de1d3870e75814f477` |

[The machine-readable audit](../reports/cc12_type5_recursive_lifetime_domain_audit.json) contains all pins and exact Source references. Frozen context: `local/t5domain/context_snapshot.json`, SHA-256 `d441f25a98696f55262b557df092ac1af38027c31f3a982a047caa2af72d7916`. Final handoff supplies receipt/manifest hashes and exact inventory count. No accepted receipt or old association was repinned; no new Source/reconstruction credit is granted.
