# Retained application render-resource providers (CC10)

This packet exposes the existing runtime-texture, surface-factory, texture-level
getter and holder providers through the actual ready application renderer. The
application retains every context; the returned view only borrows references.
It introduces neither a native producer call nor B107F0 invocation into startup.
The existing cold B14A10 entry remains unchanged.

## Process cells and native evidence

`GameNativeRendererScalarProcess` now owns the one source-process copy of each
native cumulative allocation cell. Both native addresses lie in the loader-zero
tail of the installed PE's `.data` section, beyond its file-backed raw extent but
inside its virtual extent; the live Ghidra bytes are also zero. These are allocation
totals, distinct from the existing live texture/surface tracking counters.

| Cell | Exact native accesses | Retained source consumers |
| --- | --- | --- |
| 0108D4BC | B2A287 read, B2A292 FILD, B2A2C0 write | B2A070 runtime texture factory, also reached through B4E020 |
| 0108D4C0 | B2A924 read, B2A92F FILD, B2A96C write; B2AAFA read, B2AB06 FILD, B2AB3A write | B2A7C0 render-target and B2A9A0 depth factories, sharing one context |
| CE221C | PE KERNEL32.dll!InterlockedIncrement import; B3FD9C captures its current target in EBP, B3FDCC calls that target before the null test | retained texture-level getter; holder's external-surface retain reads the same cell |

The source process object resolves the real Windows increment export once using
the existing CameraGraph import-resolution convention. Its address-stable
volatile pointer cell is borrowed by the getter; no increment occurs during
resolution or view acquisition. The holder borrows CameraGraph's existing
current CE2220 decrement cell. The two cumulative DWORDs initialize once to the
proven loader-zero preimage, and neither graph construction nor native deletion
resets them. The existing raw factories retain their exact x87 accounting;
this packet adds no arithmetic projection or alternate counter.

The report records five live-Ghidra/installed-PE byte blocks: the three accounting
tails and the complete B4E020 and B3FD80 bodies. Each block records inclusive and
exclusive ends, final instruction size and SHA-256. All direct call rows derived
from the two complete bodies are checked by `verify_report_calls.py`; indirect
dispatch remains covered by the existing provider contracts and the focused
application exercise below. No Ghidra functions or annotations were changed.

## Retained identities and lifetime

`RenderResourcesGraph` retains these contexts in dependency order:

1. `NativeRuntimeTextureConstructionContext` uses the established texture-owner
   context and original D619E4 literal.
2. `NativeRuntimeTextureCreationContext` uses that constructor, the existing
   process synchronization domain, actual DeviceGraph recreation context,
   process 0108D4BC cell, and original CE3978/D57DA0 numeric literals.
3. `NativeRendererSurfaceFactoryContext` shares the existing surface-owner
   context, synchronization/recreation and literal references, with 0108D4C0.
4. `NativeTextureSurfaceGetterContext` shares that surface-owner context and the
   process CE221C cell.
5. `NativeRenderTextureSurfaceOwnerContext` references the preceding factory,
   getter and surface contexts and the existing current decrement cell.
6. `NativeRenderResourcesDirectTerminalDomain` references the application's
   existing canonical owner registry, that holder context and original D61EB8.

Consequently, the factories use the same actual renderer publication, texture
and surface pools, string storage, support singleton, serial/tracking cells,
device and registry as the existing application graph. No temporary provider
context, duplicate accounting state, additional ownership credit, profile copy
or blanket nested registration is introduced. Dependencies are declared before
RenderResourcesGraph in the enclosing Impl and outlive it.

`render_resource_providers()` requires the existing ready phase and returns a
`GameNativeRenderResourceProviders` reference view. Copying the view does not
copy its providers. The caller must keep the application alive, retain each
operation's acquired/diagnostic storage through all dependent use, and retire
surviving native owners before renderer drain. Failed attempts retain the
existing providers' acquisition state and may require process retention;
borrowing this view adds no rollback or exception cleanup.

The direct-terminal domain is retained but **not installed**:
`render_resources_lifetime().direct_terminals` remains null. A subsequent
composition packet must establish genuine completed producer/backing identities,
live native counts and cleanup preconditions for every reached raw surface,
holder and runtime texture before binding this exact domain. It must preserve
canonical lookup/retirement first and the restriction against bypassing a bound
nested companion. Having original profile words alone supplies no admission.

## Verification and remaining work

The strict MSVC Win32 build and all three existing CTests pass. One ignored
focused exercise was rebuilt from **current** `src/game_main.cpp`, 69 current
production application objects and the three current libraries. Only the
StartupHost forwarding pattern/build setup was adapted from R100; its old
executable was not run. The current platform dependency `powrprof.lib` was added
to that historical local link command after its unresolved power-import symbols
were diagnosed. The probe uses `/MD /fp:strict /MANIFEST:EMBED` and ran through
`tools/run_game.ps1` with a distinct log and settings directory.

At the actual ready application's loop boundary, the exercise checked two
borrowed views' stable identities, canonical process cells, the real Windows
increment export and shared renderer/surface/recreation/decrement/registry
domains. It constructed a genuine 18h mode-one B4E020 holder, which produced a
64x32 runtime texture, cached level surface and independent render target. An
additional getter hit retained the same cached surface; only that returned
credit was dropped. B2A9A0 then produced a 64x32 D24S8 depth surface.

Observed cumulative bytes were texture `0 -> 8192` and surface
`0 -> 8192 -> 16384`. After releasing the depth and holder creator counts and
calling their actual deleting providers, renderer list lengths and native
tracking counters returned to baseline; cumulative bytes stayed at 8192 and
16384. The canonical registry was unchanged and direct-terminal pointer stayed
null throughout. The normal application singleton drain returned, followed by
final device/API COM releases of zero. Three loop ticks yielded two presented
frames and one skipped present, with process exit zero.

This proves this focused source composition on the installed application/device
path. It does not prove original binary ABI/FH3/SEH, exceptional producer paths,
concurrent domain rebinding, complete B107F0 execution or renderer gameplay.
The next composition work must retain the staged initializer's other contexts,
actual argument cells and acquired blocks, then admit the appropriate lifetimes.
The bloom fourth-DWORD backing and distortion cleanup-preimage boundaries remain
explicit; this packet neither enlarges allocations nor invents unwritten bytes.

Ignored raw listings, import/PE facts, local probe sources, object/toolchain
manifests, build/launch logs and executable hashes are indexed by
`reports/native_application_render_resource_providers_cc10.json` for archival.
