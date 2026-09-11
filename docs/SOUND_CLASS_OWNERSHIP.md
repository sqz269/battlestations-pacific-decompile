# Sound descriptor ownership and pointer-table storage

Addresses: 00a7c350 00a7c3f0 00a7c460 00a7bbe0 00a7c2c0

Packet `orch3_sound_class_ownership`, branch
`agent/orch3-sound-classes-20260910b`. Descriptive names are hypotheses, not
recovered symbols. Verified Ghidra reads used `bsp.py ghidra` against project
`bsp`, `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, with the
client checking project/program, language and image base before each live query.
This worker made no Ghidra mutations. The parent repaired two false no-return
flow gaps under its write lock; refreshed exports were inspected afterwards.

## Concrete boundary

`SoundClassDescriptor` owns the native string through the existing
`NativeStringStorage` boundary and exposes the existing `SoundClassLevel` as its
base object. The level routines therefore read/write the same index, volume and
name view that setup initializes. `SoundClassOwnership` implements
`SoundLevelTableHost` and uses `SoundManagerLevels::classes_98` as its canonical
table. It keeps one reference per nonnull slot and a separate logical capacity.

Growth creates **null slots**, not descriptors. Consequently an out-of-range
call to `set_sound_class_level_00a7f8e0` remains invalid even with this host: the
native immediately dereferences the new null slot. Registered, nonnull indices
remain a precondition. This resolves the former unknown allocation/initialization
boundary in `SOUND_MANAGER_LEVELS.md` without inventing missing-category recovery.

## Descriptor construction and destruction

`00A7C350` takes the allocated descriptor in ECX, returns it in EAX, and ends at
`00A7C3E1` with plain RET. Setup allocates `20h` bytes separately at `00A80D39`,
then calls the constructor at `00A80D56`.

| Native field | Construction evidence |
| --- | --- |
| `+0` | Base vtable `00CEB130`, then descriptor vtable `00D5AC3C` |
| `+4` | Reference count 1 at `00A7C37A` |
| `+8` | Class index 0 at `00A7C3AE` |
| `+C` | Single byte 0 at `00A7C3AB`; padding bytes are not initialized |
| `+10/+14` | Both 1.0 from `00D7A24C`, bytes `00 00 80 3F` |
| `+18/+1C` | Empty native string length/pointer at `00A7C38C/38E` |

The zero-length resize at `00A7C3B1` sees an already zero-length header and
returns without allocation. The following guarded memcpy cannot run on the
normal path. The meaning of the byte `+C` and float `+14` remains open here.

`00A7C3F0` takes ECX descriptor and has plain RET at `00A7C450`. It captures the
name pointer at `00A7C40D`; if nonnull, it returns exactly `length+1` bytes through
the sized pool at `00A7C42D`. It then calls existing refcount-base destruction
`00BD30F0`. It does not clear the native string header before the storage callback.
The projection reuses `destroy_native_string_header_0041dd20` for this established
same header-release operation; it does not duplicate the pool implementation.

Vtable bytes `00D5AC3C` identify `00BD30E0` in slot 0 and the existing compiler
helper `CG_scalar_deleting_dtor_00a7c460` in slot 4. Existing `00BD30E0` invokes
slot 4 with flag 1. `00A7C460` calls descriptor destruction, frees the descriptor
only when flags bit 0 is set, then returns the original pointer. Its ABI is ECX
descriptor plus stack flag byte, RET 4 at `00A7C47B` (inclusive `00A7C47D`). The
helper is analyzed only; C++ final `release()` uses `delete this` and does not
expose the native flags/return-pointer ABI.

## Pointer reserve and resize

Both helpers take ECX pointing at the three-dword header at manager `+98`: pointer,
count, capacity (`+98/+9C/+A0`). Both take one signed 32-bit stack argument and
return with RET 4.

`00A7BBE0` reserves capacity, complete through RET 4 at `00A7BCD1` (inclusive
`00A7BCD3`). Clamp requested capacity to at least 1. If it exceeds current
capacity, allocate `4*requested` bytes. Copy all currently counted slots in forward
order; each nonnull copied pointer receives an interlocked increment. Only after
all copies are retained does a second forward pass decrement each old nonnull
pointer, call vslot 0 if the decrement returns zero, then clear that old slot.
Free the old pointer allocation, install the new pointer, and store requested
capacity. Count remains unchanged. Slots beyond count are not constructed or
initialized by reserve.

The old export incorrectly stopped its allocating branch at `_free` call
`00A7BCAA`. Live bytes and installed-image decoding established the missing
`00A7BCAF..00A7BCC1` instructions, including pointer store `00A7BCBC` and capacity
store `00A7BCBE`. The parent's flow repair restored these to the export. The
deleting helper likewise omitted `ADD ESP,4` at `00A7C475..477` after its free;
its repaired export now returns the original pointer on both branches.

`00A7C2C0` resizes count, complete through RET 4 at `00A7C33B` (inclusive
`00A7C33D`). Reserve exactly the requested count if capacity is insufficient.
For growing count, initialize newly counted slots to null. For shrinking count,
decrement the visible count first, then release and clear that removed slot;
repeat in reverse order. Finally store requested count. Capacity is retained.
No call to descriptor construction occurs in either table helper.

## Setup connection

The separate setup packet owns the full `00A7FF80` body. Its inspected Categories
fragment allocates/constructs a descriptor, populates its name and `+10` volume,
takes a temporary reference, writes descriptor `+8` from table count, then appends.
At `00A80EBA..00A80EF5`, capacity is doubled only when count equals capacity,
with minimum new capacity 1; the inserted descriptor gets a table reference before
count increments. Two subsequent decrements drop the temporary and creator
references, leaving one table reference. Thus setup assigns ordinal==class index
at insertion; the later level functions do not themselves enforce that relation.

`append_retained` is a supporting C++ operation for that insertion fragment, not
a claimed additional whole native function. The setup worker owns fragment
reconstruction/ledger entries and remains responsible for name/volume/index
initialization and temporary-reference scope. The factory returns creator ref1;
append retains another; releasing the creator leaves table ref1. Independently
retained descriptors survive table shrink until their last release.

## Scope and validation

These are C++ behavior interfaces, not native layouts or drop-in Win32 ABIs.
The native vtable writes are represented by C++ type/destruction, reference count
uses sequentially consistent `std::atomic`, and library allocation uses actual
C++ allocation/vector ownership. The table's logical capacity is exact even if
the C++ library reserves more physical storage. No CRT/STL implementation is
reconstructed. Native null-return allocation failures, integer overflow, SEH,
reentrant structural table mutation and corrupted reference counts are outside
the valid-input boundary. Calls must be serialized. The manager must outlive its
owner; storage must outlive all descriptors; callers cannot structurally mutate
the bound vector or insert unrelated `SoundClassLevel` objects.

Host-owner destruction shrinks the table to zero; it is not a reconstruction of
the full sound-manager destructor. Entry references that outlive the table need
their own retained ownership. Actual native entry-to-descriptor attachment and
the meanings of descriptor `+C/+14` remain future work.

`scripts/build.ps1` passed Release MSVC Win32 and both existing CTests after all
8 seed byte comparisons matched the installed image. An ignored local ownership
probe passed constructor defaults, growth capacity, reserve reference stability,
null-slot growth, existing level setter/dirty propagation, reverse shrink with
visible count updated, delayed final release and exact string allocation sizes.
No permanent tests were added. These checks provide compilation and host fixture
evidence, not native audio differential, ABI compatibility, playback or game
validation. Because another orchestrator owns `cmake/startup.cmake`, validation
uses ignored `local/extra-sources.cmake` through `CMAKE_PROJECT_TOP_LEVEL_INCLUDES`;
the integrator must add `src/sound_class_ownership.cpp` to its build registration.
