# Type9 storage and recursive lifetime domains — CC12

This Source-only audit finds a conditional ownership connection, not a new implementation or admission. A successful copied Type9 record can satisfy the existing recursive release contract while its allocated child remains live, exclusive and undisposed. A retained-pointer Type9 record cannot enter that release while the pointer remains caller-owned. The constructor writes byte `+0x2C = 1` in both branches; that marker cannot select between these ownership cases.

The worktree baseline is `882064808e466755abd99639a3c1d0fe3577a10f`. The frozen current Main context is `0129dddbf4376c7e7b7e656e07e91bc15b41f089`, after Root's Type9 publication `e02d9a050b194c2eadce9a3b884a9dd7e2ca53ad`. Current Source and the thirteen selected headers/implementation files matched the worktree at generation. Baseline and current report/doc copies are historical context; mutable Main metadata is not a forever pin. No Type9 family artifacts, including `local/t9p1`, were consumed.

## Constructor contract

The whole raw constructor `008E F360` takes the actual fresh/unowned 56-byte root in ECX, an unused EDX formal and three actual stack DWORDs: count, input pointer and flag bits. It returns the root in EAX with `RET 12`. The admitted successful domain requires positive count through `0x3FFFFFFF`, stable readable `4 * count` input, disjoint nonwrapping root/input/frame, and DF0 across current providers. The four-byte groups are opaque bits.

