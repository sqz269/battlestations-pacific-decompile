# System prefix time constants

`write_system_time_constants_00b46cb4` reconstructs the interior range
`00B46CB4..00B46D96` of `00B46A70`. It patches initialized float storage and
returns the captured fog owner for the following packet. It does not allocate
or clear the system-constant array. The names below are descriptive hypotheses.

## Ordered reads and stores

| Native site | Destination | Recovered source or operation |
|---|---|---|
| `00B46CB4` | `c33.x` | Raw bits of the captured timer's `+4` |
| `00B46CC2` | `c33.y` | Lazy particle singleton getter, then raw `+18` |
| `00B46CDB` | `c33.z` | Foliage group manager `+2C`, x87 load/store |
| `00B46CED` | `c33.w` | Foliage owner `+8`, x87 load/store |
| `00B46D04` | `c34.x` | Reloaded timer virtual `+1C`; two signed-64 loads, divide, float spill |
| `00B46D1A` | `c34.y` | First service `+84` getter's `+24` byte, unsigned conversion |
| `00B46D36` | `c34.z` | Second service `+84` getter's `+28`, raw bits |
| `00B46D64` | `c75.x` | Camera `+174` byte, unsigned conversion |
| `00B46D6D` | `c34.w` | Delayed x87 spill of reciprocal camera `+1C4` |
| `00B46D74` | `c75.y` | Reloaded foliage owner `+11` byte, unsigned conversion |

The camera `+1C4` field is the existing `camera.projection.fov`; no second
field is introduced. Both bytes retain all eight bits, including values other
than zero and one. This packet infers no time units and supplies no clock
defaults or zero-denominator fallback.

The preceding axes packet captures the actual `FrameClock*` at `00B46C89`.
That retained object supplies `c33.x`; the later virtual call independently
reloads `global01090AB0`. Its concrete entry is `00BEE070`, recovered from
`vtable00D68D50+1C`: `8D 41 40 C3`, or `LEA EAX,[ECX+40]; RET`. The new
concrete adapter returns the canonical `FrameClock.interval`. An explicit
virtual adapter is required for an owner with different dispatch.

Several global captures occur between a source load and a destination store:

- `00B46CCC` captures the foliage group manager after loading particle time
  and before writing `c33.y`.
- `00B46D0B` captures the first render service after the second signed-64
  `FILD`, before `FDIVP` and the `c34.x` spill.
- `00B46D27` independently captures that service before the `c34.y` store.
- `00B46D51` captures the final foliage owner before the `c34.z`, `c75.x`,
  and reciprocal stores.
- `00B46D79` captures camera `+184` after the byte getter and before `c75.y`.

Inline assembly retains these interleaves and the pending x87 reciprocal.
`timestamp_seconds_x87` already implements the same arithmetic, but calling
its float-return interface would move the renderer capture across its spill.
The fragment therefore uses its exact `FILD64/FILD64/FDIVP/FSTP32` sequence
in place. No double conversion or premature single-precision division is used.

`SystemRenderTimeOwner` is the same retained `global00F8D39C` owner used by
the earlier camera-matrix packet: it contains references to the embedded
`+84` region and the actual `+1D8` matrix. The two `00B0CF30` calls return
that owner's embedded region; they are not calls to a generic read callback.
`SystemFoliageTimeOwner` similarly borrows the actual `+8` and `+11` storage.
The `+2C` owner is the existing `FoliageGroupManager`, and the particle owner
is the existing `ParticleClock` consumed by `set_particle_clock_time_00b19a10`.

## Particle singleton lifecycle

`particle_clock_singleton_004de4b0` implements the native getter's complete
local control flow. Its populated fast path returns the first slot load without
calling the lifetime manager. On the null path it obtains the actual manager,
captures its optional `+10` critical section, enters that section and increments
the section's `+18` counter. It then rechecks the singleton slot.

If still absent, allocation requests exactly `0x1C` native bytes. A nonnull
owner receives these ordered initialization stores:

1. Secondary vtable `+4 = 00CE7D08`.
2. Record storage/count and words `+8..+14` become zero.
3. Primary vtable `+0 = 00CE7D38`.
4. Secondary vtable `+4 = 00CE7D24`.

There is no store to `+18`. The allocator adapter must preserve the actual
allocation's `+18` bit pattern in the canonical `ParticleClock.shader_time`.
Default/value construction of that projection does not establish the native
preimage. The volatile construction stores retain the otherwise overwritten
base-secondary-vtable write.

The new pointer, including null on allocation failure, is then published to
`global00F8D420`. A second manager lookup occurs before the singleton slot is
reloaded for `00BD0C30`. Registration is invoked even for null; the actual
registration routine ignores null and retains nonnull objects for LIFO
destruction. The originally captured critical section is decremented and left
before the final singleton slot reload. Publication during entry, and slot
replacement during registration, retain their observed effect on the result.

`ParticleClockLifetimeAccess` requires actual manager, allocation,
registration and Win32 critical-section adapters. Those dependencies have no
default implementation. It does not pretend to reconstruct the allocator,
lifetime-manager allocation/destruction, pointer-vector growth or the particle
destructor. C++ scope cleanup covers throwing adapter calls after successful
entry; native SEH handler/context identity is outside this interface.

## ABI and validation boundary

Native `004DE4B0` takes no inputs and returns the pointer in EAX with plain RET.
The five leaf getters use ECX for the owner and plain RET. `00AF0460` and
`00AD5700` return an x87 value; the new store interfaces combine each getter
with its caller's float spill. `00AD5740` returns AL; `00B0CF30` and `00BEE070`
return EAX pointers. The original interior fragment uses EDI for the camera,
ECX for the captured timer, stack-relative output, and ESI for the fog capture.
The public C++ functions and retained reference projections are new interfaces,
not binary-compatible replacements or native-layout objects.

The prefix function requires valid owners, callbacks and at least 302
initialized float words. Invalid output capacity throws a host-side
`invalid_argument` before any work. Valid inputs preserve untouched words,
including `c75.zw`. Volatile slot references preserve ordered reloads; they do
not supply object lifetimes or C++ synchronization for concurrent mutation.

Strict MSVC Win32 compilation passed with `/W4 /WX /permissive- /fp:strict /O2`.
The worker used temporary copies of the primary integrator's shared camera and
particle headers; their exact hashes are in the audit. The repository's
`scripts/build.ps1` and existing `reconstructed_math` test passed. Eight seed
ranges matched the saved Ghidra project and installed executable.

One local native differential fixture executed the original 227-byte fragment
and 199-byte lazy singleton getter from isolated executable storage, relocating
their global slots and external calls to controlled actual-field adapters. Both
byte ranges separately matched the installed executable. Sixty prefix/input
combinations compared all 308 output words, returned fog identity and x87 sticky
exception flags across the three defined precision settings and four rounding
modes. Inputs included signed-64 extremes, zero denominators, signed zero,
subnormals, infinities, signaling NaNs and full-byte 255. Four singleton paths
compared callback order, captured-lock counters, allocation preimage and final
pointer identity: allocation, publication during entry, null allocation, and
replacement during registration. Populated fast paths were also checked.

The native fixture uses masked floating-point exceptions and controlled
external adapters. It does not prove exception-context compatibility,
multithreaded publication, manager teardown or game behavior. No game execution
or visual validation is claimed. Shared-header integration and Ghidra/ledger
annotation belong to the primary integrator; proposed names and preimages are
recorded in `reports/system_time_constants_audit.json`.
