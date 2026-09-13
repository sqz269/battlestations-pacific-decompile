# Native retained cube and volume callbacks

Addresses: `00b3d7c0`, `00b3d800`, `00b3e1f0`, `00b3e230`.

The four complete callbacks operate on the existing raw logical texture,
retained-memory stream and real COM objects. They supply the cube and volume
profile callbacks needed by the renderer resource recreation pass. These are
new MSVC Win32 source interfaces; native caller ABI, SEH/FH3 and game behavior
are not claimed. No semantic owner mirror, fake reference count, caller method
callback, fallback profile, private renderer or allocator is introduced.

| Original entry | Inclusive body | Bytes | Source entry | Coverage |
| --- | --- | ---: | --- | --- |
| `00b3d7c0` | `00b3d7c0..00b3d7f3` | 52 | `release_native_cube_texture_retained_00b3d7c0` | complete |
| `00b3d800` | `00b3d800..00b3d833` | 52 | `release_native_volume_texture_retained_00b3d800` | complete |
| `00b3e1f0` | `00b3e1f0..00b3e221` | 50 | `recreate_native_cube_texture_retained_00b3e1f0` | complete |
| `00b3e230` | `00b3e230..00b3e261` | 50 | `recreate_native_volume_texture_retained_00b3e230` | complete |

All four original entries receive the owner in ECX, have no stack arguments,
preserve ESI/EDI and end with plain `RET`. They are modelled as `__thiscall`
methods with no semantic result. The recreation tail leaves D3DX's incidental
HRESULT in EAX; the native body neither checks nor stores it. The C++ API does
not expose that incidental register state. No original exception frame is
present. A throwing call prevents all later calls and stores; source adds no
cleanup, retry or rollback.

## Full release sequence

`B3D7C0` and `B3D800` have the same complete instruction sequence. Capture
`owner+10` in EDI. If nonnull, read that object's current table and call COM
`+04` (AddRef), then read the captured object's table again and call `+08`
(Release). Reload `owner+10` independently; if nonnull, read its current table,
call Release and then clear `owner+10`. Each COM method has a single pushed
interface pointer and the standard stdcall cleanup. The wrapper itself uses
no stack arguments or caller cleanup. Nothing touches the retained source.

| Entry | Captured AddRef | Captured, refreshed-table Release | Current-owner Release | Clear after return |
| --- | --- | --- | --- | --- |
| `B3D7C0` | `B3D7D1` | `B3D7D9` | `B3D7E8` | `B3D7EA` |
| `B3D800` | `B3D811` | `B3D819` | `B3D828` | `B3D82A` |

The original current table dispatch remains real COM virtual dispatch in C++.
The fixture changes the captured object's table during AddRef and replaces
the owner's pointer with a second real texture. The comparison distinguishes
the original table, refreshed table and second texture, while forwarding every
method to its real D3D9 implementation.

## Full retained recreation sequence

1. Read the current four-byte renderer publication at `F8D394`, then call the
   existing complete `B1FEF0` provider. Its seven-byte body returns the borrowed
   device at renderer `+1A10`, without AddRef. Keep that device across later calls.
2. Read current retained source `owner+2C` for cube or `owner+30` for volume.
   Read its current profile/table and call length slot `+30`. Preserve only EAX,
   the low DWORD of the signed EDX:EAX length; the high DWORD is discarded.
   The native push of `owner+10` before this call is a pending D3DX argument,
   not an argument consumed by the ECX-only length method, whose body ends `RET`.
3. Independently reload the owner's retained source and call complete `BEF610`.
   This reads stream backing `+08`, then backing data `+08`; it ignores the cursor.
   The pending length push is another D3DX argument, not an input to this leaf.
4. Call the simple four-argument D3DX cube/volume import with captured device,
   current data, saved low length and actual `owner+10` output slot. The stdcall
   import consumes exactly four DWORDs; no `ADD ESP` follows the call. Pop EDI,
   pop ESI and return. Do not preclear output, release its previous value or use
   a temporary output slot. Ignore HRESULT and retain all stream ownership.

| Entry | Device getter | Current length virtual +30 | Reloaded source data | Concrete import |
| --- | --- | --- | --- | --- |
| `B3E1F0` | `B3E1FA -> B1FEF0` | `B3E20D -> BEF600` | `B3E213 -> BEF610` | `B3E21A -> C2DFE0` |
| `B3E230` | `B3E23A -> B1FEF0` | `B3E24D -> BEF600` | `B3E253 -> BEF610` | `B3E25A -> C2DFDA` |

`C2DFE0` jumps through IAT `CE2404` to
`D3DXCreateCubeTextureFromFileInMemory`; `C2DFDA` jumps through `CE2408` to
`D3DXCreateVolumeTextureFromFileInMemory`. The new concrete import class resolves
exactly those exports from the caller-owned actual `d3dx9_40` module. Resolution
errors occur during host construction, outside the callback sequence. No DLL
load/search policy or callable setter is included.

