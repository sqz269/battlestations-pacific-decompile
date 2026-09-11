# Native renderer texture binding

`native_renderer_texture_binding3` reconstructs all 297 bytes of `B24710`
and the two four-byte cube/volume getter leaves at `B3CF90` and `B3D0C0`.
The implementation uses actual borrowed renderer, logical-owner, reference,
profile, notification, pool and D3D9 storage. It exposes new MSVC Win32 C++
interfaces; it is not a game ABI replacement or a game rendering validation.

## Native contract and evidence

| Entry | Native ABI | Complete range |
| --- | --- | --- |
| `B24710` | ECX renderer; stack sampler and incoming owner; RET8; no semantic result | `B24710..B24839` |
| `B3CF90` | ECX cube owner; EAX borrowed COM; RET | `B3CF90..B3CF94` |
| `B3D0C0` | ECX volume owner; EAX borrowed COM; RET | `B3D0C0..B3D0C4` |

Fresh guarded Ghidra capture verifies project `bsp`, program
`/battlestationspacific.exe`, image base `00400000`, against the installed PE.
The 41 captured spans contain 1,834 bytes. All 305 owned bytes match the PE
and decode completely. Both previously undefined getter leaves have bytes
`8B 41 10 C3`; their missing Ghidra function metadata was retained in the
capture. Names describe established behavior rather than recovered symbols.
The existing 2D getter `B3CEA0` is reused unchanged.

The eight-DWORD profiles are `D61948` (2D), `D61870` (cube) and `D618B0`
(volume). All have virtual0 `BD30E0`; slot4 selects full scalar deletion at
`B3F590`, `B3F410` or `B3F430`. Slot1C selects the corresponding four-byte
COM getter. Host profiles contain immutable original entry tokens. Source
dispatches established source providers; it never calls those tokens as
host addresses.

## Operation order

1. Read current synchronization mode. When enabled, save the receiver and
   call full optional entry `B33AD0` before reading the bound slot.
2. Compare incoming against renderer at DWORD-wrapped
   `sampler * ACh + 504h`, then arm native cleanup state0.
3. On outer change, reread and capture the current old owner. If the inner
   comparison still differs, publish incoming, increment its actual +4
   reference count, then decrement old. A zero old count reads current
   virtual0 and `BD30E0` independently rereads current slot4. The complete
   actual 2D, cube or volume lifetime provider receives scalar flags1.
4. Map unsigned samplers below16 directly; otherwise add F1h modulo2^32.
   After old lifetime work, the original incoming pointer reads its current
   profile/getter. Null input produces null COM without a getter.
5. After the getter returns, reload renderer device+1A10 and the device's
   current vtable slot104. Call actual `IDirect3DDevice9::SetTexture`.
6. Only a changed path whose COM call returns reads current counter+1BC4
   and writes counter+1 modulo2^32. There is no HRESULT gate or rollback.
7. Read current exit mode and disarm before normal `B33B00` leave. The
   cleanup-only exceptional path calls full `B21110`.

The native EH handler is `CBCFF8`, its complete FuncInfo at `DF5668` has one
cleanup state, no catches and EH flags1. Unwind map `DF5660` contains
`[-1, CBCFF0]`; that funclet obtains saved guard `[EBP-14h]` and jumps to
`B21110`. Source preserves the full optional guard and terminates a second
C++ exception during cleanup. Entry failure occurs before arming; normal
leave is already disarmed and cannot be retried.

## Actual context

`NativeRendererTextureBindingContext` borrows synchronization and the full
2D/cube/volume owner contexts plus the three immutable profile views.
Owner contexts share actual renderer publication, synchronization and string
allocation domains. The 2D provider additionally retains its actual surface
pool, support singleton and lifetime domain; all owner pools are initialized
canonical storage supplied by those providers. No hidden pool, renderer,
shadow reference count, semantic cache or callback terminal substitutes for
these services.

Every reached raw address must remain valid, including wrapped sampler
slots and current profile/COM accesses. Arbitrary profile words are outside
the established domain. Disabled entry followed by enabled exit is outside
the native guard's valid domain because its local receiver/AL were never
initialized. No bounds checks, HRESULT repair or reference rollback is added.

## Bounded verification

The strict Win32 Release build uses `/W4 /WX /fp:strict`; the existing math
and native differential CTests pass after eight verified seed byte matches.
The owned translation unit is frozen in `bsp_core.linked.lib` with SHA-256
`b55000000f046bac385cb17adc858c2525c36bd66b6726caa848609920a9e3e7`.
It is never recompiled with fixture defines or replaced for observation.

