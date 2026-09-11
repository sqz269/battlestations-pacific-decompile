# Native GUI mesh ownership and publication fragments

`GuiNativeGeometryOwners` supplies concrete mesh/section allocation, canonical
reference companions and model association. It does **not** make the existing
typed `GuiGeometryRuntimeServices` rebuild usable by native bounds or rendering.
There is no cast from `GeneratedInstanceGeometry` to a raw mesh and no second
native reference count, hierarchy, mesh stream list or section field cache.

## Evidence and scope

Ghidra reads on 2026-09-11 verified project `bsp` at
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 LE32,
image base00400000 through `bsp.py ghidra` before every query. No Ghidra
mutation occurred. Existing names are descriptive hypotheses, not symbols.
The exact old names/comments are captured in `reports/gui_native_geometry.json`.

| Owner | Whole native body, inclusive bytes | Original ABI | Implemented fragment |
|---|---|---|---|
| Icon constructed+74 | AB2540..AB25F7 | ECX widget, RET | AB2563..AB25C4 common mesh allocation/association/release |
| FrameBox constructed+74 | ACF8F0..ACF9A7 | ECX widget, RET | ACF913..ACF974 same sequence |
| Icon rebuild+80 | AB3CB0..AB44AB; final RET4 starts AB44A9 | ECX widget, stack signed16 state, RET4 | raw section acquisition and AB443D..AB4489 publication |
| FrameBox rebuild+80 | AD0D80..AD1580; final RET4 starts AD157E | ECX widget, stack signed16 state, RET4 | raw section acquisition and AD150D..AD155D publication |

All four functions currently exist in the saved program. The old AB2540 ledger
note saying no function existed is stale. Read-only flow audits report57/57
instructions for the two constructed hooks and614/630 for the rebuilds, with
zero listing gaps. No free-gap repair is required for these bodies. Assembly
was checked because the constructed hooks use x87 and the rebuild pseudocode
contains unaffiliated registers/unreachable-block warnings.

Both constructed hooks test widget+74, capture widget+4C, allocate the BC-byte
mesh via canonical pool0108FFF8, call B73D70, load D7A260 once with FLD, duplicate
that float to both stack slots, call B75170(0,mesh,s,s), then decrement the
creator reference and dispatch current virtual0 on zero. D7A260's saved bytes
are `00 00 80 BF` (-1). B75170 treats these as unchanged sentinels, preserving
model+178/+17C; they are not newly assigned LOD limits. The mesh constructor
itself establishes its four phase bounds, lod_count0 and lod_bits from D7A24C.
The caller still owns the widget+74 gate and AA7DC0(zero position) after this
fragment, including when the gate suppresses allocation.

## Concrete ownership

The domain allocates actual C0-byte mesh slots containing the BC-byte payload,
and64-byte section slots containing the60-byte payload. Their separate pool
words at+BC/+60 remain owned by the existing pools. Exactly one
`NativeMeshReference`/`NativeMeshSectionReference` is registered per allocation;
each borrows that object's actual+04 without initializing or retaining it.
Registration callbacks insert and erase these companions in the caller's SAME
`NativeRenderActualOwners`, which is also required by the model environment.
There is no fallback resolver or process-global replacement pool.

On association the creator starts at1, B75170 publishes raw model+180 and
increments to2, then the creator decrement leaves1. Existing mesh releases go
through their current canonical companions. On final release, the native
destructor runs, its physical slot returns to the issuing pool, registration
is removed without reading returned storage, and only then the host companion
is disposed. This domain and its environments must outlive all such releases.

`acquire_gui_native_section_fragment` creates and registers a section when
raw mesh+58 is zero. Otherwise it retains raw `[mesh+54][0]`. Its boolean is
only the recovered local append decision. Raw primitive/range/material writes
and all actual resource construction belong to the caller's prior rebuild.

`publish_gui_native_section_fragment` begins after stream unlock and AA7220.
It calls B864C0 with the selected real material, releases the temporary material
reference, calls B865A0 with the actual mesh and current renderer layout service,
appends via B73C60 only for a new section, then releases the acquired section
reference. The caller runs widget virtual+50 last. The material/section pointer
arguments become null immediately before their releases; remaining nonnull
pointers still belong to the caller if an earlier operation throws. No rollback
or original SEH parity is claimed. A failed host allocation/registration unwinds
its new actual object through the existing destructor/pool path.

## Required next packet

The typed rebuild currently assigns `GeneratedInstanceGeometry.mesh_stream`,
`.section.material_clone_owner`, and `.combined_layout`. Those assignments do
not write raw mesh+64, section+20 or section+50 and cannot feed B732C0/B855B0
native GUI bounds or a raw render queue. This module intentionally installs no
`GuiGeometryRuntimeServices` callbacks and adds no apparent successful adapter.

The next packet must establish these identity/lifetime projections:

1. Renderer current+38 loads `SimpleColor.mvfm`; current+5C creates a4-vertex
   Icon or54-vertex FrameBox stream, flags1. Its raw stream and canonical+04
   companion must be the SAME resource represented by `LogicalVertexStream`,
   including lock/unlock current+10/+14, descriptor current+24 (B48CE0 reads
   stream+68), physical buffer and device-cache ownership. Then B73BB0 must
   publish stream0 and release the returned creator and declaration refs.
2. 00535320 resolves effect through manager+48, allocates110 bytes via B18780,
   constructs material via B18900, and releases its effect temporary. That
   actual material requires canonical destruction and a `MaterialCloneState`
   view over its SAME textures, lighting, effect and parameters. The existing
   semantic B18B60 clone is not a native owner. Clip/widget registration is a
   separate packet at AA9F10/B18A40; this packet neither changes nor replaces it.
3. B865A0 selects actual stream descriptors and calls CURRENT renderer+40.
   Its retained native layout (B2F710 factory/tree/pool domain) needs the SAME
   `D3D9VertexLayout` projection and canonical zero-reference owner. Constructing
   an unrelated `make_shared<D3D9VertexLayout>` does not satisfy that identity.
4. Integrate those projections at AB3CB0/AD0D80's actual publication points,
   using this concrete mesh/section domain. Keep the widget's native model+180
   and any typed geometry view over one authoritative native owner. Do not
   shadow primitive/ranges or invent stream/material counts for bounds.

Raw suffix callers must supply those real registered resources and current
layout service; missing resources fail explicitly. No resource, shader queue,
COM stream or material is synthesized here. Existing typed runtimes are
unchanged and remain outside the completed native-owner boundary.

## Verification

The packet uses `scripts/build.ps1` (MSVC Win32, `/W4 /WX /fp:strict`) and the
two existing CTests after `verify-seeds`. One ignored constructor/ownership
fixture in `local/gui_native_geometry_probe.cpp` checks actual creator and
retained counts, model+180, sentinel preservation, raw section identity,
replacement while externally retained, canonical retirement and pool return.
It supplies no fabricated stream, material or layout and consequently does not
claim the raw publication suffix was render-tested. Final results are recorded
in the report. No permanent tests, native ABI replacement or game validation.
