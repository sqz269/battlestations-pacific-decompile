# Dynamic buffer reset readiness

The dynamic vertex/index owners at renderer `+1974h` / `+1978h` have a
separate readiness byte at `+1d8ch`. Scheduler `00b2abd0` releases them before
general resources and restores them after default/offscreen resources. This
pair closes that buffer phase without replacing the device-reset scheduler.

## Release `00b237d0`

Native ABI is renderer in ECX, no stack arguments, ordinary `RET`. The complete
routine enters the optional renderer guard before testing readiness. If ready
is zero it leaves without releasing either buffer. Otherwise it writes ready
zero before any COM operation, processes the vertex wrapper first, then the
index wrapper, and leaves the guard. There is no lost-state gate.

For each wrapper, native loads COM pointer `+28h`, and if nonnull calls AddRef
then Release as a balanced access pair. It reloads the wrapper member afterward,
releases the owned reference if still nonnull, and stores null. It does not
destroy wrappers, remove logical stream registrations, rewind cursors, change
flags/capacity/lock counters, or clear other metadata.

`release_dynamic_buffers_for_reset_00b237d0_fragment` implements this body
phase in `src/d3d9_buffers.cpp`, including its readiness test and early write.
The fragment deliberately does not claim the outer guard: that interface is
private to `D3D9StateCache`. The integrated method
`D3D9StateCache::release_dynamic_buffers_00b237d0` acquires the existing guard
before invoking the fragment, including when readiness causes the phase to
skip. Direct fragment callers must supply that coordination explicitly.

## Restore `00b1fd90`

Native ABI is likewise renderer in ECX, no stack arguments, ordinary `RET`.
This routine has no internal guard. It returns if ready is nonzero or lost
byte `+1d8ah` is nonzero. Otherwise it stores ready one **before** invoking
vertex wrapper virtual `+20h(device)`, followed by index wrapper virtual
`+20h(device)`. The callback identities were verified from the native vtables:

| Owner | Vtable entry address | Target |
|---|---|---|
| Vertex | `00d61e9c` | `00b492b0` |
| Index | `00d61e78` | `00b49180` |

The existing typed recreation functions implement these targets, so
`restore_dynamic_buffers_00b1fd90` composes them in the same order. Index
recreation still runs when vertex recreation reports failure. Ready remains
true after either failure, matching the native early transition and absence
of a rollback branch. Native ignores both HRESULTs; the typed function returns
`S_FALSE` on a ready/lost skip, otherwise the first failed HRESULT or the final
index result. This return status is an explicit new interface feature.

The typed API receives stable vertex/index wrapper references and a device
reference. Native reloads the index owner and device after the vertex virtual
call; owner/device replacement during callbacks is outside this typed API's
contract. No fake renderer layout or unresolved virtual implementation was
introduced. The existing recreation helpers retain their documented failure
behavior, including preserving prior owned buffers on API failure; the normal
reset path enters with null COM pointers following release.

## Ownership and validation boundary

The release phase intentionally differs from `release_dynamic_buffers`, whose
older direct-COM owner helper destroys a temporary buffer pair in reverse
order. Reset keeps the physical wrappers and logical stream relationships
alive. The metadata survives release and can be used by the existing recreate
functions. Releasing a buffer still bound on the device does not alone prove
it is reset-safe: general resource unbinding and all other DEFAULT-pool owners
remain part of the surrounding scheduler lifecycle.

The existing real-device reset sequence now releases the typed dynamic buffers
before defaults and registered surfaces, then restores them after those surfaces.
It checks lost/ready restore skips, repeated guarded release, preserved upload
cursors, recreated 16 MiB vertex/1 MiB index buffers, pool/usage/format and
balanced guard state. The Win32 build, both existing CTests and full installed
asset probe passed; see `reports/dynamic_buffer_reset_probe.txt`. No test target
or framework was added. The probe exercises ordinary real-device behavior;
failure propagation and native ABI compatibility were not runtime validated.

Each live verification batch checked project `bsp`, program
`/battlestationspacific.exe`, x86 image base `00400000`. Complete original
bodies matched installed PE bytes:

| Range | Bytes | SHA-256 |
|---|---:|---|
| `00b237d0..00b238c2` | 243 | `92b213ddab7130d8ac41cc441d12d7392b628d8b8c68202db471f69bf9fad71c` |
| `00b1fd90..00b1fdd5` | 70 | `bbd6b07b5ee4ea6764bab28df153cad3383743b11aad75deee1eb67e6a64fa81` |

Both vtable words also matched the installed image. Parent integration updated
the address ledger and Ghidra evidence comments, preserved previous annotations,
saved the project and refreshed both function exports.
