# Dyn body creation and native storage

Addresses: 00C5D580, 00C43CA0, 00C5C940, 00C57F50, 00C57C40, 00C55FC0,
00C50470, 00C54AA0, 00C4CDA0, 00C36F10, 00409170, 00409450, 0040B4A0,
0040AC90, 0040B750, 00407C70, 00407ED0, 004085F0, 00408040; selected stores
from 00C41AD0. Descriptive suffixes are hypotheses, not recovered symbols.

`dyn_world_create_convex_body_00c5d580` now produces the native 88h body,
its C8h dynamic motion record or borrowed static motion record, linked 218h
convex-shape slots, and a real 50h SAP proxy. It accepts the existing
`DynBodyDescriptor`, `DynShapeDescriptor`, and actual `AvoidZoneDynHullData`.
It implements both body allocation branches and restricts shape dispatch to
type 4. The other shape types are rejected before allocation. There is no
substitute geometry or physics algorithm.

This is a C++ interface over native Win32 storage, not a drop-in ABI replacement
or a constructed physics world. The caller must supply the real world/scene
storage, coherent allocation ownership and complete callable vtables. In
particular, shape virtual lifetime operations must free through the same convex
allocator that created the slots. Collision dispatch, thread tasks, SAP endpoint
insertion, stepping and world/body destruction remain outside this packet.

## Producer contracts

| Entry | Recovered behavior and native ABI |
| --- | --- |
| 00C5D580..00C5D8F3 | ECX world, stack descriptor, RET4. Static flag 1 selects world+4C body pool; dynamic selects +170 and motion pool +294. The native descriptor vector at +78/+7C is an explicit pointer array/count in the new API. |
| 00C43CA0..00C43E9C | EAX body, EDX descriptor, RET. Initializes all established body fields and dynamic motion fields; static motion is untouched. |
| 00C5C940..00C5CAB8 | ECX shape descriptor, EDX body, RET. Type 4 calls 00407ED0 then 00C57F50, links the shape at body+70, recomputes bounds, and registers the first shape's body. Cases 0/1/2/5 and invalid selectors are outside the source API. |
| 00C57F50..00C57FE8 | ECX body, stack shape/descriptor, RET8. Writes material, filter, transform, list links and borrowed hull pointer, installs ConvexMeshShape vtable 00D7A0AC and refreshes bounds. |
| 00C57C40..00C57F46 | ECX shape, RET. Expands the actual hull box by double 0.02, transforms it using the native x87 store boundaries, then recomputes body bounds. |
| 00C55FC0..00C56441 | ESI body, RET. Unions both corners of every linked shape. Flag 8 also updates proxy bounds and invokes the real manager slot 2. |
| 00C50470..00C50731 | ESI body, stack scene, RET4. Sets flag 8, transforms body bounds, calls scene manager slot 0 and stores returned proxy at body+60. The identified concrete slot is implemented directly. |
| 00C54AA0..00C54D96 | ECX manager, stack body/bounds/static byte, RET0Ch. Allocates a proxy, initializes it, and appends it to the pending-insertion vector. |
| 00C4CDA0..00C4CE5A | ECX body, DL static, stack proxy/bounds, RET8. Sets proxy body/bounds/static byte, clears inserted byte +38, and allocates ten adjacency pointer slots. |
| 00C36F10..00C3700E | Stack manager, RET4. Constructs three endpoint pools, pair pool, static/dynamic proxy pools, and a 500-entry pending vector. |

The existing semantic `DynBody` and `DynMotionState` are not native layouts.
They omit such fields as the previous transform at motion+84. This module uses
separate byte-storage records instead of duplicating or changing those semantic
types. The native body descriptor is 84h; the semantic descriptor keeps only a
shape count after its matching 78h prefix. The new call makes the actual vector
input explicit and checks the count.

Body+66..67 is unspecified. Dynamic motion+B5..B7 is unspecified. Shape+64..203,
most unused proxy fields, and unused pool slots remain as the allocator supplied
them. Source writes follow the established stores rather than clearing complete
objects. Static bodies read their motion pointer from world+368: this is the
first active motion-pool node, not a null pointer. The native world constructor
allocates it and zeros C0h bytes while preserving its list links.

## Pool and owner state

The five pool constructors share a precise layout: page vector at +0/+4/+8,
free head +C, start sentinel +10, end sentinel +10+stride, active count
+10+2*stride, with previous/next in each slot's final eight bytes. Each constructor
allocates 1,000 slots and an initial two-pointer page vector. Growth is
`capacity * 2 + 2`, and page allocation threads each slot's final word. Taking
a slot pops the free chain and appends it before the end sentinel.

| Constructor | Slot stride | Container bytes | Use |
| --- | --- | --- | --- |
| 00409170..0040926F | 88h | 124h | Bodies |
| 00409450..0040954F | C8h | 1A4h | Motion states |
| 0040B4A0..0040B58D | 10h | 34h | SAP axis endpoints, ECX input/RET |
| 0040AC90..0040AD7F | 10h | 34h | SAP pairs |
| 0040B750..0040B83F | 50h | B4h | SAP proxies |

Other pool constructors take the storage pointer on the stack and RET4.
`dyn_world_body_pool_fragment_00c41ad0` implements only the three pool calls
at 00C41B4D/00C41B5F/00C41B70 and shared-motion tail 00C41FA4..00C420C1.
It deliberately does not construct a world or scene. Existing world settings
remain in `dyn_world_settings.hpp`.

