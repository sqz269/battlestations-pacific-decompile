# Native retained 2D texture recreation

Addresses: 00b3d7b0, 00b3e190

Packet AW implements both complete current-profile bodies over the existing
actual storage and provider domains. Descriptive names remain provisional.

| Native inclusive range | Coverage | Original ABI | C++ entry |
|---|---|---|---|
| `00B3D7B0..00B3D7B6` | complete, 7 bytes | `__thiscall`, ECX owner, no stack arguments, tail JMP, no semantic result | `release_native_texture_2d_retained_00b3d7b0` |
| `00B3E190..00B3E1EC` | complete, 93 bytes | `__thiscall`, ECX owner, no stack arguments, RET, no semantic result | `recreate_native_texture_2d_retained_00b3e190` |

`B3D7B0` has no saved Ghidra function at this audit. Its full raw sequence is
`MOV EAX,[ECX]; MOV EDX,[EAX+20h]; JMP EDX`. Current actual profile `D61948`
has `B3DD30` at `+20`, so the source selects the existing complete actual-storage
reset release provider. Its signed level iteration, cached surface provider,
current COM reference operations and final output clear remain in that provider.
The primary must define precisely these seven bytes before annotating this entry.

`B3E190` captures the current renderer publication `F8D394`, calls actual
`B1FEF0` to read renderer `+1A10`, and keeps that device for creation. Format at
owner `+18` selects 21 when it equals 20; every other value selects 0. It captures
the first retained stream `+4C`, actual output address `owner+10`, height `+2C`,
mip count `+3C`, and width `+28`, in that order. It calls the first stream's
current table `+30` for length, consumes only EAX, reloads owner `+4C`, and calls
`BEF610` on that freshly loaded stream for the backing data pointer. The supported
actual stream profile is `D642C0/+30=BEF600`; its established length provider uses
wrapped `stream.end - backing.data` and sign extends to EDX:EAX. The fresh data
provider loads stream `+8`, then backing `+8`, without reading cursor or end.

The 15 D3DX arguments, in API order, are:

```text
device, fresh_backing_data, low32(length), width, height, mip_levels,
usage=0, format=(owner.format==20 ? 21 : 0), pool=1,
filter=00070004, mip_filter=FFFFFFFF, color_key=0,
source_info=null, palette=null, output=actual owner+10
```

All 15 DWORD arguments are consumed by the imported stdcall entry. There is no
caller `ADD ESP`; after D3DX the body only pops EDI/ESI and returns. `C2DFE6`
is the six-byte jump through IAT `CE2400`, whose installed PE import is
`d3dx9_40.dll!D3DXCreateTextureFromFileInMemoryEx`. HRESULT is ignored. The
entry does not release an existing output, clear it, copy through a temporary,
retry, change the retained source, or insert an exception cleanup scope.

## Borrowed context and layout evidence

`NativeTexture2DRetainedRecreationContext` borrows the actual renderer publication,
the existing retained-memory context, actual `D61948` table words, the existing
reset surface profile, and a reference to the live real D3DX import cell. Original
numeric class/slot words select complete existing C++ providers; they are not
invoked as host callbacks. The current profile is re-read at release dispatch,
and the current stream profile/length slot is re-read at the native length call.
Unsupported profiles and invalid accessed storage are outside the API domain.

The producer `B3F930` installs `D61948` at `B3F968`, initializes mip count `+3C`,
level-array `+40/+44/+48` and retained source `+4C`, and writes width/height at
`B3F9C8/B3F9CB` and format at `B3F9FF` from the real COM descriptor. The existing
`B23640` assignment retains the source into `+4C`; the established `BEF6D0` and
`8D43C0` providers supply the actual 14h stream and 10h backing layouts. This
packet creates no copied owner, stream, COM interface, reference counter,
registry, destructor callback or substitute memory lifetime domain.

| Call site | Containing owned body | Native provider | Evidence |
|---|---|---|---|
| `B3D7B5` | raw `B3D7B0..B3D7B6` | `B3DD30`, current table `+20` | complete 7-byte raw sequence and actual `D61948+20` |
| `B3E19A` | `B3E190..B3E1EC` | `B1FEF0` | complete 7-byte callee reads `ECX+1A10` |
| `B3E1D8` | `B3E190..B3E1EC` | `BEF600`, current stream table `+30` | full leaf and actual `D642C0+30` |
| `B3E1DE` | `B3E190..B3E1EC` | `BEF610` | complete 7-byte callee, preceding fresh `MOV ECX,[ESI+4C]` |
| `B3E1E5` | `B3E190..B3E1EC` | `C2DFE6` | full thunk, actual installed import, 15 DWORD pushes |

