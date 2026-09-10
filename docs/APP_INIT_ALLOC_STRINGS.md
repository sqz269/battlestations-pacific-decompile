# Application initialization allocator and string helpers

Addresses: 0041dd40, 0041e870, 00438e40, 00419cc0, 00bd1510, 00bd1780, 00bd17a0, 00be0a30,
00bd1120, 00bd11f0, 00bd1620, 00bd12a0, 00bd1680, 00bd0f50, 00bd1000, 00bd1480, 00bd1570

Packet `app_init_alloc_string_helpers`, owner `agent/init-alloc-strings`. Every claim below was read
from the disassembly, not from the pseudocode: these helpers pass arguments in registers, call
singleton getters that consume no stack, and Ghidra attributes the caller's pushes to the wrong
callee at 0041dd64, 0041ddc9, 00bd1780 and 00bd17a0. Ghidra is read-only for this packet; the
reviewed names listed here are recorded in the ledger for the integrator to apply.

## The two pools

The process has two instantiations of one sized-pool template, both lazy singletons behind a
critical section, both built the first time anything asks for memory and never destroyed until
teardown is registered through 00415350 / 00bd0c30.

| | String pool | Game pool |
| --- | --- | --- |
| Getter | 00419cc0 | 00bd1680 |
| Singleton global | `DAT_01090aa8` | `DAT_01090aac` |
| Constructor | 00bd1480 (ring 00bd0f50) | 00bd1570 (ring 00bd1000) |
| Object size | 0x8ad4a0 | 0xb89ab4 |
| Allocate / release | 00bd1120 / 00bd1510 | 00bd11f0 / 00bd1620 |
| Size classes, threshold | 0x96 (150) | 0x81 (129) |
| Ring slice per class | 0xda7 (3495) | 0xfe0 (4064) |
| Bump arena | this+8, 7,000,000 bytes | this+0x200414, 10,000,000 bytes |
| Carve granularity | exact byte count | size rounded up to 4 |
| Public face | `BSP_NativeString_Resize` | GameAlloc / GameFree |

Both getters take **no arguments** and return the pool in EAX with a plain `RET`. Callers push the
arguments of the *following* `__thiscall` before calling the getter, which is why the decompiler
invents parameters for them. The lock is a `CRITICAL_SECTION` plus one adjacent depth counter at
`+0x18`; enter and leave go through the imports at `[00ce2218]` and `[00ce2210]`, and
`InitializeCriticalSection` at `[00ce220c]` runs in the constructor.

### Pool object layout (string pool, 00bd1480)

| Offset | Field |
| --- | --- |
| +0x0 | vtable `00d68200` |
| +0x4 | unused, never written by the constructor |
| +0x8 | bump arena, 7,000,000 bytes, uninitialized |
| +0x6acfc8 | bump offset, zeroed by the constructor |
| +0x6acfcc | free-ring subobject, 0x2004b8 bytes |
| +0x8ad484 | critical section plus depth counter, 0x1c bytes |

The game pool is the same shape with the ring first (+0x4) and the arena after it (+0x200414).

### Free-ring subobject

| Offset in ring | Field |
| --- | --- |
| +0x0 | 0x80000 pointer slots, 2 MB |
| +0x200000 | `head[class_count]` |
| +0x200000 + 4*class_count | `tail[class_count]` |
| after `tail` | live block count |
| +4 | high-water mark |

Every size class owns a contiguous circular slice of that one shared ring. The constructor lays them
down as `head[i] = i * stride`, `tail[i] = head[i] - 1`, and rewrites entry 0's tail with the mask
`0x7ffff` so the empty test `(tail[i] + 1) & 0x7ffff == head[i]` holds for every class. 150 classes
at stride 3495 leave 38 slots unused at the end of the ring; 129 at 4064 leave 32.

### Allocate, 00bd1120

`__thiscall`, ECX pool, stack `(uint size, uint unused)`, RET 8. The second argument is never read;
`BSP_NativeString_Resize` passes 1 for it. `size >= 0x96` goes straight to `_malloc` 00bf9f1a with no
locking. Below the threshold, under the lock: pop `ring[tail[size]]` and decrement `tail[size]`, or,
when the class region is empty, carve from the bump arena and advance the offset by the exact byte
count. The class index **is** the byte size, so a freed block is only ever reused for a request of
exactly the same size.

**There is no arena bounds check anywhere in either pool.** The carve reads the offset, adds, stores,
and returns a pointer. A workload that allocates more small blocks than the arena holds without
freeing them walks off the end silently.

### Release, 00bd1510

