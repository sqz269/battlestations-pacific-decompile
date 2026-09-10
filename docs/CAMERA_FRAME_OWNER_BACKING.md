# Camera frame fields and owner slots

`CameraFrameState` now has a borrowed-storage constructor. Its scalar, matrix,
plane-set and axis references address the caller's actual fields. Construction
does not read, initialize or copy those fields. `CameraState` remains the same
stable companion shared by transform, projection, frame and shader consumers.
The default frame constructor still owns diagnostic zero values; it is not the
native camera constructor at `00B71A80`.

`CameraFrameBacking` binds bytes `174/17C`, clear fields `188..194`, render mode
`198`, inverse matrix `260`, plane records/count `2F4..437`, borrowed context
pointer `43C` and axes `440/44C`. `CameraPlaneSet` is checked as `144h` bytes,
with sixteen `14h` records followed by count at `140h`. Native construction of
these records remains a separate operation. The borrowed context is a float4
pointer; it is not one of the intrusive owners released by camera destruction.

## Viewport identity

`CameraViewport` can bind the same fields of a `NativeViewportOwner`. It neither
retains that owner nor initializes its storage. Its default/value constructor
retains the former diagnostic interface. Copy construction owns independent
values; assignment writes through the destination's existing references.

`CameraViewportSlot` borrows either a legacy view-pointer slot or the actual
`NativeViewportOwner*` word at camera `180`. A native read captures that word
and requires a stable view from `CameraViewportResolver` with exactly matching
owner identity. Null bypasses resolution. The resolver must only look up a
binding; it may not allocate, mutate fields, dispatch callbacks or retain.
A missing nonnull binding is a host error. No previous owner is cached.

The descriptor's slot address cannot be rebound. Direct view assignment is
supported only by the diagnostic slot. Actual camera owner publication must
use the separately recovered native retention setter `00B71990`, whose full
implementation remains pending. Renderer binding `00B26770` continues using
native fixed depth 0..1, independently of viewport `18/1C` depth metadata.

## Fog owner address versus field address

The native word at camera `184` holds `SystemFogOwner*`; fog fields begin eight
bytes later. Writing a `SystemFogState*` into that word would corrupt native
owner identity and reference-count access. `SystemFogSlotView` records the
actual pointer-word address and whether it contains an owner or a legacy field
view. `SystemFogSlotRef` adds writes to that same word. Copying a descriptor
keeps its slot identity and does not retain, cache or rebind the pointed owner.

The slot accepts only the already reconstructed concrete `D63180` ownership
profile for retention operations. A legacy field view passed to an ownership
API must likewise originate from that live owner; standalone diagnostic fog
fields are valid only for the existing read-only diagnostic paths.

The camera setter overload preserves `00B71940` ordering: capture old owner,
skip equal identity, publish new owner, increment new count, decrement/release
captured old owner. The clear overload preserves `[00B71F68,00B71F8A)`: the old
word remains published through destruction, then clears after the callback.
Null skips that clear store. Neither overload maintains a second refcount.

Consumers use the descriptor's address directly in the existing assembly:

| Native sequence | Preserved boundary |
| --- | --- |
| `0078D076..0078D180` environment writes | Each scalar `FLD` precedes the live owner-word load; `FSTP32` follows it. Convert the captured word to the field view after that sequence. |
| `00B46D79` system-time tail | Capture the owner word before the final c75.y conversion/store; convert the captured value after the store. |
| `00B46E69` fog color | Reload the live slot before the c37 color reads. |
| `00B46E91..00B46E9A` underwater selection | Read c37.w source, capture current owner word, then write c37.w. Resolve that captured owner after the write; do not reload afterward. |

The previous pointer-reference fog APIs remain available and delegate to the
same descriptor-aware implementation. The mesh probe's shutdown guard now
holds a descriptor of the same slot. World factory publication and frame
ambient access consequently reach the same owner without a mirrored pointer.

## Validation and limits

The strict Win32 build and both existing CTests pass. One ignored host
regression checks all borrowed frame reference identities, unchanged preimages,
write-through fields, live viewport replacement/null lookup, and rejection of
unretained native-slot publication. Its viewport/frame words are explicitly
seeded host backing; it does not claim native camera or viewport construction.

The same sequence allocates real fog owners through the shared CRT, checks
actual owner-versus-field addresses, self-assignment, replacement counts,
clear and final frees. It checks signaling-NaN x87 spill effects and exact
agreement across 320 output words for the raw-owner and legacy field-slot paths.
Finally, it places the actual raw slot at the c37.w destination: that write
changes the slot to another live fog owner, while c74 correctly uses the owner
captured before the write. This new test is a host regression; the component
native fixtures remain separately recorded in their existing reports.

The installed-asset probe still passes the shared camera checks, all 77 VS/PS
system vectors after material updates, lifetime cleanup and draw readback.
It still constructs a diagnostic camera. It does not execute a full native
camera owner, full shadow ownership, startup scene or gameplay. The unchanged
dark diagnostic render is not visual-parity evidence.

Evidence: `reports/camera_frame_owner_backing_audit.json`,
`reports/native_viewport_shadow_integration_audit.json`,
`reports/camera_state_backing_audit.json`, and
`docs/NATIVE_CAMERA_OWNER_NEXT.md`.
