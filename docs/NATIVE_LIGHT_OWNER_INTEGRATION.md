# Native light storage, node lifetime and fog factory

Subsequent work is recorded in [native light chain integration](NATIVE_LIGHT_CHAIN_INTEGRATION.md):
the directional owner, type bootstrap and lighting configuration now run together
in the installed-asset diagnostic. The results below describe this earlier batch.

This batch integrates the shared directional slot pool, native node construction
and direct destruction, light scene retention, and the bounded world fog factory.
The complete directional-light owner and process startup composition remain
separate work. These modules use new C++ interfaces; native field layouts and
selected byte comparisons do not establish a drop-in binary ABI or game rebuild.

The pool uses one supplied allocator-list head and actual 38h owner storage,
including a real Win32 critical section. Its 1F0h slots retain their allocation
preimages; compaction rewrites every moved slot's authoritative +1EC slab ID.
Allocation preserves native publication and lock state if the allocator throws.
The static adapter requires the actual 01090154 owner and CRT exit registration;
the application has not yet composed that process lifetime. The string pool is
a separate service and is not silently registered in this allocator list.

`NativeNodeStorage` occupies the actual first 174h bytes of a slot. Its external
`CameraTransform` and `SceneNodeAttachment` bind the same fields without copies
or synchronization. A default transform still owns diagnostic storage. Value
construction copies into newly owned fields; assignment preserves its target's
backing and writes into it. `RenderNodeRootList` similarly borrows the actual
outer +0C head and +1C scene-resource slot. These are the same values read by
the existing scene and shader consumers.

Direct B6F440 destruction now handles a nonnull parent and existing roots. It
switches only the dying node's current 0C/40/50/54 dispatch into node phase,
preserves callback-driven field reloads, releases real retained owners and scenes,
and frees the actual point array and pooled name before ending prefix lifetime.
The requested-parent-null path of B6E680 is reconstructed; nonnull reparenting
remains open. Physical pool return and external companion disposal are separate.
`forget_destroyed_binding` removes only the host association by identity, without
reading ended fields or undoing a scene installed by a destruction callback.

Light scene retention uses that same node binding and its separate actual +178
array. It preserves append/register/retain and erase/unregister/release order,
including recursion on duplicates or absent entries and fresh sibling reads.
The actual base-node type predicate remains a required binding while type
bootstrap is reconstructed independently.

The fog factory uses live game slots for the optional D64518 receiver at +19E8,
the same camera at +19FC, and the authored scene record at +5FC. It publishes
both counted references and drops the creator before scalar/color writes, then
reloads the record for each directional copy. The packed-color temporary remains
an output for the unowned continuation. The receiver is distinct from the world
at game+19CC. The existing installed draw probe now executes this factory with
an absent receiver/record and its real diagnostic camera, then applies the same
explicit environment values as before.

Validation is deliberately bounded:

- Strict Win32 build and both existing CTests pass.
- Pool: one original-native comparison matches 136 states, 3,714,020 raw slab
  bytes and eight real allocation/free observations.
- Constructor: all 496 seeded slot bytes match the executed original empty-name
  constructor; nonempty pooled-name ownership is separately checked in C++.
- Direct destructor: four whole slots match 13 executed original bodies after
  pointer-identity normalization. Nonempty ownership/reentry/unwind have focused
  host checks; those native ownership paths were not executed.
- Fog factory: 64 comparisons of the original continuation cover two receiver
  states, two scene states, eight x87 control words and two stack depths. They
  compare overlapping source storage, references, packed output and x87 state.
- Light retention and shared root storage have independent source/assembly
  review and one focused host callback sequence.
- Installed draw: factory packed output and final camera release pass; all 77
  VS/PS constants survive two material uploads; 2,499 visible pixels and 54
  colors match the prior diagnostic image byte for byte. D3D9 state restores.

The original game EXE remains unchanged. The diagnostic image is still very
dark and does not prove visual parity. Complete light/type/shadow construction,
actual startup/exit composition, full native exception ABI and gameplay remain
unvalidated. Per-source, artifact, annotation and export hashes are recorded in
`reports/native_light_owner_integration_audit.json`; component reports preserve
the distinction between native execution, host fixtures and source evidence.