`NativeCubeVolumeRetainedRecreationContext` borrows the publication storage,
existing `NativeRetainedMemoryOwnerContext` and concrete import class. It reuses
`dispatch_native_memory_stream_length` and the original-ABI `BEF610` leaf;
supported current profile `D642C0` has `BEF600` at `+30`. Arbitrary profiles and
changed method identities are outside this existing domain, not silently
accepted or dispatched to no-op substitutes. The two release entries require
only the raw owner and its current real COM objects.

## Producer and caller evidence

Fresh bounded assembly confirms cube constructor `B3CED0..B3CF50` installs
profile `D61870` at `B3CF0D` and zeros source `+2C` at `B3CF13`. Volume constructor
`B3CFA0..B3D02E` installs `D618B0` at `B3CFDD` and zeros `+30` at `B3CFE3`.
The existing base owner contracts establish COM `+10`; the complete cube owner
and volume owner reconstructions use that actual storage. In loader `B2C2D0`,
`B2C730` forms cube source slot `+2C`, `B2C807` forms volume slot `+30`, and the
common call `B2C80E -> B23640` publishes the retained wrapper using the existing
complete retained-memory assignment provider.

All live direct xrefs to these callbacks are their profile data slots:
`D61898` / `D6189C` hold cube release / recreate at `D61870+28/+2C`;
`D618D8` / `D618DC` hold volume release / recreate at `D618B0+28/+2C`.
The profile words were independently read from the verified saved program.
The current renderer orchestration uses these dynamic slots; no extra callback
arguments are inferred. That caller belongs to the integrator's separate packet.

Pseudocode incorrectly attributes the output and length pushes to the stream
leaves and suppresses the owner-source reloads. Full assembly, callee bodies,
profile producers and stack cleanup establish the contracts above. The report
retains exact numeric call rows and source locations. No Ghidra edits were made
by this worker; existing library names remain intact and four fresh exports were
written through the verified read-only exporter.

## Verification and replay

`python tools/ghidra_export.py verify-seeds` passed. A separate strict MSVC
14.51.36231 Win32 build used `/W4 /WX /O2 /MD /fp:strict`; the actual callback
source compiled and linked against the integrator's three supplied current
libraries. Existing repository build/CTest results are recorded in the report.
The source is intentionally not registered in shared CMake by this worker.
Integration must register it and replay against the resulting current library.

One bounded differential fixture is preserved outside the worktree at
`C:/Users/sqz269/bsp-aw-cube-volume-retained`. It saves its generator, source,
11 raw spans (301 bytes), live-byte output, span hashes, generated header,
compiler/replay script, executable and log. All spans matched the current disk
image and verified `bsp` program before execution. Fixed original low addresses
collided with process mappings, so the fixture relocates by `20000000`:

- Only four absolute code-storage operands change: `B3E1F5` and `B3E235`
  (publication addresses), `C2DFE2` and `C2DFDC` (IAT addresses).
- All relative calls, instruction sequences and complete release-body bytes
  remain unchanged. Original table/stream profile addresses relocate as data;
  their native method identity is restored before calling the source provider.
- Both D3DX IAT slots bind the actual installed SysWOW64 `d3dx9_40.dll` exports.
  The original and source execute real D3D9 HAL cube and volume operations.

The eight recreation comparisons cover both kinds, valid/invalid DDS and
empty/populated initial output. Successful resources match level metadata,
mip count and all level-zero pixels (all cube faces or volume slices). Invalid
DDS leaves the existing output unchanged; successful DDS replaces it without
releasing its old reference. Every unrelated owner field, retained stream field,
nonzero cursor and intrusive/accounting count stays unchanged.

Six release comparisons cover both kinds with null, ordinary and replaced
current output. Their exact forwarded COM method schedule, actual reference
postimages, final clear and unrelated owner storage match. Real retained-memory
allocation/copy/destruction providers are used and finish with zero counters.
This does not test native fault handling, arbitrary stream implementations,
full device-reset integration, in-game execution or visual parity.

`run.ps1` defaults to compiling **only** `probe.cpp` and linking the three
supplied current libraries (`-CoreLibrary`, `-LuaLibrary`, `-ZlibLibrary`), using
`-SourceRoot` for current headers. Before registration, explicit `-WorkerMode`
adds this one packet's source. No private archive rebuilding or hidden source
injection is part of the default replay. The executable embeds its manifest.

## AW integration analysis refresh

The integrator saved all ten AW original signatures and reviewed names,
verified their complete stored bodies and refreshed exports. The seven-byte
B3D7B0 body and two ten-byte EH handlers CBD2FB/CBD168 were defined under
owned leases and the Ghidra write lock. Missing-function observations above
describe the earlier worker capture. EH definitions are analysis metadata,
not additional reconstructed normal-body claims. Combined final-commit
validation remains separate from the worker fixture evidence.

## AW exact merged validation

The exact combined source commit `842045e886c30b2dce4cb63f333cffb9a851697b` passed the Win32 build,
both existing tests and four current-library-only original-byte fixtures.
Full counts, original-byte relocation, exception branches and parent coverage
are recorded in `reports/native_renderer_device_recreation_aw_validation.json`.
The earlier pending statements describe initial capture stages. This does not
establish whole-game rendering, native ABI identity or general concurrency.
