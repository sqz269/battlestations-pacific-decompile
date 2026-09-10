# Concrete system fog owner

`SystemFogOwner` supplies the constructor, field setters and counted lifetime
of the native `94h`-byte object published at camera+`184h`. Its
`fields_08` member is the existing `SystemFogState`, at native offset `08h`;
the camera's ambient color and system fog constants borrow that same member.
There is no copied state, owner registry, separate ambient object or synthetic
lighting source. Fog/AmbientLight remain semantic hypotheses.

This packet implements the core bounds in `SYSTEM_FOG_OWNER_NEXT.md`. It
does not construct the surrounding camera, world or environment or supply
authored directional colors. Interfaces are new C++ APIs restricted to MSVC
Win32, not native calling-convention replacements.

## Storage and initialization

The concrete standard-layout type contains the original vtable-address word,
the interlocked reference count and the actual field projection. Compile-time
checks enforce `sizeof(SystemFogOwner)==94h`, count offset `04h`, field offset
`08h` and the retained field layout. The vtable-address word is volatile so
the owner/base transitions remain actual writes; it is not a host function
pointer.

`allocate_system_fog_owner()` calls the existing real
`singleton_lifetime_allocate({object,94h,sizeof(SystemFogOwner)})` host CRT
boundary. The same in-place initializer is also callable with aligned supplied
backing storage. It captures allocation bytes `[28h,68h)` before placement
construction and restores them into `fields_08.directional_colors_28`.
The projection is also at `28h` in the checked host layout, so the native
allocation preimage and eventual consumed records refer to the same bytes.

The native constructor `[00B84E50,00B84F61)` has no calls: ECX is storage,
EAX returns the same address, plain `RET`. The initializer sets both color
float4 records to positive-zero words, refcount to 1 and vtable to `00D63180`
after the base `00CEB130` transition. Scalar fields receive the exact native
constant words:

| Offset | Bits | Float32 display |
| --- | --- | ---: |
| 68 | 3ECCCCCD | 0.4 |
| 6C | 43480000 | 200 |
| 70 | 45DAC000 | 7000 |
| 74 | 43480000 | 200 |
| 78 | 3ECCCCCD | 0.4 |
| 7C | C2F00000 | -120 |
| 80 | 44480000 | 800 |
| 84 | 3F7851EC | 0.97 |
| 88 | C3480000 | -200 |
| 8C | C30C0000 | -140 |
| 90 | 3F7EB852 | 0.995 |

The native constructor does **not** initialize the four directional records.
Fresh allocation therefore does not establish their values or make the full
owner ready for a rendering assertion. The in-place API can preserve an
explicit initialized preimage for comparison; it never invents zeros or reads
those bytes through a floating-point operation. The environment producer is
a separate integration dependency.

## Fields and ownership

The color setters `00B84C40`, `00B84C70` and directional setter `00B84FA0`
retain four forward integer read/store pairs, including propagating overlap
and signaling-NaN bits. The directional index uses the original unchecked
32-bit `28h+16*index` arithmetic; valid resulting storage is a caller contract.
The eleven scalar setters use raw `uint32_t` float bits in the new C++ API.
They reproduce the native `MOVSS` data copy without an accidental float
argument conversion. The environment caller's preceding `FLD/FSTP` is outside
these leaves.

`system_fog_owner_from_state` maps a live `fields_08` view back to the same
containing object using its checked offset. Every nonnull slot passed to a
retaining setter or release fragment must originate from this concrete owned
profile. A standalone diagnostic `SystemFogState` remains a valid borrowed
shader input but must never be retained or released through this owner API.

`set_system_fog_camera_owner_00b71940(camera.fog_184, owner)` and the receiver
`+10h` counterpart at `00BBDF20` operate on the actual projection slot. The
second receiver has vtable `D64518` and occupies `game+19E8`; it is distinct
from the world at `game+19CC`. The existing C++ `world_owner` name is historical,
not a recovered class name. See `SYSTEM_FOG_WORLD_FACTORY_NEXT.md`. Both setters
compare old/new identity, publish new, increment new, then decrement old and
destroy it if the count reaches zero. A creator can release its initial
reference after the camera/receiver binding acquires one. The native method ABI
is ECX camera/receiver, one stack owner pointer, `RET4`.

`initialize_system_fog_camera_slot_00b71ae3` implements only the fresh camera
constructor null store. `clear_system_fog_camera_slot_00b71f68` implements
`[00B71F68,00B71F8A)`: decrement/release the old owner while its view remains
published, then clear the slot after destruction/free returns. A null slot
skips the store. Clearing is deliberately not implemented by the retaining
setter, whose publication order differs.

On zero references the proven concrete vtable profile resolves as
`00D63180[0] -> 00BD30E0 -> 00D63180[1] -> 00B84F70(flag1)`. The host uses
that actual bounded deleting-destructor implementation directly; it does not
introduce abstract or placeholder callbacks. The invoker accepts null and
does not itself decrement. Arbitrary subclasses and foreign vtables are
outside this concrete contract.

The deleting destructor writes the owner vtable, executes the bounded
`00BD30F0` base-vtable effect, and uses `singleton_lifetime_free` when flags
bit 0 is set. The normal path returns the original address even when freed.
Flags 0 leaves the bytes allocated, with the base vtable installed; the owner
is logically destroyed and must not be retained/released again. The native
`ADD ESP,4` at `00B84F8B`, omitted from Ghidra's listing after `_free`, was
retained as returning behavior. No nested fields need separate destruction.

## Verification and remaining integration

The focused MSVC Win32 comparison links the actual new implementation and
existing `src/singleton_lifetime.cpp`. One sequence compares original code
with the translation: constructor preimages, all 14 setter effects with
signaling-NaN words and one-word forward overlap, flags-0 bytes/return,
self-assignment, replacing a sole retained old owner, then clearing the last
camera reference. It also exercises the real allocation API and world slot.
All comparisons passed.

The fixture maps the installed PE into its own process; it does not launch
the game or execute its entrypoint. Because the binary has no relocation
table and its preferred range was occupied, only 19 decoded absolute image
operands/vtable entries are rebased. Relative branches stay unchanged. The
two atomic imports use the actual Windows functions, the selected camera
fragment continuation becomes `RET`, and both native/host free boundaries
are intercepted solely to record visibility before calling the real
`std::free`. These are fixture modifications, not production callbacks.
Original and fixture bytes for every patch are recorded in the audit.

At old-owner free, both paths expose the new published slot with new count 2.
At camera-clear free, both retain the old published slot and count 0 until
free returns. Both expose the base vtable and old count 0. Compiled assembly
also confirms the publication store precedes `LOCK INC`, and the clear store
follows the actual free call. Native vtable addresses are normalized only for
comparison because the fixture relocates them; host addresses remain native
evidence words.

The required `scripts/build.ps1` passed after updating to integrated base
`29109d2`; both existing CTest checks passed. This four-file packet leaves
shared CMake registration to the integrator, so that repository build covers
the integrated base while the new unit is separately compiled and executed
by the focused check. The earlier inherited `ParticleClock::owned_records`
build mismatch was resolved by that base update.

The audit lists each complete function and interior range, exact native
hashes, interface differences, fixture patches and artifact hashes. Existing
getters, shared camera headers, CMake, Ghidra annotations and ledgers were
not edited by this packet. Full camera/world/environment wiring, allocation
failure and native EH behavior, arbitrary concurrent mutation, native ABI
compatibility and gameplay/render validation remain unclaimed.
