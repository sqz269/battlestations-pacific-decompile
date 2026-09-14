# Native SBH allocation source (BZ)

This packet implements the complete `C12968` allocation body (739 native bytes), `C1207C` region helper (176 bytes), and `C1212C` group helper (262 bytes). It uses the actual borrowed native CRT cells and real Win32 HeapReAlloc, HeapAlloc, HeapFree and VirtualAlloc. It creates no heap, globals, allocator callback, container or bootstrap owner.

The accepted discovery is `d61bab1db06225b86fc80d7b087e29ab4d90b2b0`, report SHA256 `3a51bbfa25cb639c8f001e263f6ab5f22dbf3a341f2ece37abf432cf9bdff019`. All 1,325 frozen discovery artifacts are retained unchanged, together with that document and report. Fresh BSP CLI batches verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, and all 1,177 native bytes against the installed PE SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Source base: published `06e8986477baed33aa46e05c368f65026f4c6df5`. The original functions are cdecl: size to pointer-or-null at C12968; no argument to descriptor-or-null at C1207C; descriptor to group-index-or-minus-one at C1212C. The added state references, C++ symbols, compiler stack/register use and omitted native argument-slot scratch writes define new source interfaces. They are not native ABI thunks or native frame, FH3, SEH or asynchronous-fault identity claims.

## Canonical state binding

`include/bsp/native_crt_sbh_state.hpp` defines one borrowed `NativeCrtSbhState`. It does not define the storage referred to by its members. Its types match the unchanged `NativeCrtSmallBlockHeapInitContext` exactly:

| SBH view | Type | Existing initializer member / owner |
| --- | --- | --- |
| `actual_heap_0109e1bc` | `void* volatile&` | `actual_heap_0109e1bc` |
| `actual_descriptor_count_0109ed64` | `volatile uint32_t&` | `actual_word_0109ed64` |
| `actual_descriptor_capacity_0109ed74` | `volatile uint32_t&` | `actual_word_0109ed74` |
| `actual_descriptors_0109ed68` | `void* volatile&` | `actual_result_0109ed68` |
| `actual_allocation_rover_0109ed70` | `void* volatile&` | `actual_second_result_0109ed70` |
| `actual_retained_empty_descriptor_0109e310` | `volatile uint32_t&` | `actual_word_0109e310`; raw native pointer bits, not a pointer reference |
| `actual_retained_empty_group_0109ed78` | `volatile uint32_t&` | Same actual native owner; C11CF5 does not initialize this word |

Static type checks enforce the six existing member mappings. `bind_native_crt_sbh_state` returns a reference view of those exact initializer cells plus the caller's actual ED78 cell. It neither invokes the initializer nor supplies values. Both initializer files remain unchanged. The native threshold ED6C remains with the initializer and outer allocation policy; the three bodies here do not add a threshold test.

The caller must bind actual canonical state, perform the real required bootstrap, hold the established external lock-4 contract, and supply coherent native SBH data, the original valid size-class domain and sufficient writable mapped extents. The view is stable for the call. Original bootstrap, teardown, matching free integration and other callers remain separate ownership work. Allocation failures are allowed and retain their original partial state; this is not a promise that failure restores entry invariants.

## Source behavior

All arithmetic that wraps in the native body uses 32-bit unsigned values. Signed class decisions and arithmetic shifts use the required MSVC Win32 signed representation; shifts mask their count to five bits. Integer address calculations preserve unsigned comparisons and wrapped offsets. There is no new null, overflow, mask-consistency or class-range rejection.

Raw record helpers use inline x86 assembly for native-width reads, stores, AND/OR read-modify-write operations and byte decrement. This prevents host typed-object assumptions about raw metadata and preserves accesses such as OR -1 on newly allocated descriptor words whose prior bits are not initialized. No old indeterminate word is given an invented C++ meaning. These helpers are implementation operations, not fabricated native providers.

The allocation body preserves high-mask-before-low-mask reads during descriptor/group eligibility checks, the two rover-based scan ranges, the separate available-group scan, actual helper calls, publication of a failed group index before returning null, and rover publication only after that failure check. It reads the current metadata pointer separately for group-result publication, failure checking and later allocation.

Allocation takes the first node in the selected native size-class ring and carves from its high end. If the remainder stays in the same class, the ring/masks remain unchanged. Otherwise it clears an emptied group's bit, decrements the class byte count, conditionally clears the descriptor's bit, unlinks with the original intervening reloads, and reinserts a nonzero remainder in order. A newly nonempty class increments its byte count before descriptor/group bit publication. Remainder tags precede allocated tags; the group allocation count precedes retained-empty marker clearing; the current-group store is last. Payload bytes are not cleared by the allocation body.

The region helper uses actual current heap/array cells. Descriptor growth uses HeapReAlloc flags zero and wrapped `(capacity+16)*20`; success updates current capacity, reloads current count, then publishes the array. Later failures retain those updates. HeapAlloc uses flags 8 and 41C4h bytes; its result is published into the descriptor even when null. VirtualAlloc reserves 1 MiB with type 2000h/protection 4 and likewise publishes null. Reserve failure frees the current metadata pointer with the current heap, ignores HeapFree's result and leaves the descriptor word uncleared. Success publishes available-group and class masks, increments current count, reloads metadata and sets current-group to -1. There is no rover adjustment in this helper.

The group helper initializes 63 empty sentinels before its real VirtualAlloc commit. Commit failure returns -1 with those list pairs retained. Successful initialization uses the requested address, retains the native unsigned wrap branch and page-store order, patches the class-63 ring endpoints afterward, then publishes low/high group masks, the class-63 byte count, the conditional descriptor summary bit and the available-group bit clear. It does not set the group allocation count or current-group index itself.

The interfaces return null or -1 on the native failure paths; no host `bad_alloc`, error callback or transactional cleanup is inserted. Existing host CRT `singleton_lifetime_allocate/free` is not used. Native BF9E56's external lock/scope and BF9F1A's fallback/new-handler/errno policy are not reconstructed by these three source bodies.

## Validation and limitations

The machine-readable report binds the final source, compiler command/read/write logs, current object, exact unique member of the current archive, extracted member equality and focused emitted-code evidence. It also carries every native span and the complete frozen local artifact inventory with independent SHA256/SHA512 passes.

The Release Win32 build passed with `/W4 /WX /fp:strict`; all eight seed comparisons and both existing CTests passed. Build inputs remained unchanged. The new allocation object and unchanged initializer object each occur exactly once in the current archive, and each extracted member equals its current object. The math tests are not validation of SBH allocation behavior. No new test, probe or allocation executable is introduced, and no game/native allocation execution is performed. Static source and emitted-code checks support this bounded source reconstruction; whole-heap ownership, native ABI/frame/fault identity and runtime/game behavior remain unclaimed.

The emitted block body is 1,709 bytes: MSVC inlined the actual region source provider with its four real Win32 IAT calls, while retaining the direct group-provider call. The standalone region and group bodies are 314 and 551 bytes. Fourteen focused emitted-code ranges retain the material arithmetic, current loads, alias-visible stores and failure publications. Canonical volatile count/cache updates can lower to separate reads and stores; the E310 clear does so. Raw-record AND/OR and byte decrement helpers retain native-width read-modify-write instructions. These observations do not claim native instruction, call-boundary, stack or asynchronous-fault identity.

Only the new source/headers, appended deferred bsp_core registration, three owned reconstruction records, this document and its report are delivered. No initializer edit, Ghidra mutation/settings change, shared-global definition, merge or push is included.