`__thiscall`, ECX pool, stack `(void* block, uint size, uint unused)`, RET 0xC. The third argument is
never read. `size >= 0x96` goes to `_free` 00bf9dc8. Otherwise, **only while `DAT_01090aa4` is zero**,
the block is pushed onto the ring by 00bd12a0 with ECX = ring base and arguments
`(&block, size)`; the pointer is read through, never written back. A non-zero `DAT_01090aa4` drops the
block on the floor: it is neither pooled nor freed.

The size argument selects the free list, so it must be the size the block was allocated with. This is
the single most important contract in this layer, and it is why every string release recomputes
`length + 1` rather than remembering a capacity.

### Ring push, 00bd12a0

Advance the class's own tail. If that runs into the head of the next class, rotate that class's whole
region forward by one slot: its first element is displaced into the slot just vacated, becomes the
block still looking for a home, and the cascade continues to the class after it, modulo the class
count, until a class with slack is found or the walk wraps back to the starting class. Then
`ring[tail[last]]` takes the carried block, the live count rises and the high-water mark follows.
`allocate` decrements the live count and deliberately leaves the high-water mark alone.

## GameAlloc 00bd1780 and GameFree 00bd17a0

Three and six instructions respectively. Both are `__cdecl`; the caller cleans the stack.

```
void* GameAlloc(unsigned size);              // 00bd1780 -> 00bd1680 then 00bd11f0
void  GameFree(void* block, unsigned size);  // 00bd17a0 -> 00bd1680 then 00bd1620
```

The free is **sized**, and Lua's `l_alloc` 00a6a1d0 is the proof: it allocates `nsize`, copies
`min(osize, nsize)`, and frees the old block with `osize`, which is exactly the reallocation protocol
this pool forces on its callers.

`docs/APP_INITIALIZE_MAP.md` phase 0 reads these two as "allocator hook installers". That is wrong.
The disassembly at 0073d431 is an allocator **warm-up**:

```
PUSH 0x10 ; PUSH 0x10 ; CALL 00bd1780 ; ADD ESP,4 ; PUSH EAX ; CALL 00bd17a0 ; ADD ESP,8
```

It allocates sixteen bytes and immediately frees them, forcing the game pool singleton, its 10 MB
arena and its 2 MB ring, to exist before anything else in initialization runs. The two 0x10 pushes
are the sizes for the two separate calls. These are the only two callers besides `l_alloc`.

## The native string

Eight bytes, no capacity, no small-buffer area, no allocator pointer, no destructor:

| Offset | Field |
| --- | --- |
| +0x0 | `unsigned length` |
| +0x4 | `char* data`, null when the string has never held a non-empty value |

The buffer is always `length + 1` bytes and holds a terminator at `data[length]`. Capacity and length
are the same number, so **the growth policy is exact**: there is no geometric growth, no slack, and a
shrink reallocates just like a grow. Ownership is single: the object owns its buffer and no routine in
this layer ever shares one. There is no aliasing hazard on grow because the new block is taken before
the old one is released.

### Resize, 0041dd40

`__thiscall`, ECX string, stack `(uint length, char preserve)`, RET 8. 2280 callers.

1. `length == length_` returns immediately, **even when the buffer is null**. Resizing a zeroed string
   to 0 is a no-op that leaves `data` null, which is why callers test the pointer and not the length.
2. `length == 0` releases the buffer with `old_length + 1` and clears both fields.
3. Otherwise allocate `length + 1`, then, if `preserve`, copy `min(old_length, length)` bytes, then
   release the old buffer with `old_length + 1`, then store the new pointer and length and write
   `data[length] = 0`.

When `preserve` is set on an empty string the native calls `memcpy` with a null source and a count of
zero; the reconstruction skips the call, which is the same result.

### Assign, 0041e870

`__thiscall`, ECX string, stack `(const char* text)`, RET 4, returns `this`. 1230 callers.

This is a **constructor body, not an assignment**. Its first two instructions zero the length and the
pointer, so calling it on a live string abandons that string's buffer. It then measures `text` inline,
calls `Resize(len, preserve=1)`, and copies `len + 1` bytes if the pointer came back non-null,
rewriting the terminator `Resize` already placed. Assigning an empty string leaves `data` null.
`BSP_NativeString_ConstructFromCString` would be the accurate name; the existing one is kept so the
five concurrent application-initialization packets keep referring to the same symbol.

### Duplicate, 00438e40