The base checkout contained the volume owner source before its shared CMake
registration. The fixture therefore also links a byte-identical copy of the
previously frozen volume library, SHA-256
`0b6711e15330c9a369ba2d4f73a609ef46ab35d5a516b39b99e3c76a6be25f75`.
MAP evidence shows only `native_volume_texture_owner.obj` comes from that
companion; all other reached owner/guard/pool providers and owned code come
from the binding library. This is a composed fixture, not evidence that the
base CMake target alone linked a complete binding client.

The ignored fixture privately maps all captured native spans. Original
`B24710`, all three getters, `BD30E0`, guard enter/leave/unwind and native
FH3 data execute from that mapping. The sole owned-byte adjustment is the
four-byte EH registration operand at `B24719`, pointing to a host jump-only
trampoline into unchanged `CBCFF8`. Three declared old scalar entry jumps
compose the full frozen owner providers. One runtime FH3 jump and four
Win32 IAT bindings complete the declared bridges. The old original provider
bodies are pinned evidence; their bridged scalar entries do not constitute
an independent native-versus-source owner comparison in this fixture.

Actual owner pools, retained stream/backing storage, notification record and
name aliases are nonempty. The 2D case also uses an actual texture registry,
one cached actual surface owner, a real `GetSurfaceLevel` result, support
singleton and both tracking counters. Canonical raw-slot reuse is checked
after successful scalar deletion. Incoming owners use isolated real raw
storage with their actual intrusive +4 counts and real COM pointers; their
constructor/whole-owner lifetime is not under comparison.

Two hidden real HAL devices and actual 2D/cube/volume textures are used.
Copied actual COM tables replace only forwarding reference/SetTexture
observers. Every observed reference call forwards its actual method; every
SetTexture forwards exactly once, and successful calls verify `GetTexture`
readback. Extra actual AddRefs keep observation receivers alive. All 168
ordinary-run resources, both devices and the D3D factory clean up to zero.
Recorded module identities, relocated method bytes and original resource
table words match the installed x86 `d3d9.dll`. The actual devices use runtime
allocated tables; all 119 words are recorded, with the reached SetTexture
and GetTexture targets verified against their original module bytes.

PAGE_NOACCESS plus instruction single-step observes actual renderer and
incoming reads/writes. It leaves executable bodies unchanged. Explicit
mutations are recorded separately. Original/source traces match 16,068
DWORDs in 206 event frames; 38 writes per path match in order and value.
There are 28 call records per path: nine getter loads, nine actual owner COM
reference calls and ten SetTexture calls. MAP/call-site evidence identifies
the corresponding native, owned-source or composed-provider instructions.

| Scenario | Compared behavior |
| --- | --- |
| 0, 1 | Null identity and guard callback changing outer identity; entry/leave still occur |
| 2 | Nonempty 2D full lifetime, registry/cache/surface cleanup, new cube getter, counter wrap |
| 3 | Nonempty cube full lifetime; old COM release changes incoming to current volume profile/COM; sampler15 |
| 4 | Nonempty volume full lifetime and diagnostic COM pair; new 2D getter; sampler16 maps257 |
| 5 | Disabled guard, null replacement, old count2-to1, real null SetTexture |
| 6 | Sampler FFFFFF0F wraps to device stage0; valid surrounding raw slot; current counter mutation |
| 7 | Mutation after actual outer read makes inner owner equal incoming; references skipped, binding retained |
| 8 | Actual getter EAX retained while callback changes device, current device table and incoming COM field |
| 9 | Throw after real SetTexture: published/retained incoming survives, counter skipped, full guard cleanup |
| 10 | Throw after actual old cube Release: full old base cleanup, no getter/SetTexture/counter or pool return |
| 11 | Normal leave throws after actual OS leave; counter already changed, no leave retry |
| 12 | Entry throws after actual OS entry; cleanup not armed and texture work not started |
| 13 | Current mode disabled by SetTexture callback skips normal leave |

Separate original/source processes test a SetTexture exception followed by
a leave exception. Both terminate with exit91 and identical terminal state:
incoming remains published, old/new refs1/6, unchanged counter, one COM set,
one enter/leave and zero nesting/tracked depth/OS recursion.

Returning HRESULT failure was not exercised by the final actual-driver
fixture; its ignored status follows the complete instruction/source audit.
No arbitrary invalid sampler memory, concurrent mutation, unsupported profile,
real allocation failure, full renderer integration, ABI replacement or game
visual result is claimed. The tests cover explicit callback-boundary changes,
not unsynchronized data races. Permanent tests and shared metadata are left
to the primary integrator's existing workflow.

`reports/native_renderer_texture_binding_audit.json` records the capture,
function boundaries, original EH proof, declared bridges, library/MAP and
runtime byte checks, scenario observations and immutable artifact hashes.
Reproduce the frozen fixture from a fresh output directory by invoking the
worker's `build/texture-binding-check/texture_binding_check.exe` with the
installed game executable path. Use optional `native-double` or `host-double`
only in separate output directories; those intentionally exit91.
