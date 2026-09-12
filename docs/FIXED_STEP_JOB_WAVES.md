# Fixed-step job waves (the five group heads at `00F876C8` and the four factories)

Addresses: 00f876c8, 004c1130, 008754d0, 008755a0, 00875670, 00875750, 00875890,
00875a80, 008759b0, 00874c90, 00874fe0, 008750a0, 00875160, 00875b90

Every name here is a hypothesis, not a recovered symbol.

`docs/IN_MISSION_SUBSYSTEM_TICK.md` established the shape: the fixed-step driver `00875BB0`
walks five `68h`-stride group heads three times per fixed step and once more per frame, and
none of the four job factories had been read. This packet reads the group container, the
registration path, the four factories and the four job bodies they return.
`include/bsp/fixed_step_job_waves.hpp` and `src/fixed_step_job_waves.cpp` hold the
reconstruction.

## The group array

Five groups of `68h` bytes starting at `00F876C0`. A group is two list sentinels of `34h`
bytes each:

| field | address (group 0) | role | evidence |
| --- | --- | --- | --- |
| `group+0h` | `00F876C0` | head sentinel node | `00874CB8 ADD ECX,0xF876C0` with `ECX = index*68h` |
| `group+8h` | `00F876C8` | head sentinel's `next`, i.e. the first element | `00875CB7 MOV ESI,[EBX-0x2C]` with `EBX = 00F876F4 + i*68h` |
| `group+34h` | `00F876F4` | tail sentinel node, the walk's terminator | `00875CBC CMP ESI,EBX`; `00874CCA LEA ESI,[ECX+0x34]` |
| `group+38h` | `00F876F8` | tail sentinel's `prev`, i.e. the last element | `00874CC4/00874CD0/00874CD6` |

A node is `34h` bytes with `+4h` `prev` and `+8h` `next`, so the head sentinel's `next` at
`group+8h` and the tail sentinel's `prev` at `group+38h` are the two ends of one list. An
empty group has `group+8h == group+34h` and `group+38h == group+0h`; appending sets
`node->prev = [group+38h]`, `node->next = group+34h`, `[group+38h]->next = node` and
`[group+38h] = node`. The same two-sentinel layout is used by the pending list: head
sentinel `00E0B6D0` (its `next` slot is `00E0B6D8`, its unused `prev` at `00E0B6D4` is
zeroed), tail sentinel `00E0B704` (`prev` at `00E0B708`, unused `next` at `00E0B70C`).

The driver never uses the `prev` links; it reads `[group+8h]` and follows `+8h` until it
reaches `group+34h`.

## The element and its payload

`00875890` (`__thiscall node*(node, void* payload, int group)`, body
`00875890..0087591A`) is the element's constructor. It is the producer for every offset the
waves read:

| offset | written | meaning |
| --- | --- | --- |
| `+0h` | `00D0DEC8` | vtable, the base implementation |
| `+4h` | `[00E0B708]` | `prev`, appending to the **pending** list |
| `+8h` | `00E0B704` | `next` |
| `+14h` | argument 3 | the group index `00874C90` multiplies by `68h` |
| `+18h` | `0` | linked flag (`00875A80` uses the same offset on its own list) |
| `+19h` | `0` | |
| `+1Ch`, `+20h`, `+24h` | `0` | head, tail and count of the element's own sub-list |
| `+28h` | argument 2 | the **payload**: the object the waves test and the jobs advance |
| `+2Ch` | `0` | |
| `+30h` | `00D7A260` | a float constant, the same one `00778450` resets its timer from |