`__fastcall`, ECX `const char* text`, no stack arguments, RET 0. 29 callers. Returns null for a null
argument, otherwise a `strlen + 1` byte copy from the CRT helper 00bf55be, a tail jump to 00bf681b.
**This one is not a pool block**: it never touches 00419cc0, and its callers release the result with
`_free` 00bf9dc8, including the copy `BSP_Application_Initialize` parks in `DAT_00e1ae78` and frees at
0073e466.

### The copy idiom, inside 00be0a30

`BSP_FileBlock_Construct` embeds a native string at `this+0x14` and copies into it with the idiom the
rest of the codebase repeats: compare the two addresses to reject a self-copy, `Resize(source.length,
preserve=1)`, then `memcpy` of exactly `length` bytes over the terminator `Resize` wrote. The object is
28 bytes: vtable at +0 written twice (base 00ceb130 then 00d68494), refcount 1 at +4, three zeroed
dwords, then the string. The rest of its behaviour, `BSP_FileBlock_PrepareIdentifier` 00bdf950 and
`BSP_VFS_EnterFileBlock` 00be0980 on the singleton at `DAT_0109ceec`, stays as
`docs/VFS_LOAD_PROCESSING_START.md` established it; this packet only adds the string evidence.

## Reconstruction

`include/bsp/storage_pool.hpp`, `src/storage_pool.cpp`, `include/bsp/native_string.hpp` and
`src/native_string.cpp`. The pool geometry is a value (`SizedStoragePoolConfig`) with the two
recovered configurations as factory functions, the system allocator is injected, and the string takes
its storage as an explicit argument on every call so the object stays exactly eight bytes and reads no
global. `static_assert(sizeof(NativeString) == 8)` holds.

Deliberate deviations from the native, all of them at points where the native is simply unchecked:

- The arena carve throws `std::bad_alloc` where the native would run past the end of the arena.
- `allocate` never returns null; the CRT-backed allocators throw instead.
- `memcpy` with a zero count and a null source is skipped rather than called.
- `NativeString` has no destructor, matching the native, but is move-only so the host cannot silently
  double-own a buffer. Owners must call `release_to` before the object dies.

## State reached

| Address | Name | State |
| --- | --- | --- |
| 0041dd40 | `BSP_NativeString_Resize` | reconstructed, build-tested, fixture-tested |
| 0041e870 | `BSP_NativeString_Assign` | reconstructed, build-tested, fixture-tested |
| 00438e40 | `BSP_NativeString_Duplicate` | reconstructed, build-tested |
| 00419cc0 | `BSP_SizedStoragePool_GetSingleton` | analyzed |
| 00bd1510 | `BSP_SizedStoragePool_ReturnBlock` | reconstructed, build-tested |
| 00bd1120 | `BSP_SizedStoragePool_AllocateBlock` | reconstructed, build-tested |
| 00bd12a0 | `BSP_SizedStoragePool_ReturnSmallBlock` | reconstructed, build-tested |
| 00bd1780 | `BSP_GameAllocator_Allocate` | reconstructed, build-tested |
| 00bd17a0 | `BSP_GameAllocator_Release` | reconstructed, build-tested |
| 00bd1680 | `BSP_GameAllocator_GetSingleton` | analyzed |
| 00bd11f0 | `BSP_GameAllocator_AllocateBlock` | analyzed, geometry reconstructed as a configuration |
| 00bd1620 | `BSP_GameAllocator_ReturnBlock` | analyzed, geometry reconstructed as a configuration |
| 00be0a30 | `BSP_FileBlock_Construct` | analyzed, string fragment reconstructed |

Nothing here is ABI-compatible or game-validated. The C++ is a semantic projection with new
interfaces.

## Uncertainties and what remains

- `DAT_01090aa4`, the gate that turns small-block returns into silent drops, is only read on the
  release path in both pools. Nothing in this packet establishes who writes it or when; a shutdown or
  low-memory switch are both plausible.
- Pool word +0x4 in the string pool is never written by the constructor and never read by the four
  routines examined. It may belong to the vtable's class or be padding.
- The pool has 2851 release callers against 63 direct allocate callers. Most allocation sites must
  reach the arena through some other inlined or wrapper path that this packet did not chase.
- No teardown was examined. 00bd0c30 registers something at construction and the pools are never
  freed by anything read here.
- Whether the ring cascade can lose a block when every class region is full is not established by
  inspection; the loop terminates when the walk wraps to the starting class and then stores into
  `tail[last]`, which by then may be a slot another class still owns. The reconstruction reproduces
  that behaviour rather than guarding it.
- The 38 (150-class) and 32 (129-class) ring slots past the last region are never addressed by the
  constructor and can only be reached through the cascade.
