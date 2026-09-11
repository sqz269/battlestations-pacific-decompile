# Retained Icon runtime over evaluated Lua

Addresses: 00AB5C60, 00AB2540, 00AB3310, 00AB66C0, 00AB10F0, 00AB3CB0.
Names are descriptive hypotheses. This is a new C++ ownership/interface
projection, not the 138h native class, allocator, intrusive reference ABI or a
drop-in binary replacement.

`GuiIconRuntime` owns the existing `GuiIconWidget` derived state over the same
`GuiLayoutWidget` used by the base owner. It consumes the actual evaluated
`GuiTable` snapshot from `GuiLua51Host`; no Lua source parser is involved.
The factory must construct the type6 base first, call `constructed74_00ab2540`
after node binding/parenting, run the base property/child traversal, then call
`read_properties_00ab3310` and finally `loaded78_00ab10f0`.

The derived reader uses the recovered `00BD63B0` conversions. It appends
consecutive positive integer `States` entries and stops at the first nil.
Each state's omitted Size/Pivot inherits the already-bound base values.
`UV_LURB` is four indexed float reads: a missing lane becomes zero, whereas
an absent UV table retains (0,0,1,1). The resolved atlas rectangle still uses
`gui_icon_set_resolved_uv_00ab1680`: only exact reversed endpoint pairs flip
an axis; arbitrary authored subrectangles do not crop the atlas rectangle.
Native conversion behavior for malformed aggregate shapes is not claimed:
the runtime throws explicitly instead of consuming native uninitialized data.

The load hook calls the real base `00AA7170`, then selects `(0,0,1)`. This
resets authored partial-display mode/ratio when that selection requests a
rebuild. A textured empty state list remains unbuilt when the original
selection predicate rejects state0 without any mode/ratio change. State
indices remain signed16, with the native unsigned range rejection represented
by a checked exception rather than `__report_rangecheckfailure`.

## Native hooks and geometry evidence

The configured CLI verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe` before each live batch. Analysis was read-only;
no functions, names, comments, prototypes or bytes were changed in Ghidra.

| Entry | Original ABI | Inclusive final instruction |
| --- | --- | --- |
| 00AB5C60 | ECX=this, no stack args, EAX=this | 00AB5D29 RET |
| 00AB2540 | ECX=this, no stack args | 00AB25F7 RET |
| 00AB3310 | ECX=this, stack visitor pointer | 00AB3834 RET4 |
| 00AB66C0 | ECX=this, stack name/UV/pivot/size pointers, EAX index | 00AB6942 RET10h |
| 00AB10F0 | ECX=this, no stack args | 00AB110F RET |
| 00AB3CB0 | ECX=this, stack low16 state index | 00AB44A9 RET4 |

The actual vtable bytes at00D5C530 establish +74=00AB2540,
+78=00AB10F0, +7C=00AB1860 and +80=00AB3CB0. Ghidra has **no function**
at00AB2540. Its184 disk bytes were disassembled through `bsp disasm-raw`
and compared by inspection with the live byte dump. The body checks base+74,
allocates a BCh mesh through00B73B60/00B73D70 when clear, and calls
00B75170 with geometry index0 and bounds (-1,-1). The constant00D7A260
was independently verified as `00 00 80 BF`, not a maximum-float sentinel.
It then always calls00AA7DC0 with a zero position. No base+74 call occurs.

The new shared publication helper composes the existing
`GeneratedInstanceGeometry`, `LogicalVertexStream`, `MaterialCloneState`,
`MaterialParameterBindings`, `D3D9VertexLayout` and `D3D9StateCache` owners.
A GUI mesh uses only the existing generated shape's mesh stream, layout and
section; instance stream/index buffer must be null. It is retained by the
same model association returned by the required environment callback.
This reuse is a typed ownership projection, not a claim that GUI meshes are
native instancing objects.

The helper requests actual `SimpleColor.mvfm`, four vertices, stream flags1,
locks through00B49980, selects/reuses the actual compiled material, registers
live cOverbrightFactor/cLowColor/cHighColor/cBlendFactor sources, invokes the
required clip/owner binding, and assigns the selected texture through00B189F0.
The section is primitive5, range words{0,4,0,2}. Cached filter equality reuses
the prior material; ShaderName changes alone do not invalidate that native
cache predicate. State size/pivot overwrite the SAME base before quad writes.
Base+74 suppresses position, UV and color writes. Otherwise declaration
offsets select float3 position, float2 UV and packed DWORD or float4 white
color; unsupported declarations fail explicitly. Invalid partial modes retain
position/UV bytes but still clamp the ratio and write white color.
The helper unlocks, recomposes the base, retains the material, constructs the
section layout and invokes real virtual+50 color publication. It issues no draw.

FrameBox can reuse `GuiGeometryRuntimeServices` and the publication helper with
its own count/range/material decision and geometry writer. Its native class
behavior remains the responsibility of that separate runtime.

## Resource contracts and unsupported paths

Texture resolution delegates to the existing00AA2660 atlas owner contract.
Every returned native reference has a matching release token, including
duplicate states referencing the same texture. Material slot0 retains the
actual LogicalTexture identity; its supplied shared owner must keep the COM
resource alive independently of the widget state references. No fallback
texture, fabricated declaration, effect identity, queue value, mesh attachment
or no-op scene hook is supplied.

The environment must provide real model association, named stream/material
creation, clip registration and material-to-widget lifetime binding, base
load/position/transform/color hooks, texture resources and platform filter
state. Device/cache/base storage must outlive all consumers of their borrowed
parameter sources. The active scene owner controls teardown and cache unbinding.
Failed callbacks/HRESULTs throw with partial native-order state possible;
startup must discard the failed page. Exception unwinding unlocks the stream
and releases locally owned references. Native SEH/OOM behavior is not reproduced.

DynamicVB is read and retained; this native rebuild passes flags1 regardless
of that byte. AutoRotate and DelayedTextureLoad are also decoded, but this
runtime explicitly rejects nonzero rotation rate or delayed loading: their
frame consumer00AB6430 is outside this packet. Clone construction and that
frame updater are not supplied. The installed `_Mouse` page needs none of
those unsupported paths.

`geOrder` remains in the retained snapshot as authoring metadata. The later
correction in `GUI_LAYOUT_LOADER.md` is confirmed: current disk scans found
neither ASCII nor UTF16 `geOrder`, and the referenced-string query found none.
It is not read by the Icon runtime and must not be invented as a draw sort.
Actual layer `RenderOrder` remains the existing layer/queue pipeline's input.

## Validation

MSVC Win32 Release build and both existing CTests passed after eight native
seed byte checks. An ignored manifested Win32 fixture executed the actual
installed `_Mouse.lua` in Lua5.1.1, closed Lua after taking the snapshot,
then bound the real base and Icon fields. It verified10 Icons,14 states and
four authored horizontal flips; all states inherited the expected base
Size/Pivot, and every geOrder remained in the snapshot. One focused sparse
Lua table verified nil termination, numeric-string integer/float coercion,
Lua truthiness of zero and zero-filling missing aggregate/UV lanes.

The fixture does not invoke real mesh/material/texture/scene services. Those
implementations are required at the application boundary and remain for
integration validation. No runtime draw, screenshot, complete native lifetime,
native Icon differential, ABI compatibility or in-game result is claimed.
No new permanent tests were added; the game installation was unchanged.
