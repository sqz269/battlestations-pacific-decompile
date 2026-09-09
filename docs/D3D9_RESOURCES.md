# Surface and buffer ownership

`00b3cc80` initializes a surface binding. Assembly shows an AddRef followed by
GetDesc, copying Format, Width, Height and MultiSampleType to offsets `18h`, `1ch`,
`20h`, `24h`. The surface pointer is at `2ch`. Offset `28h` remains unchanged for
nonnull input; null clears the metadata including that field. The pseudocode's
apparent stack pointer stored as Format is a decompiler error.

The helper overwrites the surface pointer without releasing a previous reference.
Its confirmed caller is constructor `00b3f630`; it is not a replacement API.
The new `surface_initialize_00b3cc80` requires an empty binding and reports API
failure with cleanup. Native code ignores HRESULTs. `surface_release` is new
owner cleanup, not a reconstruction of the complete wrapper destructor.

The full constructor creates a 52-byte wrapper with intrusive reference count 1
at `+4h`, installs vtable `00d619a0`, initializes flags, binds the surface, invokes
singleton helper `00b3e730` and updates a tracking counter for certain flags.
These surrounding operations remain unported.

## Default surface capture

`00b238d0` gets render target 0, queries its description, constructs a wrapper with
flags zero, installs it at renderer `+197ch`, and releases the temporary wrapper
and COM getter references. The installed wrapper retains one COM reference through
`00b3cc80`. It repeats the ownership transfer for the depth surface at `+198ch` and
calls `00b21690`, which binds the wrapped surface through SetDepthStencilSurface.
This resolves the broken decompiler stack tracking, but is not yet a full C++ port.

## Dynamic buffers

Renderer `00b2aeb0` creates a 16 MiB vertex buffer with FVF 0 and a 1 MiB INDEX16
index buffer. Both have usage `208h` (DYNAMIC | WRITEONLY), DEFAULT pool, and no
shared handle. These API calls are extracted in `create_dynamic_buffers_00b2aeb0`.
The extraction reports failures and cleans up a partially created pair; the
original assumes success. It is a separately counted fragment, not the complete
resource initialization sequence.

The native buffers are attached to 44-byte wrappers initialized by `00b4bbb0`
(vertex) and `00b4bb60` (index). Attach methods `00b4c370` and `00b4c250` store the
two metadata arguments at `14h`/`18h` (startup supplies `1000h` and byte capacity),
then replace COM pointer `28h`, retaining the new object before releasing the old
one when distinct. They also construct diagnostic strings and invoke `00b3e730`.
Destructors `00b4bab0`/`00b4b900` release and clear `28h` before base teardown.
The new fragment owns direct COM references; it does not simulate this registry
or supply fake wrapper implementations.

## Verification and next work

The existing D3D9 probe checks a surface binding after releasing the getter's
reference, queries both buffer descriptions, and releases all owned references.
It observes 640x480 color format 21, capacities 16777216/1048576, usage `208h`,
DEFAULT pool and INDEX16. The build and existing CTest pass; no new test cases.
Original code ranges match disk, with hashes in `reports/d3d9_resources_validation.json`.
This does not prove refcount parity, reset compatibility, drawn geometry or gameplay.

## Surface reset lifecycle

The surface vtable's `+3ch` method `00b3d510` releases and nulls the COM pointer,
preserving metadata. It makes a balanced AddRef/Release pair first; the port
preserves that sequence. Newly recovered `+40h` method `00b3d550` recreates from
saved metadata. Byte `+30h` selects CreateDepthStencilSurface with discard TRUE
or CreateRenderTarget with lockable FALSE. Both use multisample quality 0 and a
null shared handle. The method consumes one device argument with RET 4h.
The new interface returns the HRESULT and requires an empty binding. It does not
infer surface kind from the format or replace default-surface capture.

The diagnostic host now creates offscreen color and depth surfaces, releases them
through `00b3d510`, calls a real windowed device Reset, recreates both through
`00b3d550`, and checks 128x128, formats A8R8G8B8/D24S8 and usages RENDERTARGET/
DEPTHSTENCIL. All temporary getter/swap-chain/default-pool references from earlier
probe stages are released before Reset. This validates the two helpers across a
real reset; it does not validate the native reset scheduler or full renderer.
See `reports/d3d9_surface_reset_validation.json` for the separate evidence.

The native scheduler `00b2abd0` first calls `00b237d0` to release dynamic COM buffers
without deleting their wrappers, then `00b262c0` to unbind resources, invoke reset
callbacks and clear bindings. `00b241c0` receives the cache subobject at renderer
`+34h`, clears 210 render-valid bytes and twenty sampler banks, and releases
intrusive binding references. After Reset, `00b23b10` reacquires default surfaces
and invokes registered callbacks, including surface `+40h`. These orchestrating
routines are named and traced, but remain unported.

Registry investigation: `00b61d50` only installs vtable `00d62b64` in the eight-byte
singleton. Its recovered deleting destructor `00b61d60` clears global `0108fedc`.
This alone does not establish all registry behavior. Buffer base cleanup shrinks
the pointer array at `+8h` to zero through `00b49700` and frees its backing storage.
Ghidra still truncates those bodies after free despite the earlier no-return fix;
their missing fallthroughs must be recovered before claiming complete teardown.
Inline Ghidra scripts are disabled on the current server, so no scripted body repair
was performed. Ordinary analysis, naming and export remain available.

Next: reconstruct the remaining resource callbacks and buffer restoration, recover
truncated base cleanup, and integrate the native release/reset/recreate sequence.
