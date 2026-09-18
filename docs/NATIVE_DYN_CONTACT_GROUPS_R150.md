# Native Dyn contact groups (R150)

Six complete normal-return bodies, **1,512 native bytes**, are implemented in
`src/native_dyn_contact_groups.cpp`. The raw manager at `world+448h` now groups
actual collision manifolds and supplies the existing native solver tasks.
`reports/native_dyn_contact_groups_r150.json` records byte, call, fixture,
annotation and publication evidence. Names are descriptive hypotheses.

## Recovered contracts

| Address / inclusive end | Original ABI | Behavior |
| --- | --- | --- |
| 0040F810–0040F86B (108 B) | ESI destination, EBX source; EAX destination, RET | Copy the 0Ch pointer-vector record; allocate exactly source size, copy pointers, capacity becomes size. Empty input zeros all three words. |
| 00C36AC0–00C36B31 (114 B) | ECX manager, RET | Reset dynamic-body +64h and all manifold +D4h 16-bit marks to FFFFh. |
| 00C36B60–00C36BC8 (105 B) | ESI group, stack manifold, RET 4 | Append a pointer; grow capacity as `2*capacity+2`, copy old size, free old data and publish the new data. |
| 00C3F410–00C3F469 (90 B) | ESI manager, RET | Free each active group's pointer array; zero manager size/count, retain outer data/capacity and stale entries. |
| 00C4B550–00C4B60B (188 B) | stack manager, RET 4 | Set flag 10h on both endpoints only when every contact has both endpoints' low flags &3 nonzero; then clear groups. |
| 00C4B610–00C4B9AA (907 B) | stack manager, RET 4 | Clear/reset, allocate a manifold-count scratch stack, form depth-first connected groups, free scratch. |

The copy and append functions expose these concrete pointer-vector instantiations,
not a general standard-library implementation. Original register ABIs are recorded;
the public source functions use explicit C++ arguments.

## Records, ordering and graph rules

The manager is `{world, group_data, size, capacity, group_count}`. A group is
`{manifold_pointer_data, size, capacity}`. Existing world construction establishes
the back-pointer. The lifecycle fixture obtains all records from the actual process,
world, body and collision producers.

Reset walks `world+204h`, sentinel `world+208h`, next `body+84h`: **only the dynamic
body list**. It then walks the manifold pool from `world+444h -> scene+B0h`, head
`pool+ECh`, sentinel `pool+F0h`, next `manifold+DCh`. Scratch capacity is pool+1D0h.

Seeds require nonzero point count at +C8h, an unvisited mark, and at least one
endpoint's low flags &3 equal to zero. Traversal visits a body's contact vector
only if flag 1 is clear and its 16-bit mark is FFFFh. Thus a shared static body
does not merge otherwise independent islands. Null contacts and zero-point
contacts are skipped. Newly discovered contacts are marked when pushed; both
endpoints' full flag words are ANDed with FFFFFFEDh, waking connected sleepers.
Endpoint A is traversed before B; the scratch stack gives last-in-first-out order.

Outer capacity grows exactly to the requested group count. Existing inner arrays
are deep-copied by 0040F810 and freed one at a time before the outer array is freed
and replaced. Allocation call-site IDs and reloads across callbacks are retained.
The native resize shrink branch is preserved; ordinary fresh group formation
cannot reach it because clear resets size/count before monotonically adding groups.
Sleep scans a signed contact count and reloads the current entry during flag writes.

Six returning-free fall-through gaps (22 B total) were repaired under the Ghidra
write lock: C36BAA–C36BAC, C3F444–C3F446, C4B78C–C4B78E, C4B7B0–C4B7B2,
C4B81B–C4B821, C4B997–C4B999. Padding after unconditional jumps was left alone.
The callee's no-return flag was not changed. Saved comments/names and refreshed
exports preserve prior annotation values.

## Validation

- All 1,512 bytes match the installed PE and live Ghidra program; six body ranges
  and every direct CALL row are checked. The original installation is unchanged.
- Strict MSVC Win32 build and all three existing CTests pass.
- One focused local differential fixture covers eight topologies: empty, one
  contact, eight disconnected islands, a star, a static bridge, a waking chain,
  cycles with null/empty/static contacts, and entirely sleeping contacts.
  It compares **3,292,340 normalized bytes and 412 allocation/free events per side**,
  including state at every allocator boundary and eleven checkpoints per case.
  Clear/recreate, sleep/clear, outer reuse, deep copies, inner growth and empty copy
  are observed. Logical frees retain backing bytes for comparison.
- The composed lifecycle uses actual process-owned world/body/shape/scene/SAP
  records and a real convex contact. This packet replaces the prior manually
  supplied group with create-groups and adds sleep/clear. Both solver modes run
  on the existing worker: three total batches, nine worker allocations. Full
  physics teardown, pool trim and actual process atexit leave zero tracked
  allocations. The fixture explicitly closes 101 handles left by native cleanup.
- A serialized one-frame application run created its window/device and exited 0.
  It presented zero frames, skipped one Present, and reported 45 unimplemented
  host methods. It proves startup only; this application path does not yet admit
  the reconstructed world-substep pipeline.

## Limits and next dependency

The reference copies omit original FH3 handlers and FS writes while preserving
normal instruction flow. Valid records, successful allocations and no concurrent
mutation are required. Native unwind/OOM behavior, binary ABI compatibility,
adversarial callback mutation and the resize shrink branch lack runtime proof.
The fixture still supplies cube geometry, transfers an actual SAP pair into the
scene vector, supplies derived inverse inertia and explicitly sets the dynamic
body's sleep-eligibility bit. These are not claims for missing integration stages.

Next: recover native velocity integration (00C41550) and collision-pass orchestration
(00C57070), then compose world substep 00C5BB30 around actual task/profile owners.
Raw game-constructor admission, full world stepping and gameplay remain open.
