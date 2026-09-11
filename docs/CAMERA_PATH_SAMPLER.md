# Camera path sampling

`src/camera_path_sampler.cpp` reconstructs the complete local/world path
sampling bodies below. The public C++ views and resolver are new interfaces;
path, knot, direction and endpoint names are descriptive hypotheses.

| Address | Native ABI | Inclusive body |
| --- | --- | --- |
| `007B04C0` | ECX path; stack float parameter, position pointer, optional direction pointer, flags DWORD; RET10h; no useful return | `007B04C0..007B05DE`, 287 bytes |
| `007AFE80` | Same arguments and RET10h | `007AFE80..007B03B7`, 1,336 bytes |
| `007AFAC0` | ECX path; stack float parameter, position pointer, optional direction pointer; RET0Ch | `007AFAC0..007AFC52`, 403 bytes |
| `007AE200` | ECX destination; stack seven floats: from XYZ, to XYZ, parameter; EAX destination; RET1Ch | `007AE200..007AE27A`, 123 bytes |

All Ghidra queries and export requests verified the configured
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, through
`bsp.py ghidra`. Complete assembly was checked because the decompiler omits
register inputs and x87 spill details. The integrator owns the saved Ghidra
annotations and refreshed exports. Original names and comments before that
mutation, body hashes, and live/disk byte equality are in the accompanying
report.

## Actual borrowed storage

The path uses pointer-array begin/end at `+08/+0C`, parent pose owner at `+14`,
and a byte at `+24`. Every pointer-array element names the actual record. Its
first eight float words are position XYZ at `+00`, a stored vector at `+0C`,
segment start at `+18`, and duration at `+1C`. The resolver returns this actual
contiguous prefix. It cannot create a replacement record or return a copy.
The view likewise borrows the mutable pointer slots and parent slot.

Pointer ranges must be valid native-sized ranges with a representable signed
count. Empty/missing/out-of-range records call the actual CRT operation at
`00BF6713`. That operation's installed invalid-parameter callback can return;
the reconstruction then resumes the corresponding unchecked access, including
the native pointer reloads. It does not retry or invent a record. A returning
callback may repair the actual pointer fields, as exercised by the fixture.

## Local sampling

Both samplers scan from index zero for the first record where the incoming
parameter is unordered or is at most `start + duration`. The comparison uses
`FCOMIP/JBE`, with the sum still on the x87 stack. Equality chooses the earlier
segment. There is no parameter clamp, duration-zero guard, or automatic wrap
of the incoming parameter. An ordinary parameter beyond the final end reaches
the range callback.

`007AFE80` calls the linear sampler when the signed record count is below three.
The linear sampler selects the following record, or record zero after the last,
and computes `t = (parameter - start) / duration`, with a binary32 spill.
`007AE200` computes `from + t*(to-from)` with separate binary32 subtraction and
product stages. After writing position, the linear sampler re-reads the
current actual record and copies its stored `+0C` vector to optional direction.
That vector is not recomputed from the two positions.

For three or more records, the cubic sampler selects neighbors as follows:

| Selection | `+24 == 0` | `+24 != 0` |
| --- | --- | --- |
| Previous at index zero | count - 1 | count - 2 |
| Following at final index | 0 | 1 |
| Other indices | index - 1 / index + 1 | index - 1 / index + 1 |

The two tangents are the adjacent stored-vector sums, each first rounded to
binary32, multiplied by the rounded `0.5 * current.duration`. When the flags
DWORD's low byte is nonzero, index zero replaces the first tangent with
`current.vector * current.duration`; otherwise index `count - 2` replaces the
second tangent with that same current-record product. These endpoint gates are
independent of `+24`, and the first gate excludes the second.

The position is the Hermite combination of current/following positions and
those tangents. Optional direction uses the derivative of that combination
with respect to normalized `t`; it has no final division by duration and no
normalization. Position is committed first. The derivative then re-reads the
actual current/following positions, so an output overlapping a knot position
changes the derivative just as in the native body. Overlap of the two output
vectors and overlap with actual knot float storage are supported. Outputs
overlapping path pointer slots or the record pointer table are outside this
borrowed API's contract.

The arithmetic kernel preserves the observed x87 expression order, cached
stack intermediates and every float store. The doubles at `D7A280`, `D7A2B0`,
`D7A328`, and `CE6628` are verified as 0.5, 3, 4, and 6. One disassembly
ambiguity matters: `007B02D0` is `DC C9`, which means `FMUL ST1,ST0`.
Ghidra prints only `FMUL ST1`; interpreting it as the common `ST0 *= ST1`
changes the derivative. The byte-grounded destination is explicit in source.

## World sampling and integration

`007B04C0` first calls the local sampler with the actual output storage. It
captures the current parent `+14`, resolves that actual canonical pose, and
calls `refresh_pose_00414db0` only when the pose's actual `+C8` byte is zero.
Position XYZ is copied with bit-preserving loads to a float4 whose W is the
verified `D7A24C` constant 1. It is transformed by the canonical `00B62D10`
kernel through the existing `transform_camera_plane_00b65ba0` interface; that
interface adds only a final bit copy. A separately rounded `1/W` is multiplied
into all three transformed coordinates before the output writes.

For nonnull direction, the wrapper re-reads the path's actual `+14` after the
position writes, performs the same validity gate on that newly captured pose,
then uses canonical `0042D0D0` with normalization byte zero. It copies the
result back through ordered x87 loads/stores. Source/output overlap is retained.
There is no manufactured identity transform, coordinate convention or unit
direction.

The integrator can replace `CameraPositionHost::sample_path_007b04c0` in
`007954A0` with `sample_camera_path_world_007b04c0` and inherit `CameraPathHost`.
The path and knot lookups remain required actual-storage bindings. CMake and
the existing camera sources are owned by the integrator and were not edited
by this packet.

## Validation and limits

- New source compiled and linked with MSVC Win32 `/O2 /W4 /WX /fp:strict`.
- `scripts/build.ps1` passed both existing tests after `verify-seeds` enabled
  the native math differential test. Packet source registration is pending
  integration; the isolated fixture directly compiles and links the new source.
- One ignored fixture passed **9,991 isolated native comparisons**, with exact
  output/record bit comparisons under x87 precision controls 24, 53, and 64.
  It covers 1/2/3/5 records, neighbor-wrap and endpoint flags, flags `0x100`,
  finite/extrapolated/boundary/quiet-NaN/signaling-NaN parameters, optional
  direction, output/record overlap, and a returning range callback that repairs
  actual begin/end fields. The world path uses a non-affine matrix with a
  nonconstant homogeneous W and a valid canonical pose.
- The fixture relocates only verified slices and constants into its own
  allocation. It does not load or start the game. The CRT callback is a
  controlled boundary and the dirty-pose call is guarded against in this
  differential fixture. Dirty-pose behavior is grounded in the source/assembly
  and existing canonical implementation, not newly runtime-proved here.

This establishes complete bounded behavior and focused fixture agreement,
not a drop-in native ABI, authored path-loader reconstruction, gameplay or
visual validation. The timing producer and the semantic meaning of the stored
record vectors remain outside this packet.
