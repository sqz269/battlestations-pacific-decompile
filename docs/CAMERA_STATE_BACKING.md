# Camera state borrowed backing

`CameraProjection` and `CameraState` can now refer to the fields and existing
transform/projection views of one actual camera. Binding preserves every owner
byte. Default construction still supplies zeroed diagnostic values; these are
not native camera constructor defaults. This packet does not construct a raw
camera, retain an owner, allocate a viewport, or change native ownership.

The substantive native correction is in `get_camera_projection_00b6fcf0`.
Its previous `&camera.fov + N` loads assumed that four projection inputs occupied
consecutive words. That matched the former diagnostic struct but not the actual
camera: two unrelated words occur between aspect and near plane. Four independent
field pointers now preserve the assembly's far, near, aspect, FOV load/spill order.

## Binding contract

`CameraProjectionBacking` takes mutable references in this order:

| Field | Native offset |
| --- | --- |
| `fov` | `1C4` |
| `aspect` | `1C8` |
| `near_plane` | `1D4` |
| `far_plane` | `1D8` |
| `original` | `1E0`, 64 bytes |
| `cached` | `2A0`, 64 bytes |
| `valid_flags` | `2F0` |

The explicit constructor `CameraProjection(backing)` binds those fields without
reading, initializing or caching their values. In particular, it does not touch
`1CC/1D0`, the inverse matrix at `260`, or the extra plane at `2E0`.

The borrowed `CameraState` constructor is:

```cpp
CameraState(CameraTransform& transform, CameraProjection& projection,
    CameraMatrix& view_projection, std::array<float, 3>& direction,
    std::array<float, 3>& target) noexcept;
```

`view_projection`, `direction`, and `target` identify native `220`, `1AC`, and
`1A0`. The constructor retains references to the caller's existing transform and
projection views. Callers must supply a coherent set from one actual camera and
keep every view and its storage alive. There is no independent transform refresh
state, cache flag copy, pointer ownership, runtime type dispatch, or destruction
callback in these companions. The directional semantic names remain provisional.

Both classes use the established `CameraTransform` value semantics. Copy and move
construction own independent values; moving leaves the source's bindings intact.
Copy and move assignment write through the destination's existing references and
never rebind them. `CameraState` assignment also delegates through the existing
transform and projection assignments. These are C++ interface semantics, not
recovered native camera copy constructors or assignment operators.

## Native evidence

The owner layout is established in `docs/NATIVE_CAMERA_OWNER_NEXT.md`, discovery
commit `7aacfa0`, particularly the constructor's explicit `1CC/1D0` stores and
untouched matrix caches. This packet independently reread the getter assembly and
compared its complete closed execution slice against live Ghidra bytes and the
installed executable. Every CLI batch targeted `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; no Ghidra or ledger mutation was performed here.

| Span, end exclusive | Bytes | Role |
| --- | ---: | --- |
| `00B6FCF0..00B6FD60` | 112 | Modified getter; ECX camera, EAX `camera+2A0`, plain `RET` |
| `00B642F0..00B643A0` | 176 | Existing projection builder; ECX destination, four stack floats, `RET 10h` |
| `00412E20..00412E33` | 19 | Existing x87 tangent helper; one stack float, ST0 result, `RET 4` |
| `004134F0..00413557` | 103 | Existing sequential x87 matrix copy; ECX destination, stack source, `RET 4` |
| `00D7A24C..00D7A250` | 4 | Native float one |
| `00D7A280..00D7A288` | 8 | Native double half |

All 422 bytes match; exact disk offsets and SHA-256 hashes are in
`reports/camera_state_backing_audit.json`. The getter tests bit `8`, then loads
`1D8`, `1D4`, `1C8`, `1C4` with an individual `FLD/FSTP32` for each argument.
It builds a temporary matrix, copies temporary to `1E0`, copies `1E0` to `2A0`,
ORs bit `8`, and returns the actual cache address. A set bit skips all scalar
loads and returns that same address. Existing builder/copy arithmetic is unchanged.

## Validation and limits

`local/camera_backing_native_check.cpp` is one focused regression scenario with a
typed `2F4h` camera prefix at native offsets. It reserves an original-relative
image with inaccessible unused pages, commits only the six spans above, and
rebases two absolute constant operands. All calls stay within the four original
functions. It loads no PE entrypoint/imports, installs no hooks, and supplies no
unresolved-call stubs.

For x87 control words `027F`, `067F`, `0A7F`, and `0E7F`, the scenario runs a dirty
projection, a cache hit after changing scalar fields, and another dirty projection
with a signaling NaN near plane. It compares all 756 owner bytes after each call:
12 states and 9,072 matching bytes. It also compares masked x87 status/control
words and checks the exact returned cache field address. Signaling NaNs in both
intervening words remain untouched and cause no invalid exception; a cache hit
does not load a signaling NaN FOV; the dirty near input does raise masked invalid.

The same local executable checks every supplied reference identity, untouched
owner preimage on binding, actual target/direction writes, cached view-projection
identity, independent copy/move construction, assignment through existing backing,
and unchanged diagnostic zero defaults. Its strict MSVC Win32 `/W4 /WX /fp:strict`
build and execution passed. One fixture-only narrowing warning was corrected;
the production changes passed on the first build. `verify-seeds` passed all eight
spans, and `scripts/build.ps1` passed both existing CTest cases.

No new permanent test target or broad test suite was added. This evidence does not
execute native camera construction/destruction, native transform hierarchy or
frame/fog integration, unmasked x87 traps, concurrent mutation, or arbitrary
overlapping bindings. The companions are not drop-in native object ABIs. No game
or visual rendering validation is claimed.
