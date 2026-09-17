# Application texture cache and cold-load lifetime R92

Addresses: `00B319B0`, `00B30B40`, `00B2C2D0`. Read-only supporting bodies:
`00B31BD0`, `00B316C0`, `00B32370`; loader-zero byte `0108D5A4`.

## Application change

`GameNativeRendererApplication` now constructs the actual texture-loading
context and cache alongside its existing device/destruction graph. Previously
the application had cache construction/deletion and texture terminal providers
but no connected cold-load context for render-resource and compiler callers.

The new private `TextureLoadingGraph` borrows the application's existing:

* `NativeRenderActualOwnerRegistry`, renderer publication and synchronization;
* actual string pool, VFS resolution/open/conversion/date routes and platform pump;
* mesh/section process pools, 2D/cube/volume texture pools, retained memory,
  resource support, shared texture serial and accounting;
* original numeric profiles, device-recreation context and loaded D3DX9_40 DLL.

The owner bridge shares this registry with renderer destruction. No competing
registry, copied native owner, synthetic renderer or replacement VFS is created.
The application exposes borrowed cache/registry access for later resource
consumers. `GameStartupHost` exposes its existing renderer, consistently with
its other borrowed service accessors.

Image information uses `D3DXGetImageInfoFromFileInMemory`; 2D creation uses the
existing **Ex** import with its fifteen-argument stdcall signature. Cube and
volume imports use their actual four-argument forms. Retry calls the existing
full device-recreation provider. The nonzero post-load callback remains an
explicit unbound boundary; the verified callers pass zero. Cube/volume/retry
execution is not covered by this packet.

The stable empty-name byte is a source representation of the verified native
loader-zero `0108D5A4` value. It adds no original-address alias claim.

## Runtime evidence

One local probe copies only the production `game_main.cpp` bootstrap and wraps
its startup host with 26 forwarding methods. Before the normal platform loop,
it exercises the real application's new cache. It links **51 current CMake
application objects** and the three current libraries. It replaces no production
object or native child implementation, seeds no cache records and supplies no
fake device, VFS, loader, texture or terminal. Original machine code is not
executed by this source probe.

Actual installed assets produced:

| Texture | Level-zero dimensions | Load |
| --- | --- | --- |
| error.tga | 128 x 128 | Native recursive fallback initialization |
| black.tga | 32 x 32 | Cold source load |
| noise.dds | 256 x 256 | Cold source load |
| kosz_01.tga | 512 x 32 | Cold source load |
| szor_01.tga | 1024 x 64 | Cold source load |
| csikok.tga | 64 x 256 | Cold source load |
| splotch.tga | 1024 x 256 | Cold source load |
| white.tga | 128 x 128 | Cold source load |

The probe checks completed VFS/retained-memory publication, canonical companion
registration, the actual D61948 profile/count, D3D texture descriptions and the
same production device. Eight subsequent lookups return the exact cached
owners without entering the file loader; observed retain/release counts match.
Seven ordinary creators retire through their normal notification/deleting
routes before shutdown. The error texture's native cache/fallback references
remain for the real singleton drain, which empties the canonical owner registry.

Both the probe and unmodified executable exit **0** after two frame ticks,
one Present and one skipped Present. Their normal drain joins the renderer
worker, clears renderer/Lua/cache publications and finishes with device/API
COM reference counts **0/0**. These are runtime observations, not image/pixel
or gameplay parity.

## Shutdown observation retained

The first probe assertion overlooked the native recursive error texture and
stopped with code91 after successful black loading. Correcting that expectation
required no production change. A second probe deliberately retained an ordinary
white creator into renderer shutdown and observed a remaining canonical owner
(later metadata guard termination C0000409). The original B316C0 listing confirms
its release-then-current-tail-removal schedule, including reentrant notification;
this is not a general substitute for consumer retirement. The final probe releases
every ordinary returned reference before the renderer drain and leaves only the
native error-cache/fallback pair. Both unsuccessful observations are retained.
No native cleanup order was changed to make the probe pass.

## Static validation and remaining work

Strict MSVC Win32 Release build and all three existing CTests pass. No repository
tests were added. Six complete bodies total 3420 bytes; one loader-zero byte is
also verified against live Ghidra and the supported original PE. Direct call
rows are mechanically checked; indirect rows remain separately identified.
The only unlisted bytes are the three-byte unreachable `LEA ECX,[ECX]`
alignment at B30BDD; no functional listing gap remains unresolved.
Saved names/comments, source/object/library/executable hashes, runtime logs and
sealed artifacts are recorded in `reports/native_application_texture_cache_r92.json`.

This is new application composition, with zero new unique native-body credit.
The production application constructs the cache graph; the focused probe is
the caller that exercises these cold loads. Normal startup still marks B14A10
render-resource construction unimplemented. Its camera/type/pool/runtime
bindings, complete parent teardown, B107F0 resource initialization, actual
material effect/compiler graph, full post-effect execution, native FH3/SEH,
failure/retry/concurrency coverage, pixel parity and gameplay remain open.
