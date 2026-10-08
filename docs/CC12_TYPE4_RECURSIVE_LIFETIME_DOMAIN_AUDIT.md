# Type4 storage and recursive lifetime domains — CC12

This read-only Source audit finds no Type4 child ownership to release. The raw constructor stores two opaque DWORDs; the current value-release switch sends tag4 through its default path, then clears scalar record fields. Neither stored DWORD is dereferenced or freed. Scalar deletion separately controls disposal of the actual root. Byte `+0x2C = 1` supplies no ownership rule.

The frozen Main/worktree baseline is `7b244d3b8bc3204b1e37759a90842c28a0b78b63`. At generation, the tracked Type4 report records Source admission 0 and validation reopened pending complete helper gates. That is historical metadata, not a new validation result or a forever assertion about Main. Root's active `local/t4p3` was not read. This audit adds Source/reconstruction credit 0 and leaves independently qualified lifetime domains unchanged.

## Exact producer and release behavior

[The current constructor Source](../src/native_scene_property_record_type4_storage.cpp#L13), named for `008E F230`, is the literal zero-CALL 56-byte/16-instruction partial initializer. Its physical API takes root ECX, unused EDX, and two actual stack DWORDs, then `RET 8`. It stores the first word exactly at `+0x28` and the second exactly at `+0x0C`. Both words remain opaque; declaration identity is a descriptive hypothesis, not proof of a pointer, registry lookup, refcount or enum resolution.

The constructor writes phase identity `CE89D4`, tag4, zero DWORDs at `+0x18,+0x1C,+0x20,+0x24,+0x30,+0x34`, and byte 1 at `+0x2C`. It writes 41 bytes and preserves 15, including `+0x08..+0x0B`. The caller retains the supplied allocation's ownership. See [the declaration](../include/bsp/native_scene_property_record_type4_storage.hpp#L17), especially lines 17–30. EAX returns the actual root, ECX becomes zero, EDX holds the second word, nonvolatiles and DF are unchanged. This raw leaf does not allocate or free a root or child and does not establish an owning native class.

The six recursive lifetime functions are in [native_scene_property_bag_storage.cpp](../src/native_scene_property_bag_storage.cpp#L218), declared in [native_scene_property_recursive_lifetime.hpp](../include/bsp/native_scene_property_recursive_lifetime.hpp#L34). The exact value-release symbol is `release_native_scene_property_record_value_008f0640`.

Its only explicit cases are tags 2, 5, 6 and 8–11 (bag Source lines 262–294). **Tag4 takes `default: break`**, followed by these stores in order (lines 295–303): DWORD `+0x04=0`, `+0x0C=0`, `+0x28=0`, then `+0x08=0`. There is no Type4 provider call, child disposal, declaration dereference or declaration release. The previously zero array header remains zero. The marker at `+0x2C` is neither read nor cleared, so it remains 1 after value release.

The physical [array-block helper](../src/native_scene_property_array_block_release.cpp#L14) frees any nonnull child at an actual eight-byte array header and clears pointer/byte count. It is called for tags 8–11, **not tag4**. Its ownership requirements cannot be applied to the Type4 scalar bits merely because they occupy fields used by other tags.

## Root and scalar compatibility

| Operation | Type4 effect and ownership requirement |
| --- | --- |
| Raw constructor | Retains both opaque words in the record and leaves allocation ownership with its caller. No ownership transfer follows from pointer-shaped bits, phase bits or marker 1. |
| Value release `008F 0640` | Tag4 discards the stored scalar words with the ordered resets. It never destroys or frees any resource those bits might identify. Structural compatibility follows from the exact producer stores and default release path, within the existing record contract. |
| Record destruction `008F 0DE0` | Publishes phase identity `CE89D4` then performs that value release (bag Source lines 254–258). No class/vtable binding is inferred. |
| Scalar record deletion `004E 6730`, low flag bit 0 | Still destroys/releases the record, but retains its root in the existing root domain. The words are cleared; flags zero does not preserve the scalar value. |
| Scalar record deletion, low flag bit 1 | Destroys first, then passes the actual owned current-canonical allocation base to `singleton_lifetime_free`. No interior header or arbitrary borrowed storage may be substituted. |

[Scalar deletion](../src/native_scene_property_bag_storage.cpp#L245) captures root address bits before disposal; its returned bits are not permission to read retired memory. The actual [free provider](../src/singleton_lifetime.cpp#L65) is `std::free`. There is no scalar-child allocation or child cleanup schedule for Type4. Any declaration/resource represented by a word has a separate lifetime outside these operations.

These Source effects do not themselves admit a Type4 owning record. Connection to the qualified lifecycle domain additionally requires the independently accepted genuine producer, valid actual record storage/root ownership, applicable real lifetime context and exactly one root disposition. Type4's current Source0 producer-validation boundary remains Root's separate decision. Constructor DF preservation also does not admit DF1 execution through ordinary C++ lifetime/provider APIs; follow their existing calling and current-provider DF0 contract.

A record in an owning bag needs further publication evidence. The full map clear at bag Source lines 220–240 destroys each record with scalar flags one, destroys its real pooled key, and returns its genuine node slot to the same initialized property pool. Its declared domain requires real current-owned record roots, real 20-byte slots/pooled-key headers from that pool, finite unshared acyclic ownership, and the canonical string/pool/gate resources outliving their borrowers (lifetime header lines 16–20 and 34–49). The Type4 constructor does not publish a node or establish a parent/owner link; its `+0x30` store is simply zero. No fabricated bag or pool node was used here.

## Clone connections remain bounded

[The copy specification](../src/scene_property_bag_merge.cpp#L67) classifies the Type4 semantic `Enum` arm as inline DWORD plus `shared_decl`. Its historical comment names `008E F230` with source `+0x28/+0x0C`; this is not an actual constructor call or evidence of declaration ownership. The assignment specification separately preserves the destination declaration while copying the scalar value (lines 101–105). These classifications do not reconstruct the full native clone caller.

[The semantic model](../include/bsp/scene_property_bag.hpp#L82) is a C++ representation rather than a native 56-byte record. [Its clone](../src/scene_property_bag.cpp#L363) copies `ScenePropertyValue` containers and follows model subbag indices. Semantic assignment copies `int_value` for matching Enum types (merge Source lines 164–171); it is not a raw native identity or ownership binding.

The merge walk calls `host.clone_record(source_record)` ([line 274](../src/scene_property_bag_merge.cpp#L269)); [the host declaration](../include/bsp/scene_property_bag_merge.hpp#L162) remains pure virtual. A bounded `src`/`include` search finds the Type4 constructor symbol only at its definition/declaration, and `clone_record` only at that call/interface. This establishes no concrete direct raw clone binding in those searches; it is not an indirect-dispatch or class-closure proof. The [empty-bag clone fragment](../include/bsp/native_scene_property_bag_storage.hpp#L28) requires zero count/all heads empty and cannot clone a bag containing Type4 records.

The next raw clone connection requires an evidenced complete caller/provider binding, the two genuine source words, fresh output storage, actual ordinal/owner behavior and publication, and any declaration lifetime contract independently established by its producer/consumer. Historical readiness comments or semantic enum names cannot replace these prerequisites. No Native frontier was queried.

## Evidence

Thirteen current implementation/header files are frozen and pinned before/after. Main metadata copies are generation context only. The audit records one failed guessed Source selector (`StopIteration`) and its bounded recovery to the actual symbol; no prior helper, family, compiler, provider, Ghidra or Native execution occurred. Existing lifetime qualification records six accepted whole ordinary Source bodies and static production/build checks, separately from this audit; no new integration execution is claimed.

| Input | SHA-256 |
| --- | --- |
| Type4 CPP | `b66adbd44744b4d56351aeb50772fef1b6a3000268bc9bf119ea1e3dd760d784` |
| Type4 HPP | `6afc1b32b59b0f18832270c977918b6b2bec63a173f5eaf45e9584c0ddf61d24` |
| Bag/lifetime CPP | `5a0ee52ea81c2eed2cff928bd7d1b4eaa93e6b46e87fad54f114fc7825678e1f` |
| Lifetime HPP | `1cffe704d444d8511d8a0e448f2c05b8e11e9c1b3040e5f8bdf3406630fae6e1` |

The [machine-readable audit](../reports/cc12_type4_recursive_lifetime_domain_audit.json) supplies all pins and exact Source references. Frozen context: `local/t4domain/context_snapshot.json`, SHA-256 `51a13ebf93211ce2d6fbcc2fc56c9d2a3d17e7496e3cdcea51e5b36718429930`. The local receipt and exact recursive manifest are identified by final handoff hashes. No owning class, original private-CRT/EH, binary replacement ABI, gameplay, lookup or parser admission is added; independently qualified domains remain unchanged.
