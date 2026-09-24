# Raw camera world editing (CC10)

Addresses: 00B6E7E0, 00B6E870, 00B70660, 00B71460.

The four complete normal bodies operate on actual node/camera storage through new source interfaces in `native_camera_raw_world_edit.hpp`. They reuse the published raw matrix, world-refresh and descendant-invalidation bodies. They do not adopt a logical `CameraTransform`, a `SceneAttachmentRuntime`, or a camera lifetime companion.

| Original entry / exclusive end | Bytes | Original ABI | Coverage / source entry |
| --- | ---: | --- | --- |
| B6E7E0 / B6E863 | 131 | ECX actual node, RET | complete; `derive_native_camera_local_00b6e7e0` |
| B6E870 / B6E8B6 | 70 | ECX actual node, stack source, RET4 | complete; `set_native_raw_world_matrix_00b6e870` |
| B70660 / B7070D | 173 | ECX actual camera, RET | complete; `refresh_native_raw_camera_direction_00b70660` |
| B71460 / B71482 | 34 | ECX actual camera, stack source, RET4 | complete; `set_native_raw_camera_world_00b71460` |

## Storage, scratch and arithmetic

The actual words remain parent+30, child+34, sibling+3C, flags+5C, inverse+60, notification owner+A0, local64+B0 and world64+F0. Camera target+1A0/direction+1AC are three DWORDs each; camera flags are at+2F0. Actual B6F5A0 establishes the base prefix. This packet does not establish the derived B71A80 lifetime or initialize unwritten tail bytes.

Frames are disjoint immutable pointer metadata borrowing initialized live volatile DWORD backing, with stable addresses. A derive view covers contiguous inverse16/local16 DWORDs; a direction view covers contiguous position3/target3 DWORDs. No aggregate object is overlaid on overlapping backing. For a B6E7E0 native entry ESP=E, inverse begins E-80h and local E-40h. B70660 position begins E-18h and target E-0Ch. For the outer B71460 entry E, derive begins E-94h and direction E-20h: the latter is the former+74h. One actual backing array can express that nested overlap. Derive scratch is dead before direction; its contents after a later call are not presented as the original derive-return output. Native saved registers, return addresses, pushed arguments and other private gaps are excluded from the source frame ABI and alias domain.

B6E7E0 captures the parent once. Captured parent flags bit8 skips inverse rebuilding; otherwise bit2 controls B6DB70. B63B30 writes inverse scratch, its returned EAX is copied to parent+60, and only then the current parent flags are ORed with8. 413920 multiplies current own world by captured parent inverse into the local scratch, then 4134F0 copies its returned EAX to own local. A root copies own world directly; both scratch regions retain their preimages on that branch. Matrix inputs/outputs may overlap within the addressed valid-memory domain; the source preserves the published helpers' forward x87 stores and late reads. Clear DF, valid acyclic actual hierarchy and the existing scaled-affine inverse domain are required. No singular or overlap repair is added.

B70660 tests the current low flags byte, refreshes world if needed, and performs three ordered FLD/FSTP copies from world110/114/118 to direction1AC/1B0/1B4. It retests current bit2 independently. Position120/124/128 is spilled into three actual scratch DWORDs. The additions are position.x+direction.x, direction.y+position.y, direction.z+position.z, with three distinct float32 spills followed by ordered FLD/FSTP target stores. The naked Win32 core retains x87 operations and exceptional encodings; EDX now supplies the disjoint source frame metadata. This is a new interface, not an original binary replacement.

## Current callback order

B6E870 captures the source argument word once before the first matrix store. After raw copy it loads current A0, current profile and current slot3C, then invokes the exact captured target on that actual receiver. Local derivation occurs after that call, so callback changes to the parent are visible. Descendant invalidation precedes the current own profile/current slot40 load. Only after the exact reached virtual40 returns does the body assign flags5C=2. B71460 captures its source before clearing flags2F0 with FFFFFE4B, performs this world edit and then refreshes direction.

`NativeNodeBaseWorldDispatch` remains a required genuine provider. Profile resolution must be a pure lookup of the captured numeric identity into the borrowed current table, with no mutation, callback or fallback. Reached targets must execute their actual body on the actual receiver. Published B6DBE0 and B6DBC0 provide concrete closures in their admitted profiles; B6DBC0's continuing A0 chain requires its genuine current-profile service. No logical notification callback replaces either target. Captured/current table reads and source argument reads are not hoisted over calls.

## Providers and evidence

| Provider | Published source / contract |
| --- | --- |
| B6DB70 | `native_camera_world.cpp`; actual parent/world recursion, original80B |
| B63B30, 413920 | `native_camera_matrix_math.cpp`; raw531B inverse and874B multiply, exact x87/alias schedule |
| 4134F0 | `native_camera_matrix_copy.cpp`; ordered16 x87 copy pairs |
| B6DA30 | `native_node_raw_transform.cpp`; current actual descendants and cache invalidation |
| B6DBE0 / B6DBC0 | `native_node_base_destruction.cpp` / `native_node_raw_transform.cpp`; actual current world/bounds notifications |

The report pins all408 live/PE bytes, complete instruction extents and every native direct/indirect call site. All four Ghidra function bodies already have complete ranges; no definitions, flow repair or EH handler are required. The worker performed no Ghidra mutation. Descriptive existing names remain unchanged.

One ignored focused probe compares five original/source pairs using the same genuine providers: standalone parent derive; wrapped edit with partial source/destination overlap; wrapped root edit with sNaN/infinity/subnormal values; standalone direction refresh; direct world edit. Original bodies are copied with only13 rel32 call operands redirected to copied packet entries or published genuine providers. Both indirect instruction streams are unchanged. Native profile words point to fixture-owned table storage whose reached entries point to exact provider bridges; source resolution maps those same identities to native target numbers. The fixture uses one live stack DWORD array for original private locals and source views, comparing standalone derive output only at its own return and direction output at the outer return. It compares whole initialized node payloads, normalized actual identities, callback trace and masked x87 status.

The mutation pair executes genuine initial B6DBC0, then explicitly injects a harness mutation of actual parent/current profile. The changed slot40 reaches genuine B6DBC0 instead of B6DBE0; both resolve through actual current table data and execute the published bounds body. This tests the dispatcher contract's late reads; it is **not** evidence that the game's B6DBC0 changes those fields. The normal pair executes genuine B6DBE0 followed by the current A0 B6DBC0. Native node construction is genuine; camera tail preimages are explicit fixture storage, not evidence of full B71A80 construction or camera companion admission.

Strict Win32 build and all three existing CTests pass. The source has no new tracked tests. These four native bodies install no EH and perform no allocation or owner-credit operation. A throwing required provider retains the completed prefix; no rollback is invented. Native access faults/FH3 identity, asynchronous mutation, aliases into excluded native saved/private gaps, whole constructor/lifetime admission and game runtime integration remain unexercised. The fixture's initial missing bounds-chain resolver was corrected in the harness; the final five-pair run passes without changing packet source behavior.

The separate ignored `local/cc10_camera_constructor_readiness/README.md` retains B71A80's604B schedule, actual mutable incoming argument/scratch and43B compiler boundaries. Next raw pose leaves and the B63F10 builder's private scratch require independent recovery before that constructor can be admitted.
