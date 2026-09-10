# Ordered system shader constants

`build_and_upload_system_constants_00b46a70` composes the complete ordered
system-prefix body from `00B46A70` through `00B47638`. It uses the same camera,
clock, foliage, render-service and D3D9 renderer owners as its field projections.
The material builder `00B42350` and material upload `00B43541` remain subsequent
operations; their first register must refer to the actual shared boundary.

The original takes an optional scene in ECX and the camera in EDX, has no stack
arguments, and ends with plain RET. This C++ interface is a checked host
projection with additional explicit bindings, output storage and error reporting.
It is not a replacement for the native object layout or calling convention.

## Storage and execution order

The original reserves a local prefix of 77 float4 registers without clearing it.
The caller of the reconstructed interface supplies an initialized preimage.
Every unwritten component retains that preimage. Zero initialization in the
installed-mesh diagnostic is a deliberate host input, not a recovered stack or
world default.

| Stage | Native range | Main effects |
| --- | --- | --- |
| Service capture | `00B46A70..00B46A83` | Capture the live `00F8D39C` owner for its embedded matrix |
| Camera and parameters | `00B46A84..00B46C4F` | Matrices, raw eye position, sixteen live global words |
| Camera axes | `00B46C50..00B46CB3` | Shared axis cache, c31/c32 xyz; capture timer between a load and store |
| Time | `00B46CB4..00B46D96` | c33, c34, c75 xy; live singleton/service reloads; capture fog |
| Fog | `00B46D97..00B46ED0` | Scalars, colors and independently reloaded camera fog owners |
| Lighting and shadow | `00B46ED1..00B475B3` | First light, ambient cube, optional shadow matrices and dimensions |
| Context | `00B475B4..00B475FC` | Optional camera+43C float4 to c71 using raw MOVSS pairs |
| Uploads | `00B475FD..00B4762D` | VS then PS, each loading the current count and renderer |

The first service capture is the same global later reloaded by the time writer,
not an independent camera-only service. The axis stage passes its captured
timer directly to the time stage. The time stage passes its captured fog to the
fog writer, whose later reads still use the camera's live fog slot. Camera
ambient binding borrows the color from that same fog owner. The time scalar at
camera+1C4 is the existing projection FOV field; no duplicate scalar is stored.

The register holes include c4/c5, c6.w, c31.w, c32.w, c35.w, c36.w, c42.yzw,
c52.w, c69.w, c72.zw, c73.w and c75.zw. Conditional absence preserves additional
fog, lighting, shadow and context registers. The component and callback details
are recorded in [camera](SYSTEM_CAMERA_CONSTANTS.md),
[axes](SYSTEM_CAMERA_AXES.md), [time](SYSTEM_TIME_CONSTANTS.md),
[fog](SYSTEM_FOG_CONSTANTS.md) and [lighting](SYSTEM_LIGHTING_CONSTANTS.md).

An empty first-light list calls the actual invalid-parameter runtime. A returning
handler continues using the retained lighting and sentinel pointers; it can
change subsequently read mode/environment/light fields. This corrected behavior
is documented in [the handler review](SYSTEM_EMPTY_LIGHT_HANDLER.md).

## Context and upload tail

`get_camera_context_depth_scale_00b6feb0` projects the complete seven-byte
`MOV EAX,[ECX+43C]; RET` getter. A nonnull first result causes a second pointer
read, followed by four individual raw load/store pairs. A null first result
leaves c71 untouched. A pointer disappearing between reads is a checked host
binding error; the native instruction would dereference that pointer.

The VS upload reads `00E13078`, then `00F8D394`, and calls `00B21820` at register
zero. The PS upload independently reads those globals and calls `00B218C0`,
even after a failed VS HRESULT. A callback can therefore replace both the count
and renderer used by PS. A zero count has the original wrapper's no-device-work
effect. Counts beyond the supplied 77-register span and missing nonzero-upload
owners produce checked interface errors while retaining prior effects.

Material uploads use the separately reconstructed
`upload_material_entry_constants_00b43541`. It retains the original signed
counts and the late PS gate/start reads; it also continues after a failed VS
call. See [the renderer-capture review](MATERIAL_RENDERER_RELOADS.md).

## Validation and remaining boundaries

The complete integration, including the actual CRT exception, singleton lifetime
and particle lifetime modules, passed the strict MSVC Win32 build and the two
existing CTest checks. A focused local
comparison relocates the exact 122-byte context/upload tail and seven-byte
getter, changing only call targets and absolute global-slot operands. The
comparison checks all 308 output words and the COM event sequence. Its nonnull
case preserves signed-zero/NaN/infinity context bits, receives VS `E_FAIL`, then
uploads PS through a replaced renderer with count 2 after VS used 77. Its
null-context/zero-count case preserves the preimage and performs no COM calls.
Both cases passed. One additional native updater check verifies that particle
shader-time storage retains signaling-NaN bits `7F800123`, while the actual
per-sink x87 load/spill forwards `7FC00123` and raises the invalid sticky flag.

The installed-model probe calls the complete builder once, reads back all 77
registers from both shaders, and confirms that both subsequent material-tail
uploads preserve them. Its real lazy particle allocation and manager-driven
deleting destructor execute once; the singleton and manager slots are cleared.
The draw passed with two queued bindings, 2,499 nonblack pixels, 54 colors and
restored D3D9 state. The captured fixture remains very dark. Build and probe
logs are retained in `reports/system_constant_builder_build.txt` and
`reports/system_constant_builder_probe.txt`.

The subsequent [owner integration](SYSTEM_OWNER_INTEGRATION.md) supplies concrete
fog, ambient and scene-resource lifetimes and live canonical registry accessors.
The installed probe now populates an allocated fog owner through the recovered
environment fragment and verifies its camera release path. Directional-light
ownership, world/service initialization, unbound concrete virtual dispatch and
original binary ABI remain separate reconstruction work. The
installed mesh uses explicit diagnostic scene values and an initialized prefix;
its draw is an integration check, not native world or visual-parity proof.