[The literal Source](../src/native_scene_property_record_type9_four_byte_array_storage.cpp#L28) tests only the low flag byte. Zero stores the exact input pointer at `+0x20`; no allocation or ownership transfer occurs. Nonzero obtains a new current-canonical child, stores its pointer at `+0x20`, and copies exactly `4 * count` bytes. Both branches store the byte count at `+0x24`, zero `+0x18`, `+0x1C` and `+0x34`, and write `+0x2C = 1`. See lines 38–67 and [the declaration's ownership and preservation contract](../include/bsp/native_scene_property_record_type9_four_byte_array_storage.hpp#L24). The constructor preserves the 27 other bytes, including the DWORD at `+0x30`; it does not establish an owner or class lifetime from those bytes.

## What the recursive consumer actually does

The requested `src/native_scene_property_recursive_lifetime.cpp` does not exist. The six functions are implemented in [native_scene_property_bag_storage.cpp](../src/native_scene_property_bag_storage.cpp#L218), with declarations in [native_scene_property_recursive_lifetime.hpp](../include/bsp/native_scene_property_recursive_lifetime.hpp#L34). The corrected path and initial failed path reads are recorded in the ignored audit family.

Record destruction `008F 0DE0` publishes literal phase bits `CE89D4` then calls value release `008F 0640`. Its tag9 arm passes the actual interior header at record `+0x20` to the physical array-block release `008F 03F0` (bag Source lines 254–258 and 290–294). [That whole helper](../src/native_scene_property_array_block_release.cpp#L14) frees every nonnull first DWORD through `singleton_lifetime_free`, then clears pointer and byte count. It does not read the constructor flag, `+0x2C`, the allocation size or an owner field. The provider is [the actual `std::free` wrapper](../src/singleton_lifetime.cpp#L65), not a callback or replacement allocator.

After that release, the record consumer resets tag `+0x04`, value `+0x0C`, ordinal `+0x28` and field `+0x08` in order (bag Source lines 299–302). Byte `+0x2C` remains 1. It is neither an ownership selector nor a reliable indication that the child remains live.

| Constructor output | Entry into the current recursive release |
| --- | --- |
| Low flag byte zero; `+0x20` is still caller-owned input | Incompatible unchanged. Release would free the borrowed pointer. A current-malloc address alone does not transfer ownership. |
| Low flag byte nonzero; `+0x20` is the new copied child | Conditionally compatible: successful complete construction, a live/exclusive/undisposed child from the same current-canonical heap, a genuine root in the existing record domain, and the applicable recursive lifetime context/ownership contract. |
| Copied child already explicitly freed during raw storage cleanup | Incompatible until a separately justified state transition removes the stale pointer. Calling recursive destruction unchanged would attempt a second free. This audit introduces no such transition. |

The Type9 fixture's explicit child-before-root cleanup and recursive release are alternative disposition schedules. A child must have one owner and one disposition. For borrowed storage, raw caller-root cleanup can leave the input under its caller's management; it is not permission to call the recursive destructor.

The scalar record operation `004E 6730` always destroys the record first. Only the low bit of its flags controls disposal of the actual root through current free (bag Source lines 245–250). Flags zero therefore does **not** protect a borrowed Type9 child. Flags one requires the actual current allocation base, never the `+0x20` interior header. The returned root bits are captured before free and give no permission to read the retired allocation. Flags zero retains the root within the existing qualified root domain; it does not admit arbitrary fabricated storage.

## Owning bag and clone boundaries

A bag clear is a stronger contract than releasing a standalone copied record. The actual map is the borrowed interior at bag `+0x04`; it is not separately freed. The whole ordinary clear visits all 64 heads, saves each successor before disposal, destroys each record with scalar flags one, destroys its real pooled key, and returns its real node slot to the same initialized `00E1 75B0` pool (bag Source lines 220–240). The declared lifecycle domain requires genuine producers and publication, unique live 20-byte pool slots with their real key headers, actual current record roots, finite unshared acyclic ownership, and the canonical string/pool/gate resources outliving all borrowers. See lifetime header lines 16–20 and 34–49. A raw Type9 constructor fixture does not supply that node publication, owner provenance or class contract.

Current clone-related code does not establish a connected raw Type9 clone operation:

- [The semantic model declaration](../include/bsp/scene_property_bag.hpp#L82) explicitly uses a C++ representation rather than the native 56-byte record. [Its clone](../src/scene_property_bag.cpp#L363) copies `ScenePropertyValue` and recursively follows model indices. Its vector semantics cannot be substituted for opaque native payload ownership.
- [The Type9 copy specification](../src/scene_property_bag_merge.cpp#L82) classifies a float-array constructor arm and `owned_block`; it is not a call to the raw constructor or a runtime ownership handoff. The historical semantic name supplies no float interpretation for this raw storage audit.
- The merge walk calls `host.clone_record(source_record)` ([merge Source line 274](../src/scene_property_bag_merge.cpp#L269)); [the host declaration](../include/bsp/scene_property_bag_merge.hpp#L162) leaves this as a virtual interface dependency. That declaration is not a concrete raw Type9 clone binding.
- [The admitted empty-bag clone fragment declaration](../include/bsp/native_scene_property_bag_storage.hpp#L28) requires zero count and all heads empty. It cannot clone a graph containing a Type9 record.

Using the copied constructor as a future clone primitive would additionally require an evidenced complete raw clone caller, valid count recovery from the stored byte count, stable input through allocation/copy, fresh output storage, ordinal/owner behavior and genuine publication. An explicit exclusive ownership handoff of a retained current allocation would likewise need its own producer/caller evidence; the retained constructor supplies no handoff. These are named unresolved connections, not reasons to revoke independently qualified Source domains.

## Evidence and limits

The current frozen Type9 report records Root Source admission 1 for successful current-provider raw storage, with both ownership branches qualified independently. The recursive lifetime report separately records six accepted whole ordinary Source bodies, Win32/all three existing checks and static production review; its primary review records no new entry execution. This audit neither replays those families nor adds an integration runtime claim. It has Source credit 0, no new tests and no Native, compiler, provider, game or Ghidra execution.

The current raw constructor qualification does not establish original private-CRT/EH, virtual/class/destructor ownership or game behavior. Existing independent lifecycle qualifications keep their own stated domains. The ordinary context-taking lifetime APIs do not acquire original register/FS-SEH ABI compatibility from the raw constructor's fixture.

| Pinned input | SHA-256 |
| --- | --- |
| Type9 CPP | `16fc191de1e1cbd856477ab9394572be4da589b657c7daeb09b4ea01532c293c` |
| Type9 HPP | `ba6731df386adb814165454ed1840344c97777764afbc2dbc35824df680143f8` |
| Bag/lifetime CPP | `5a0ee52ea81c2eed2cff928bd7d1b4eaa93e6b46e87fad54f114fc7825678e1f` |
| Lifetime HPP | `1cffe704d444d8511d8a0e448f2c05b8e11e9c1b3040e5f8bdf3406630fae6e1` |
| Array-block release CPP | `9f65c8fe0a18183c2212fba612205da341e2b39cf9c4fe917109a6dc979b340e` |
| Array-block release HPP | `33487664e85d48efc22745d5aa2a4340b1636e00f96ed3f70441003eba9d8642` |

The [machine-readable audit](../reports/cc12_type9_recursive_lifetime_domain_audit.json) carries all thirteen Source/header pins, exact line predicates and unresolved connections. Frozen context: `local/t9domain/context_snapshot.json`, SHA-256 `7fdd6f1326463a5fb366e03f5c43b4ac43b6a9963afedd531f893a7dc5d3fe47`. The local receipt and recursive artifact manifest bind the actual files, failures and frozen metadata; their final hashes are provided in the handoff. No original association or accepted receipt is repinned.
