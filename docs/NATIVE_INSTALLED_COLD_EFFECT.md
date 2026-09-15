# Installed effect admission with cold texture loading

The reconstructed `B2EBB0` path now admits the installed `debugshader.shfx`
with an initially empty texture cache and no precreated error/white texture
owners. Both `.tga` requests resolve through the native search registrations to
installed `.dds` files. Actual VFS reads, retained-memory conversion, D3DX 2D
creation, native owner construction and cache publication complete successfully.

This extends [canonical effect admission](NATIVE_INSTALLED_EFFECT_ADMISSION.md).
The production sources and root libraries are unchanged. The focused Win32
fixture adds actual stream-type initialization and binds the existing native
texture pool and name resolver. Its D3DX function pointer uses
`D3DXCreateTextureFromFileInMemoryEx`, matching the original import at `C2DFE6`.

## Observed result

| Installed input | Retained bytes | Actual HAL texture | Native references |
| --- | ---: | --- | ---: |
| `models/textures/error.dds` | 22,000 | 128 x 128, 8 mips, DXT5 | 3 |
| `effects/white.dds` | 11,064 | 128 x 128, 8 mips, DXT1 | 5 |

Each texture's actual `+4C` memory stream has one reference and a backing with
one reference. The full retained bytes equal the installed file exactly; the
streams remain at their initial cursor. Global backing count is two, requested
bytes and texture-cache accounting are both 33,064. Each cache row holds its
resolved DDS path and original TGA request as two ordered aliases.

The error texture's count of three follows native lazy fallback initialization.
`B31C20` enters `B31BD0`, which sets the guard before resolving/loading the
fallback with `acquire_new=1`. Creation supplies count one, that nested acquire
adds one, and the outer effect request's resolved cache hit adds another. The
registry fallback and effect fallback identify the same owner. White's first
load supplies one reference and four later pass acquisitions bring it to five.

The installed effect still produces primary modes 0, 1, 3 and 4, secondary mode
0 and one base-retained primary. All eight real HAL shader `GetFunction` results
equal installed cache rows 156 through 163. Cache cursor is 164; canonical owner
count is 28. Render/third/sampler state caches remain 2/1/1 with the previously
validated shared identities and reference counts. Actual VFS counters report
14 reads and 2,301,268 bytes, including the two new DDS reads.

## Evidence and validation

`reports/native_installed_cold_effect.json` records the source boundaries,
imports, direct calls, hashes and result. The ignored fixture is
`local/native_installed_cold_effect/cold_effect_probe.cpp`; the successful result
is `fixture_result_3.json`. Attempts one and two completed native admission but
stopped on an incorrect fixture expectation of one error-texture reference.
Source and assembly inspection established the lazy fallback acquisitions;
attempt three asserts their resulting count and shared identity.

MSVC Win32 `/O2 /Oy- /W4 /WX /fp:strict` compiled the fixture and 24 explicit
dependency units. The final link uses 23 of those units; the compiled-only
texture-construction service is identified in the report. All root cold-loading
providers are linked from hash-verified libraries with an existing immutable
source/build closure. No new CMake project, source registration or repository
test suite was introduced. The previous closed capture was copied, not modified.

Live Ghidra and the pinned installed executable agree on 1,934 complete function
bytes across `B2C2D0`, `B31BD0`, `B31C20`, `CD8FC0` and `CD9030`. All 26 selected
direct call sites pass the live instruction/body checker. Twelve import-thunk
bytes resolve to the original D3DX image-info and `Ex` imports. The fresh 29-byte
cache acquisition branch and its four-byte dispatch slot support the fallback
reference explanation. Prior immutable profile inputs were rechecked against
the same executable. Original `B3B280` executes only its empty branch twice;
the complete texture/effect paths execute reconstructed source interfaces.

The loader's original ABI has two stack arguments (name header and nullable
callback), returns the owner in EAX and uses `RET 8`. The two static stream-type
initializers take no arguments and use `RET`; their new C++ interfaces borrow
the same raw singleton lifetime, counter and root/file descriptors. Pseudocode's
unreliable register declarations do not define these source interfaces.

## Remaining boundary

The fixture still supplies raw renderer/capability/material-policy cells,
PC/USA settings, an explicit real HAL device and one installed loose-file mount.
It does not establish complete application/renderer startup, archive startup,
native FH3/SEH or replacement ABI, device retry, full teardown, drawing or
gameplay. Owners remain live through process exit. Shader-variant writes are
disabled; installed inputs are hash-checked before and after execution.

The immutable capture is `local/checkpoints/e7d61c14/installed-cold-effect/validation.json`,
SHA-256 `94b9259cd0b0c347a50f28a8296efcd4cbe0921d96241a3dbda56728e6776f62`. It freezes 1212 artifacts,
80 physical Win32 runtime modules and 215 root-library provider sources. The
fixture directory is closed and read-only; copy it before any further experiment.
