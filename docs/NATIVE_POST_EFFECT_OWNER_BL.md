# Native post-effect owner lifetime

The 36-byte (0x24) owners placed in the render-service texture child's auxiliary
slots use profile `D61EC8`, whose terminal route is `BD30E0 -> B4E450 ->
B4E2F0`. They are separate from the `D61948` 2D textures in that child's
loaded-texture slots. The nonzero auxiliary producers are documented in the
child-lifetime packet, including `B52860` and `B529A0`.

| Body | Inclusive native range | Bytes | Source |
| --- | --- | ---: | --- |
| Destructor | B4E2F0..B4E3C8 | 217 | `destroy_native_post_effect_owner_00b4e2f0` |
| Scalar deleting destructor | B4E450..B4E46D | 30 | `delete_native_post_effect_owner_00b4e450` |

The destructor receives its actual owner in ECX and returns without stack
arguments. The deleting wrapper receives one flags DWORD, frees only after
successful destruction when bit zero is set, returns the original allocation
identity, and executes `RET4`. These are new C++ interfaces with explicit
provider contexts, not replacement binary entry points.

## Actual member operations

The destructor stamps `D61EC8`, captures the current stream at `+18`, and then
captures the current `CE2220` InterlockedDecrement target once. That same
stdcall target serves the stream, material and frame-target decrements. It
uses Win32 `long(volatile long*)`, matching the other contexts' exact IAT cell
type; the owned counter remains an existing `atomic<int32_t>` at actual `+04`.

| Order | Member | Concrete operation |
| --- | --- | --- |
| 1 | +18 logical vertex stream | D61D6C, BD30E0, B4BF10; existing canonical `NativeLogicalVertexReference` |
| 2 | +14 material | D5E520, BD30E0, B194B0; existing canonical `NativeMaterialReference` |
| 3 | +0C camera node | Exact node binding and B6DFA0 unlink/release |
| 4 | +10 render-model node | Exact node binding and B6DFA0 unlink/release |
| 5 | +1C raw 28-byte command | Captured allocation through shared CRT free |
| 6 | +08 frame target | D5E600, BD30E0, B1FCF0; existing frame-target/surface/vector/CRT provider |
| 7 | base | BD30F0 restores CEB130 |

A null member skips its clear. A nonnull member is captured, operated on, and
only then is the owner's current field cleared. Callback writes to that same
field are overwritten after return; each later field is freshly read. The
current profile and slot-zero target are inspected only after a zero count,
and BD30E0's deleting slot uses a fresh profile read. Canonical lookup is
nonmutating and never performs a second decrement.

The single native unwind state maps through `CBFB18 -> DF86B0`, map `DF86A8`,
to `CBFB10 -> BD30F0`. The C++ cleanup projection restores only the base
profile. It does not retry a failed member, clean later members, or free the
owner after destruction throws. The inherited canonical terminal is
`noexcept`; that host terminal does not prove native FH3/SEH behavior.

## Construction and companion boundary

Original `B4E840..B4EBF3` is inspected producer evidence only. It sets count
one at B4E86E, stamps D61EC8 at B4E87F, creates the frame target, logical stream,
material, render model, camera and command, and publishes the fields listed
in the report. Its three stack arguments and `RET0C` were checked in assembly;
its pseudocode loses register/argument information. Its x87 operations and
remaining construction dependencies are not reconstructed in this packet.

The new owner/reference companions borrow already-constructed actual storage
and an already-live atomic at `+04`. They never initialize or reset that
counter, add a retain, or copy native storage. A future reconstructed B4E840
must begin the atomic's lifetime at the original count-one write. Merely
having the same four counter bytes does not prove that C++ lifetime contract.

The post-effect reference registers in the same `GuiNativeGeometryRegistration`
and `NativeRenderActualOwners` domain as its parent. The externally stored
companions themselves allocate nothing; the registry's bind implementation
can allocate metadata unless its capacity is prepared separately. Binding is
transactional. Final zero runs the complete deleting destructor, unbinds using
the captured identity without reading freed storage, then notifies host
retirement. Unbind preserves both companions until that last notification.

## Evidence and limits

`reports/native_post_effect_owner_bl.json` records native and source hashes,
eleven live/disk byte comparisons, current profile words, fourteen numeric
call rows, genuine indirect sites and constructor publications. Two listing
holes after returning CRT free were repaired and saved: B4E385..B4E38A
contains both `ADD ESP,4` and the essential `MOV [ESI+1C],EBX`; B4E465..B4E467
contains the deleting wrapper's stack adjustment. Exports were force-refreshed.
The FH3 routing function was defined with its default name and saved.

The source and existing checks have been built; final combined integration is
recorded separately. No new test or differential fixture is claimed here.
Foreign profiles, unrestricted destructive aliasing, concurrent lifetime
mutation, hardware faults, constructor-to-companion composition, original
replacement ABI, native FH3/SEH and gameplay remain outside the current proof.
