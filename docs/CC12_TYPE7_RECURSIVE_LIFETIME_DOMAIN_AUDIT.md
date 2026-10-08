# Type7 storage and recursive lifetime domains — CC12

This Source-only audit establishes the tag7 default release behavior. It clears the first inline payload DWORD at `+0x0C`, while the second and third DWORDs at `+0x10/+0x14` remain untouched. It frees no child and dereferences none of these words. Scalar flags separately govern disposal of the actual root. Byte `+0x2C = 1` is ignored and remains 1 after value release; it supplies no ownership or liveness rule.

Actual Source, header and the selected packet agree on **`008E F270`**, symbol `construct_native_scene_property_record_type7_storage_008ef270`, whole `[008EF270,008EF2AF)` — 63 bytes/19 instructions. The initial task's guessed address was corrected before any address query or lease. The lease covers metadata/local audit outputs only.

Main generation and worktree baseline: `2b766f5188074d1b24f9882c008580cc33935320`. The frozen Type7 report records Source0 and reopened validation pending complete helper gates. This is generation history, not a new result or a forever Main assertion. No active `local/t4p3`, other fixture artifact or accepted helper/stage/reader was consumed or executed. This audit adds Source/reconstruction credit 0 and changes no independent qualification.

## Producer and retained inline storage