Live xrefs to the two owned entries are their actual profile cells `D61970`
and `D61974`; neither has a direct code caller in the queried saved analysis.
The complete owned listing was inspected, including all calls and register
writes. Higher renderer registry traversal is not reconstructed by this packet.

## Verification and reproduction

Strict MSVC Win32 `/W4 /WX /fp:strict /MD` source-object compilation passed.
The existing seed verification matched all eight seeds against disk. The
repository build passed and both existing CTests passed. Details are recorded
in the accompanying report;
the primary must register this new source in its own CMake integration.

The external fixture is preserved at `C:/Users/sqz269/bsp-aw-retained2d`, outside
the disposable worktree. Its `prepare.py` invokes only the guarded read-only
`bsp.py ghidra bytes` interface. Every analysis read verifies `bsp.gpr`,
`/battlestationspacific.exe`, language and image base. All 100 owned bytes,
the full reached reset/surface/renderer/stream bodies, the D3DX import thunk,
actual profiles and two publication cells are checked against the installed PE.
The zero-filled renderer publication lies in the PE's virtual tail, not raw file
data. No game file or Ghidra state was changed.

The probe reserves native addresses in its suspended child before loader heap
initialization. It executes both full original bodies and complete copied native
dependencies at their original addresses, with no instruction relocations,
replaced instructions or service bridges. Only the renderer publication and
D3DX IAT data cells are rebound to fixture renderer storage and the real installed
D3DX export. Original native profile words remain unchanged. Code pages become
execute/read. Actual stream construction/destruction and the source release
dependencies are linked from the current primary libraries.

Five original/source creation pairs use a real HAL D3D9 device and installed
`D3DX9_40.dll`: format20 mapping, format21/0 fallback, scaled dimensions, automatic
and explicit mips, zero-width D3DX defaulting, successful replacement of an
existing output, and invalid image data with an existing output. They compare
output/null/old-pointer identity, level descriptors/counts, all level0 pixel bytes
through a deterministic hash, and unchanged owner fields outside `+10`.
The invalid image returns native EAX `88760B59` and retains the seeded old
texture in both bodies. No HRESULT-driven repair is present in the source.

Three release pairs exercise one actual cached D3D surface and texture, negative
signed count with a texture, and an empty owner. Original/source both clear the
same actual surface and texture fields. Every native invocation checks unchanged
ESP and EBX/ESI/EDI/EBP. The actual memory counters balance after stream deletion.
These checks do not establish arbitrary profile mutation, asynchronous mutation,
fault/exception behavior, source binary ABI equivalence, whole renderer reset,
game execution or visual parity. No new permanent tests were added.

```powershell
python C:/Users/sqz269/bsp-aw-retained2d/prepare.py <repository>
# Final integration mode compiles ONLY probe.cpp against supplied current libs.
& C:/Users/sqz269/bsp-aw-retained2d/run_probe.ps1 `
  -RepositoryRoot <repository> -LibraryDirectory <current-library-directory>
# Explicit worker mode additionally compiles this packet's source object.
& C:/Users/sqz269/bsp-aw-retained2d/run_probe.ps1 `
  -RepositoryRoot <worker-repository> -LibraryDirectory <primary-library-directory> `
  -WorkerSourceObject
```

`verify_report_calls.py` retains the one structural failure for `B3D7B5` while
the seven-byte entry has no saved Ghidra function. Its three direct recreation
calls verify; the checker labels the stream virtual indirect, and the profile
bytes establish its concrete provider. The
primary may eliminate that structural failure only by defining the exact raw
function, then applying annotations while preserving existing names/comments.

## AW integration analysis refresh

The integrator saved all ten AW original signatures and reviewed names,
verified their complete stored bodies and refreshed exports. The seven-byte
B3D7B0 body and two ten-byte EH handlers CBD2FB/CBD168 were defined under
owned leases and the Ghidra write lock. Missing-function observations above
describe the earlier worker capture. EH definitions are analysis metadata,
not additional reconstructed normal-body claims. Combined final-commit
validation remains separate from the worker fixture evidence.

## AW exact merged validation

The exact combined source commit `e30488f4a1059a41f17cc8836b64fa7c901b95a9` passed the Win32 build,
both existing tests and four current-library-only original-byte fixtures.
Full counts, original-byte relocation, exception branches and parent coverage
are recorded in `reports/native_renderer_device_recreation_aw_validation.json`.
The earlier pending statements describe initial capture stages. This does not
establish whole-game rendering, native ABI identity or general concurrency.