Scene constructor 00C38070 allocates 248h and calls 00C36F10. The concrete
manager is **Dyn::SAPBroadPhaseManager2**, vtable 00D7A160 (RTTI 00E171C8),
not SAPRadix. Its dynamic proxy pool begins +D4 and static pool +188; pending
insertion vector is +23C/+240/+244. Slot 2 at 00C4C270 calls endpoint updates
00C4BE10 for three axes only when proxy+38 is already inserted. This module
calls that actual borrowed virtual method; it does not substitute an empty
callback. Newly created proxies remain queued for the separate insertion pass.

The convex allocator is the real `Mit::cPoolAllocator<532,128>` at 0109ECF0.
00407C70 registers it in the allocator list rooted at 00E188B4, initializes its
critical section and reserves 32 page pointers. Its explicit storage form takes
that registry head and vtable as borrowed inputs. 00407ED0 allocates 10D04h pages;
004085F0 writes 128 reverse-ordered uint16 free indices and each slot's page
number at +214. Slot stride is 218h, including that metadata. 00408040 returns
an unlinked slot to its page and updates the earliest free page. These operations
preserve the native critical-section and lock-depth behavior on normal paths.
Full allocator shutdown/registry unlink and allocation-failure SEH remain external.

Shape+210 stores the dereferenced descriptor geometry handle. No retain or copy
occurs inside shape creation. The caller's retained hull handle must therefore
outlive every body shape using it. The preceding hull packet implements that
retained copy and data ownership; this module leaves it intact.

## Evidence and verification

Live project/program checks used `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` through the repository wrappers. All twenty native
code spans matched installed PE bytes before relocation. Parent integration
repaired the false-free fallthroughs and refreshed exports under the write lock;
the worker performed no Ghidra mutations. The report records inclusive bodies,
original register/stack ABIs, all 114 native call rows, and partial coverage.

MSVC Win32 Release build and both existing CTests passed. One ignored probe
linked that Release library and executed the original relocated machine code:

- 4,100 bodies: 2,050 static and 2,050 dynamic; first 706 with one shape, others
  with two, totaling 7,494 shapes and 4,100 proxies.
- All 706 installed Marshall extrusion inputs, built into actual hull data with
  the previously verified Dyn producer; rotated/scaled shape and body transforms,
  non-default material, velocities, inertia, damping and listener/owner words.
- Three-page growth for body/motion/proxy pools, convex page-vector growth,
  pending-vector growth, and 130 slot return/reuse comparisons.
- All 4,190 live owned buffers, including unused capacity, free links and native
  unspecified bytes, matched after normalizing only proven pointer fields.
  There were 4,200 allocation operations per creation arena.

The 38h convex allocator header itself was not byte-compared: registry links,
the Windows critical-section record, lock depth and top-level vector/header words
are excluded. Page metadata and the allocation/return/reuse effects were compared.
All tested proxies retained inserted byte +38 = 0; actual inserted endpoint
updates remain an untested borrowed virtual-call boundary.

Both worker record files have SHA256
`9a44aca7d386b637f92ec0e318b979fcd2c3cd3be3bf015c944ac64474abfd72`.
The integrator reproduced all counts against the merged Release library; its
two files match SHA256
`7c0261fba7730951c24c70d61fc6da85fda471affb44e8b1131b371f781de967`.
A separate audit found exactly 14,988 differing words between runs: the 7,494
borrowed hull pointers at shape+210, serialized both per body and in their pool
slots. These 706 shared hull allocations bypass creation-arena normalization.
They match between native and rebuilt within each run but retain process heap
addresses. No other bytes differ. Normalizing only those validated occurrences
to ordered hull IDs gives the same SHA256
`a58e17a99c043f1d4b8a203bd45442c38fe9cedc605b606f2df75cf4f45d1491`.
Raw record hashes therefore identify a run, not a stable cross-run artifact.
The fixture released all tracked allocations after discarding its quiescent
storage; this is not a native body/world destructor test. Its world storage came
from real pool constructors plus a hand transcription of the shared-motion
allocation/link/zero stores. The original world-constructor tail itself was not
executed. Its SAP manager came from the actual constructor. Scene/task startup was not
executed. CRT allocation, memory operations and constructor iteration were bound
to the host CRT, while semantic Dyn calls and the real SAP slot 2 remained native.
The probe fixed the six relocated switch-table entries and actual vtable pointers;
no original executable or saved analysis bytes were changed.

Ignored evidence is under `local/dyn_body_*`, `local/body_probe.log`,
`local/report_calls.log` and `local/build.log`. Installed extrusion input SHA256:
`5dc602ba766b1c5d866fe4f2367fd0e0e488910964f712553d81c9fd97be212e`;
source scene SHA256:
`9235d9a6364b08cbd5cf3c6735ba9828aba5a6e7b2569300dc2eecf6d8dac17b`.
These are producer/fixture results, not game runtime or visual validation.

## Ready follow-up

Connect the existing real avoidance hull/descriptor path at 00423C50 only when
its world/scene owner is available. Reconstruct 00C38070's dispatch/manifold/task
ownership and the remaining 00C41AD0 world startup before claiming a standalone
world. SAP endpoint insertion and full body destruction are separate native
dependencies. Other shape constructors can extend the dispatch without replacing
the verified allocator, body initialization or convex path.