The append happens under the critical section from `00875280`; the element joins the
pending list, not a group. `00874C90` (the fan-out's call 11, inside the world gate) splices
every pending element into `group[element+14h]` and resets the pending list. An element
therefore becomes visible to the waves at the **next** fixed step after its construction,
and only while the world is active.

The base vtable at `00D0DEC8` is `{00875920 scalar deleting dtor, 0042BB70, 0042BB80,
0042BB90, 0042BBA0, 0042BBB0}`, six slots; `00D0DEE0` is zero, which is where the table
ends. **Five** slots follow the destructor, not four. Four of them are one instruction:
`0042BB70`, `0042BB80` and `0042BBA0` are `RET 4` (one float argument each) and `0042BB90`
is `RET` (no argument). The fifth, `0042BBB0`, is two: `XOR AL,AL` then `RET` at
`0042BBB2`, so the default answer of the only predicate in the interface is **false**.
Derived classes override them; the default is to do nothing and to say no.

The **payload** is tested by two bytes, `payload+5Eh` and `payload+BDh`. Nothing in this
packet establishes what they mean beyond their use as the admission test; `payload+5Eh` is
the same byte the entity queues of `docs/FIXED_STEP_FANOUT.md` treat as "gone".

Nine sites construct an element. Eight pass group **0** (`00432786`, `004E7FCD`,
`004F2449`, `006E26BE`, `006E7B49`, `0087B6A9`, `00929E85`, `00A31766`; the two that pass
`EBX` are `XOR EBX,EBX` at `00432776` and `004F2439`) and one passes group **3**:
`007F2C9A`, inside the plane-squadron construction path reached from
`BSP_SceneUnit_CreatePlaneSquadronGen 004F0AD0`. Groups 1, 2 and 4 have no constructor
site in this reading; they are walked every step and may stay empty in this build.

## The four waves

Each wave is the same loop over the five groups: count the elements it queues, and if the
count is non-zero, dispatch once for the whole group. The admission test differs only in
the order of the two byte tests, which is a compiler artefact, not a behaviour difference.

| wave | loop | factory | admission | job body |
| --- | --- | --- | --- | --- |
| 1 | `00875CAA..00875D19` | `008755A0` | `payload+5Eh == 0 && payload+BDh != 0` | `008750A0` |
| 2 | `00875D1A..00875D89` | `00875750` | `payload+BDh != 0 && payload+5Eh == 0` | `00875B90` |
| 3 | `00875D8A..00875DF9` | `008754D0` | `payload+5Eh == 0 && payload+BDh != 0` | `00874FE0` |
| interpolation | `00875F55..00875FC0` | `00875670` | `payload+5Eh == 0` only | `00875160` |

Each range is the `MOV EBX,0xF876F4` / `MOV EBP,0xF876F4` group-cursor load through the
five-iteration back edge (`00875D18`, `00875D89`, `00875DF8`, `00875FBF`).

Waves 1-3 run inside the step loop, before the subsystem fan-out; the interpolation wave
runs once per frame after the loop and only when both the frame delta and the leftover
accumulator are positive (`00875F1D..00875F32`).

## The factories and the job descriptors

The four factories are the same double-checked lazy singleton, each over its own global,
each registered with `BSP_SingletonLifetime_Register` under the manager's critical section
at `+10h`, each returning the object's base pointer. The registered pointer and the
returned pointer are **not the same**: `00875636 ADD EAX,0x4` computes the pointer that is
pushed at `0087563D` and handed to the registrar, i.e. the `+4h` subobject whose one-slot
vtable is the deleting-dtor thunk - exactly what a lifetime registrar needs. The factory's
own result is loaded afterwards at `00875659 MOV EAX,[00F878D4]`, the object base, which is
why the descriptor the pool receives has the **job body** in `vtable[0]` and not the thunk.
The constructor also writes the base-class vptr `00D0DE94` into `+4h` at `00875608` before
overwriting it with the derived one at `00875615`:

| factory | body | singleton | `operator new` | vtable `+0h` | vtable `+4h` |
| --- | --- | --- | --- | --- | --- |
| `008754D0` | `008754D0..0087559D` | `00F878D0` | `8` at `0087552A` | `00D0DEAC` -> `00874FE0` | `00D0DEA8` -> `00874FB0` |
| `008755A0` | `008755A0..0087566D` | `00F878D4` | `8` at `008755FA` | `00D0DEB4` -> `008750A0` | `00D0DEB0` -> `00875070` |
| `00875670` | `00875670..0087573D` | `00F878D8` | `0Ch` at `008756CA` | `00D0DEBC` -> `00875160` | `00D0DEB8` -> `00875180` |
| `00875750` | `00875750..0087581D` | `00F878DC` | `8` at `008757AA` | `00D0DEC4` -> `00875B90` | `00D0DEC0` -> `00875210` |

Two single-entry vtables per object: the one at `+0h` holds the job body, the one at `+4h`
holds an adjustor thunk (`SUB ECX,4; JMP <scalar deleting dtor>`). The interpolation
descriptor is the only one allocated with `0Ch` bytes, and that extra word at `+8h` is
exactly where the driver stores the leftover accumulator at `00875F50` before dispatching.
The other three carry no state: a wave's step size is the constant `00D0DE84 = 0.05f` the
job body loads itself.

## The job bodies

All four are `__thiscall void(descriptor, element*)`, `RET 4`.

- **`008750A0` (wave 1)** calls `element->vtable[+4h](0.05f)`, then
  `element->vtable[+0Ch]()`, then `BSP_EntityPose_RefreshWorld 00414DB0(payload)`; then,
  when `payload+10Ch` is clear, it refreshes the pose again, sets `payload+10Ch` and builds
  the cached inverse with
  `BSP_Matrix_BuildOrthogonalScaledAffineInverse 00B63D50(payload+110h, payload+CCh)` -
  an inlined copy of `BSP_EntityPose_GetDerivedAffineInverse 00414E10`.
- **`00875B90` (wave 2)** calls `008759B0(element, 0.05f)`, which walks the element's own
  sub-list at `element+1Ch` (sub-node: `+8h` prev, `+0Ch` next, `+10h` expired, `+11h`
  enabled): every enabled sub-node gets `sub->vtable[+0Ch](0.05f)`, and an expired one is
  unlinked under the lock from `00875280`, has its `element+24h` count decremented and is
  destroyed through `sub->vtable[0](1)`.
- **`00874FE0` (wave 3)** calls `element->vtable[+8h](0.05f)`, then
  `BSP_EntityPose_RefreshWorld(payload)`.
- **`00875160` (interpolation)** calls `element->vtable[+4h]([descriptor+8h])`, i.e. the
  same virtual wave 1 drives, with the leftover accumulator instead of the step. The
  fixed-step pose is advanced by `0.05f` per step and then by the remainder for the frame
  that is about to be drawn.

So the three per-step waves are three distinct virtuals on one object - `+4h` then `+8h`
with `+0Ch` in between - and the per-frame wave replays `+4h` with the remainder.

## The pool ABI

`BSP_FrameJobPool_GetSingleton 004C1130` returns the pool; the interface used is the base
subobject at `pool+4h` (`LEA ECX,[EAX+4]` at `00875CDA`, `00875D00`, ...):

| slot | native | form | use |
| --- | --- | --- | --- |
| `[[pool+4h]+4h]` | `00BE3020` | `RET 8` | queue `(job descriptor, element)` |
| `[[pool+4h]+8h]` | `00BE3150` | `RET 4` | dispatch `(byte at 00E0B6CE)` |

`docs/GAME_RENDER_FRAME.md` reconstructed the pool itself: `0x138A8` bytes, worker handles
at `+4h`, worker count at `+14h`, a running flag at `+18h`, a job count at `+20h` and 10000
two-word job slots from `+24h`; the render frame uses the same two slots. The queue call is
made once per admitted element and the dispatch once per group, so a wave issues at most
five dispatches and the pool runs each group's elements together.

The dispatch argument is the **byte** at `00E0B6CE`. It is not the double-buffer index: the
index lives in the word at `00E0B6CC`, which `00875C9A` writes with a word store that never
reaches `00E0B6CE`.

## Order within a fixed step

1. `008079B0(0.05f)` - the countdown at `00F874B8` (already documented).
2. Wave 1 over groups 0-4, wave 2 over groups 0-4, wave 3 over groups 0-4.
3. The sixteen-call fan-out, `docs/FIXED_STEP_FANOUT.md`. `00874C90` inside it is what
   makes newly constructed elements visible to the next step's waves.
4. `accumulator -= 0.05f`.

After the loop, once per frame: the interpolation wave, then the tail hook.

## Reconstruction

`include/bsp/fixed_step_job_waves.hpp` declares:

- `kFixedStepJobGroupCount`, `kFixedStepJobGroupStride` and the group/element/sub-node
  offset constants above.
- `JobWaveElementView`, a read-only projection of one element (payload flags, the group
  index) so the admission rules are pure.
- `fixed_step_wave_admits(JobWavePhase, const JobWaveElementView&)`, the four admission
  rules as one pure function.
- `JobWaveDescriptor`, the factory table row (`factory`, `singleton`, `allocation_size`,
  `body`, `dtor_thunk`, `carries_leftover`).
- `FixedStepJobWaveHost`, one pure-virtual method per native call site of the loop
  (`queue`, `dispatch`, and the element's three virtuals plus the pose refresh the job
  bodies make), and `run_fixed_step_job_wave` / `run_fixed_step_job_waves`, the loop over
  five groups with the queued-count rule.

Coverage: **complete** for the loop, the admission rules, the factory table and the four
job bodies. The element's virtuals are host methods, not reconstructions: what a concrete
`+4h`, `+8h` or `+0Ch` override does is a different packet. `008759B0`'s sub-list is
covered as a rule (`fixed_step_subnode_action`) but its `vtable[+0Ch]` callee is not.

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/IN_MISSION_SUBSYSTEM_TICK.md`: "the intrusive circular lists at `00F876C8` with stride `68h` (`&DAT_00F876F4 - 2Ch`)" | the group starts at `00F876C0`, not `00F876C8`; `00F876C8` is the head sentinel's `next` field, and the list is not circular but bounded by two sentinels `34h` apart | `00874CB8 ADD ECX,0xF876C0` and `00874CCA LEA ESI,[ECX+0x34]` against `00875CB7 MOV ESI,[EBX-0x2C]` |
| same doc: wave 2 "tests the same two bytes in the opposite order" (left as a possible difference) | the two orders are the same predicate; no admitted set differs | `00875CC3/00875CC9` against `00875D33/00875D3A`, both `JNE`/`JE` to the same skip label |
| same doc: the factories were unread, "four job factories" | three of the four are stateless singletons; only `00875670` carries state, the leftover at `+8h`, and that is why its allocation is `0Ch` and not `8` | `008756CA PUSH 0xC` against `0087552A`/`008755FA`/`008757AA` `PUSH 0x8`, each followed by `CALL 0x00BF681B`; `00875F50` stores the accumulator into `[EAX+8]` |

## no_ghidra_function

Eight vtable targets in this packet have no Ghidra function. Each is bounded below by the
vtable slot that points at it and above by the first `INT3` of the alignment padding that
follows the terminating instruction; the integrator can define them with
`python tools/ghidra_define_function.py <start> <end_exclusive> --record reports/fixed_step.json`.

| start | end (inclusive) | evidence |
| --- | --- | --- |
| `00874FB0` | `00874FB7` | `00D0DEA8` points here; `SUB ECX,4` + `JMP 0x875010`, `INT3` from `00874FB8` |
| `00874FE0` | `00875003` | `00D0DEAC` points here; `RET 4` at `00875001`, `INT3` from `00875004` |
| `00875070` | `00875077` | `00D0DEB0` points here; `SUB ECX,4` + `JMP 0x875100`, `INT3` from `00875078` |
| `008750A0` | `008750F7` | `00D0DEB4` points here; `RET 4` at `008750F5`, `INT3` from `008750F8` |
| `00875160` | `00875176` | `00D0DEBC` points here; `RET 4` at `00875174`, `INT3` from `00875177` |
| `00875180` | `00875187` | `00D0DEB8` points here; `SUB ECX,4` + `JMP 0x8751B0`, `INT3` from `00875188` |
| `00875210` | `00875217` | `00D0DEC0` points here; `SUB ECX,4` + `JMP 0x875240`, `INT3` from `00875218` |
| `00875B90` | `00875BA5` | `00D0DEC4` points here; `RET 4` at `00875BA3`, `INT3` from `00875BA6`, and `00875BB0` (the driver) starts after the padding |

Because these eight lie in no Ghidra function, `tools/verify_report_calls.py` cannot check
call sites inside them; the report lists those rows under `job_body_steps` with a `site`
key instead of `address`, and the four direct calls they make (`00414DB0`, `00B63D50`,
`008759B0`) are named there.

## Follow-up packets

| packet | addresses | why |
| --- | --- | --- |
| `tick_element_overrides` | 00875920 0042bb70 0042bb80 0042bb90 007f2c60 006e7b00 00929e50 0087b670 | the six-slot vtable is all stubs in the base; what a bullet, an aircraft and an objective actually do per step is the next layer |
| `tick_element_sublist` | 008759b0 00875280 00875340 | the sub-node list `008759B0` drains, its `vtable[+0Ch]` callee and the two lock singletons |
| `job_pool_dispatch` | 004c1130 00be3020 00be3150 00be4800 | the queue and dispatch bodies, the worker wake-up and what the `00E0B6CE` byte selects |
| `tick_group_semantics` | 00874c90 00875890 007f2c60 | why aircraft go to group 3 and everything read here to group 0, and whether groups 1, 2 and 4 are used at all |

## Correction from docs/TICK_ELEMENT_OVERRIDES.md

Packet `cc2-tick-element-overrides` (main 36d71a2b) read the six-slot interface at `00D0DEC8` and
corrects three claims here: the unit's constructor argument is 0 but level 5 writes 1 into `node+14h`
before the first splice, so units tick in group 1; `element+30h` is a time scale disabled at -1.0f, not a
timer seed; only three of the five stubs are ever overridden. The interface is one fixed-step pose
contract: slot `+4h` restores the committed pose and advances it by its float (wave 1 passes 0.05f, the
interpolation wave passes the frame leftover and never commits; `00929CB0` divides its argument by 0.05f
into the physics interpolator's alpha), `+8h` advances the simulation, `+0Ch` commits the pose, `+10h`/`+14h`
are the base stub in all eight concrete tables. The group index picks the job-pool batch; every wave loops
all five groups with the same admission test.
