# Actual renderer resource restoration

The complete 319-byte `B23B10..B23C4F` body is reconstructed in
`src/native_renderer_resource_restore.cpp`. Its strict MSVC Win32 build and
both existing checks pass. The focused original-code comparison is **pending**;
this packet is not yet closed in the reconstruction ledger.

The original takes the renderer in ECX, has no stack arguments or own exception
handler, and returns with plain RET without a stable semantic EAX result.
The new C++ interface borrows the actual renderer, original profile views and
the address of the current renderer publication cell. It is not a binary ABI
replacement. Reached owner and COM storage must be valid at the native reads.

The two current bytes `+1D8B` and `+1D8A` gate the work. The first device is
captured before ready is set to one. Color target zero is reacquired and bound
to the current `+197C` wrapper, then depth to the current `+198C` wrapper. The
same local output is zeroed before each acquisition, reread after each bind,
and unconditionally dereferenced for Release. HRESULTs are ignored, and throws
leave all earlier writes and references in place.

Texture `+1B00/+1B04` and surface `+1B0C/+1B10` loops compare raw DWORD
addresses, retaining the old cursor across callbacks. They read current count,
then current base, advance the old cursor and compare it with the new end.
They do not rebase or apply signed-count guards. Query `+19A0/+19A4` uses a
signed index/count comparison and reloads the table before each owner.

| Actual profile | Reached slot | Complete provider |
| --- | --- | --- |
| `D619A0` surface | `+14` / `+40` | `B3CC80` bind / `B3D550` recreate |
| `D61948` texture 2D | `+24` | `B3DD90` restore |
| `D61870` cube / `D618B0` volume | `+24` | Genuine `B33F20` RET4 |
| `D62AD0` query | `+1C` | `B5FE60`, using current publication and `B1FEF0` |

Original table words select these full source providers at the native read
points; numeric game code addresses are never called as host pointers. Texture
callbacks receive reused local scratch cells eight bytes apart, with the device
argument seeded before each callback. The child provider seeds its surface
cell with its captured owner. This source interface does not reconstruct
unrelated caller-stack words or the native return-address layout.

Fresh Ghidra and installed-PE checks cover 14 code/data spans, 1,120 bytes,
including the full body, complete reached code providers, profile words and
the texture pool jump table. See `reports/native_renderer_resource_restore_audit.json`.
The full reset cycle, device recreation, drawing and gameplay remain unverified.
