# Directional light canonical reference

`DirectionalLightReference` binds `DirectionalLightOwner` to the existing
`GeneratedModelLifetimeRuntime` and render-command reference interface. The
reference counter is the owner's actual `+04` atomic. Transform, hierarchy,
scene dispatch, point-light array and light tail remain the same borrowed native
storage used by the constructor, renderer views and deleting destructor.

The directional table was read from the verified `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, and compared with installed PE table bytes.
The camera companion informed interface integration only. Directional dispatch
was established from its own current table:

| Slot | Directional `00D62FB0` | Light `00D62F58` | Node `00D62C88` |
| --- | --- | --- | --- |
| `+00` | `00BD30E0` | `00BD30E0` | `00BD30E0` |
| `+04` | `00B7C820` | `00B7C800` | `00B6F8D0` |
| `+0C` | `00B7C6D0` | `00B7C580` | `00B6F570` |
| `+18` | `00B6F310` | `00B6F310` | `00B6F310` |
| `+34` | `00B6E870` | `00B6E870` | `00B6E870` |
| `+40` | `00B6DBE0` | `00B6DBE0` | `00B6DBE0` |
| `+50` | `00B7C020` | `00B7C020` | `00B6ED80` |
| `+54` | `00B7BD60` | `00B7BD60` | `00B6EE10` |

The existing source implementations remain authoritative for each body:
`generated_model_lifetime.cpp`, `directional_light_owner.cpp`,
`native_node_destruction.cpp`, `light_scene_retention.cpp`,
`light_type_bootstrap.cpp` and `camera_transform.cpp`. The three supplied
22-word immutable table views must outlive the companion. The adapter reads the
actual owner's current table identity and selected virtual slot on each call.
Only the observed directional-to-light-to-node lifetime phases are supported.

`release_model_virtual18_00b6f310` delegates the whole logical-release body to
the shared runtime with views of actual `+164/+168/+16C` and `+44`. Point-light
backlink removal and child virtual `+18` traversal precede the released-byte
gate, including repeated calls. Array shrink preserves its allocation and
capacity. The constructor's one self reference is released once; queued
references can keep the owner, retained scenes and shadow alive afterward.

Directional/light `+54` uses `00B7BD60` over the actual retained-scene array at
`+178/+17C/+180`. It does not substitute the node's separate `+170` scene slot.
During node-base destruction, the same companion instead dispatches `00B6EE10`.
The owner's existing destruction code changes the actual scene/type callbacks
with each vtable phase. Runtime bindings remain available during scene, shadow
and `+130` terminal callbacks.

On a final reference, `00BD30E0` (`[00BD30E0,00BD30EE)`, ECX owner, no stack
arguments, RET) supplies flag one to current virtual `+04`; it performs no count
decrement or `+44` check. `00B7C820` (`[00B7C820,00B7C846)`, ECX owner, stack
deletion flags, EAX original address, RET4) executes the existing light/node
destruction and returns the same `1F0h` slot to its original pool. Scene binding
removal belongs to that existing destructor. The companion then removes only
its host lifetime association, marks itself retired, and invokes mandatory
companion disposal. No dead slot read or companion access follows disposal.

`allocate_native_directional_light(environment, name)` composes the existing
pool allocation and `00B7C6B0` constructor. Its environment supplies the shared
node/string runtime, original pool, real directional/light predicates, shadow
resolver and actual tables. The predicates must use initialized native type
descriptors; there are no new IDs or acceptance fallbacks. The factory binds
both canonical runtimes without adding a reference. A GUI scene allocation
service returns `{reference->light_owner(), *reference}`. The environment and
all borrowed callbacks must survive delayed queue release.

The factory's host allocation/binding rollback is separate from recovered
native EH. Before publication, the fresh object has no scenes, shadow, child
hierarchy or retained owner; an unbound host-setup failure releases its copied
name and ends the two typed storage lifetimes before returning the pool slot.
Once scene binding succeeds, rollback uses the existing native destructor with
flags zero and then returns the slot. Native constructor failures retain their
existing cleanup. No host failure path represents a recovered native factory.

Validation on 2026-09-11: all eight seed regions matched the installed binary;
MSVC Win32 Release `/W4 /WX /fp:strict` build and both existing CTests passed.
One ignored fixture used actual PE table bytes, real type bootstrap and pool
implementations, two directional owners and one retained queue reference. It
checked the actual count sequence `1/2/1/1/0`, canonical binding identity,
constructor-unwritten tail bytes, child release, point-light backlink/shrink,
retained-scene removal, repeated logical release, light/node-phase callback
reentry, three terminal resource callbacks and same-slot reuse after retirement.
No permanent test was added.

This is a build-tested and focused host-fixture-tested composition of previously
reconstructed bodies. The fixture did not execute original directional machine
code. Descriptive C++ names are hypotheses, the interfaces are not native ABI
replacements, and GUI rendering or game validation is not claimed. Ghidra names,
comments and function definitions were read-only and preserved. Detailed current
evidence, prior metadata and remaining boundaries are in
`reports/directional_light_reference.json`.
