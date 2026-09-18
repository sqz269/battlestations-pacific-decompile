# Native Dyn collision pass (R152)

The complete normal collision pass now uses the existing raw scene, SAP,
manifold pool, profiler, scheduler and callback records. This removes the manual
SAP-pair transfer and manual narrow-phase batch setup from the actual process
lifecycle fixture. The older semantic host sequence remains a separate interface.

## Recovered bodies

| Entry | Bytes | Original contract | Source function |
| --- | ---: | --- | --- |
| `00C57070` | 2061 | stack scene, `RET 4` | `run_native_dyn_collision_pass_00c57070` |
| `00C549D0` | 196 | stack pool, `RET 4` | `update_native_dyn_manifolds_00c549d0` |
| `00C4B9B0` | 669 | EDX manifold, `RET`; preserves EDX | `refresh_native_dyn_manifold_00c4b9b0` |
| `00C35480` | 361 | stack scene, `RET 4` | `dispatch_native_dyn_contact_events_00c35480` |

These are 3,287 new body bytes. The evidence also includes the existing 28-byte
float-abs entry `00401170`, 172-byte body-reference removal `00C37D30`, constants,
strings and global cells: 3,588 live/PE bytes in 16 spans. Names describe observed
behavior; they are not recovered original symbols.

Source: `include/bsp/native_dyn_collision_pass.hpp` and
`src/native_dyn_collision_pass.cpp`; report:
`reports/native_dyn_collision_pass_r152.json`.

## Collision orchestration

Six profile caches use the actual `0109E9F8` publication cell: Collide,
BroadPhase, BroadPhaseUpdate, ManifoldUpdate, IntersectLoop and GetManifold.
Cold caches call the existing profile-node append body. Timestamp defaults use
actual RDTSC; deterministic timestamps are a fixture service. Original 64-bit
carry, elapsed sum/count updates and current-parent restoration are preserved.

The dynamic-body list is `world+204h`, sentinel `world+208h`, next `body+84h`.
Only flag-8 bodies without flag 10h refresh proxy bounds. The complete x87 AABB
schedule uses the native double 0.5 and existing float-abs boundary. Broadphase
vslots 3/4/5/6 update, count, return first, and advance pairs. Manifold update
receives **the pointer stored at `scene+B0h`**, correcting older contract prose.

The pair vector `B4h/B8h/BCh` resizes to the total broadphase count. Filtering
both-sleeping endpoints fills only its prefix; its stored size stays the total
count. A zero broadphase count leaves the old vector size/storage intact.
Tasks use the filtered count, positive task capacity, inclusive ranges and a
remainder in the last task. The actual scheduler is obtained from engine cell
`0109E9FC`, engine `+10h`. Event count `DCh` is reset before processing and event
dispatch occurs even when there are no pairs.

The returning-free continuation at `00C576B1..00C576B3` was restored under the
Ghidra write lock. Prior state was saved; the callee no-return annotation and
padding after the unconditional jump were left intact.

## Contacts and callbacks

Refresh transforms both local endpoints, recomputes separation, and retains
the original SSE/x87 comparisons against `-0.02f` and approximately `0.0025f`.
Removal copies the last complete 48-byte point record and revisits that slot.
Equality and unordered branches follow the native instructions.

Pool update first skips refresh for two sleeping endpoints. Its second pass
captures the next record, removes an empty manifold from both body vectors,
unlinks it, pushes it onto the pool free list and decrements the active count.
Unchanged fields retain their native stale values.

Each callback receives a 72-byte stack descriptor: two shape pointers, four
12-byte point slots, three normal floats, and signed point count. **Transformed
points are callback-visible**, correcting the earlier interpretation that they
were unused stack temporaries. Only the populated slots are initialized.

Native normal selection uses `eventIndex * 30h + manifold + 8`, an unusual rule
preserved exactly. This is not a claim of safety for arbitrary event indices.
The second callback reloads the other body/listener and swaps the shape words;
it does not negate the normal or recompute points. A first callback can therefore
affect the second through descriptor mutation or listener replacement.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass.
- Independent COFF audit checks 982 native instructions, 48 branches, 18
  adapters, constants and strings, including the existing float-abs kernel.
- 192 copied-native/source pairs compare 12,094,176 bytes across 16 record
  configurations and 12 x87/MXCSR control combinations. The traces include raw
  records, initialized callback descriptor bytes, 2,112 callbacks, 1,536
  controlled batches and 6,144 allocation/free events across both sides.
- Those comparisons use controlled broadphase/batch boundaries, exact-capacity
  growth/reuse, total versus filtered counts, one to three task slots, cold/warm
  profile caches, nested parents, timestamp carry, contact pruning/recycling,
  shape swapping and callback mutation. Heap-backed profile strings are the
  only pointer normalization; their contents are compared.
- A separate actual process lifecycle uses cube geometry, real SAP, native
  narrow phase, one real worker, profiler and two listeners. Three full passes
  create and retain a contact, then recycle it when the dynamic body separates.
  Four callbacks and four actual worker batches complete, with nine worker
  allocations. Actual integration, group creation, both solvers, group cleanup
  and native physics destruction run; 101 native-unclosed handles are closed
  by the fixture. Actual pool trim and process atexit leave zero tracked allocations.
- The source-build application launch was refused immediately because another
  harness held the game slot. After integration the slot became available:
  the combined executable completed a one-frame launch with exit 0, a window
  and D3D device, zero presented frames, one skipped present and 45 unimplemented
  host methods. This checks startup only; raw physics admission and gameplay
  remain unproved.

These are explicit source interfaces with private context adapters, not certified
original ABI replacements. FH3 registration/unwind, allocation failure, hardware
faults, concurrent scene mutation, exhaustive IEEE behavior, raw full world-step
admission and gameplay remain open. Ghidra evidence and artifact seals are recorded
in the report, with source and combined-build results kept distinct.