[The literal constructor](../src/native_scene_property_record_type7_storage.cpp#L12) takes actual fresh/unowned writable root56 in ECX, unused incoming EDX and one actual stack DWORD pointing to a stable readable 12-byte span. Root, input and active frame must be disjoint. Phase/tag stores occur before the interleaved input loads/output stores. Input DWORDs `+0/+4/+8` are copied exactly to record `+0x0C/+0x10/+0x14`, respectively. The input pointer is not retained, allocated or freed. Its lifetime must cover the successful call; subsequent release has no dependency on that span.

The constructor zeroes DWORDs `+0x18,+0x1C,+0x20,+0x24,+0x30,+0x34`, writes opaque phase identity `CE89D4`, tag7 and marker byte 1. It writes 45 bytes and preserves 11: `[08,0C)`, `[28,2C)` and `[2D,30)`. The caller retains root/input storage ownership. It returns the actual root in EAX with `RET 4`, ECX zero and EDX equal to the second input DWORD. Nonvolatiles and DF are unchanged; XOR's defined flags are CF/OF/SF0, ZF/PF1, AF undefined. See [the header](../include/bsp/native_scene_property_record_type7_storage.hpp#L16). These are opaque word copies, without numeric, SSE, x87 or MXCSR operations, overlap/snapshot semantics or an invalid-input fault contract.

## Exact current release and root rules

The six lifetime definitions are in [native_scene_property_bag_storage.cpp](../src/native_scene_property_bag_storage.cpp#L218), declared by [native_scene_property_recursive_lifetime.hpp](../include/bsp/native_scene_property_recursive_lifetime.hpp#L34). `release_native_scene_property_record_value_008f0640` has explicit cases only for tags 2, 5, 6 and 8–11. Tag7 takes `default: break` (lines 262–297), then ordered DWORD resets `+0x04=0`, `+0x0C=0`, `+0x28=0`, `+0x08=0` (lines 299–302). It reads the tag; it does not read the marker or the inline payload on this path.

| Field | After successful construction | After tag7 value release |
| --- | --- | --- |
| `+0x0C` | Exact first input word | Zero |
| `+0x10` | Exact second input word | Same word, untouched |
| `+0x14` | Exact third input word | Same word, untouched |
| `+0x20/+0x24` | Zero array pointer/byte count | Remain zero; no array release |
| `+0x2C` | Byte 1 | Remains 1, ignored |
| `+0x08/+0x28` | Preserved caller storage bytes | Zero DWORDs without reading old contents |
| `+0x30/+0x34` | Zero DWORDs | Remain zero |

This is partial record reset, not full payload erasure. Pointer-shaped input words remain scalar bits; release neither destroys nor frees any external resource they might identify. The physical [array-block helper](../src/native_scene_property_array_block_release.cpp#L14) owns/frees a nonnull current-heap child at an actual eight-byte header, but tag7 never calls it. Its ownership contract must not be substituted for inline word storage.

[Record destruction](../src/native_scene_property_bag_storage.cpp#L254), `destroy_native_scene_property_record_008f0de0`, publishes phase identity then calls value release. [Scalar deletion](../src/native_scene_property_bag_storage.cpp#L245), `scalar_delete_native_scene_property_record_004e6730`, always destroys first. Low flags bit zero retains the root within the existing root domain; it still clears the first payload word. Low bit one frees the actual owned current-canonical root allocation base afterward through [the actual `std::free` wrapper](../src/singleton_lifetime.cpp#L65). It returns address bits captured before free; no remaining inline word may be read from a retired root.

Structural compatibility therefore requires genuine successful Type7 output, valid actual record storage and tag7, the applicable existing record/context contract, and exactly one root disposition. Root flags1 additionally requires the exclusive current allocation base; an interior header or arbitrary borrowed/fabricated buffer is insufficient. Constructor DF preservation does not qualify DF1 through ordinary C++ lifetime/provider calls; their existing calling/current-provider DF0 domain remains applicable. Type7's independently accepted producer qualification is a separate Root prerequisite, not established by this inspection.

## Owning bag and clone frontiers

A bag clear imposes genuine publication/ownership beyond the scalar storage contract. It visits all64 heads, saves each successor before disposition, destroys records with scalar flags1, destroys real pooled keys and returns genuine nodes to the same initialized property pool (bag Source lines 220–240). The declared lifetime domain requires real owned current record roots, unique live 20-byte property-pool slots/real key headers, finite unshared acyclic graphs, and canonical string/pool/gate resources outliving all borrowers (lifetime header lines 16–20 and 34–49). Marker 1 and the constructor's zero `+0x30` do not establish such publication or an owning class. No fabricated bag, parent or node was used.

[The clone specification](../src/scene_property_bag_merge.cpp#L78) selects a `vector3` flag and historically names `008E F270`; it makes no call to the raw constructor. Its comment saying “three floats” does not establish numeric semantics for this raw leaf. [The semantic model](../include/bsp/scene_property_bag.hpp#L82) uses C++ values, including a typed three-element array, rather than native record layout. [Its model clone](../src/scene_property_bag.cpp#L363) copies those values; [semantic assignment](../src/scene_property_bag_merge.cpp#L197) assigns three typed elements. Neither supplies the native allocation, raw record ABI or owning publication binding.

The raw merge walk calls `host.clone_record` ([merge Source line274](../src/scene_property_bag_merge.cpp#L269)); [the host method](../include/bsp/scene_property_bag_merge.hpp#L162) is pure virtual. A bounded Source/header symbol search finds the Type7 constructor only at its definition/declaration and `clone_record` only at that call/interface. No concrete direct raw clone binding is established by those searches; indirect dispatch and class closure are not proved. [The empty-bag clone declaration](../include/bsp/native_scene_property_bag_storage.hpp#L28) accepts only zero count/all heads empty and cannot clone a Type7-containing graph.

The tracked Type7 doc separately records a historical clone arm `[008F5134,008F5168)`, allocating `38h`, forming `source+0x0C`, calling the constructor at `008F514C` and copying `+0x34` afterward. This is named caller/provenance history, not a current Source implementation; no Native body, provider or old audit family was reopened. A connected raw clone still needs an independently evidenced complete caller/provider binding, stable genuine source payload, fresh output, actual ordinal/owner behavior and publication. Class/destructor or original private-CRT/EH/game admission does not follow from a raw storage fixture.

## Frozen evidence

Thirteen implementation/header files are pinned before/after; four matching tracked docs/reports have baseline and current generation copies. The selected packet is separately frozen with its config-generation hash, without a forever config pin. Existing six-body lifetime Source/build/static qualifications remain unchanged; this audit performs no new runtime or integration validation and invokes no compiler, Native, Ghidra or installed provider.

| Input | SHA-256 |
| --- | --- |
| Type7 CPP | `1c28752683c9015cc8ab757762631ecd9c60995ad47aa3c27afa7f9f005dc94f` |
| Type7 HPP | `315e9a12c16bb4b5076a081f33e7cd299f1b3f00fea82a371c957fb401980c34` |
| Bag/lifetime CPP | `5a0ee52ea81c2eed2cff928bd7d1b4eaa93e6b46e87fad54f114fc7825678e1f` |
| Lifetime HPP | `1cffe704d444d8511d8a0e448f2c05b8e11e9c1b3040e5f8bdf3406630fae6e1` |

All pins and exact Source references are in [the machine-readable audit](../reports/cc12_type7_recursive_lifetime_domain_audit.json). Frozen context: `local/t7domain/context_snapshot.json`, SHA-256 `80e80a3a52a979ad26dcd5eff6b572e6111d6716e3e8ff62905e682b239fbb08`. Final handoff identifies the exact recursive receipt/manifest hashes and inventory count. Mutable Main metadata is historical context; later Root decisions can advance independently.
