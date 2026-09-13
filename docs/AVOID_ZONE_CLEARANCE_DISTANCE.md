# Selected-zone endpoint clearance

Addresses: 00415D70

Packet `orch6_zone_clearance_i`. Ghidra batches verified
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`; analysis was
read only. The descriptive name is a hypothesis, with root annotation pending.

`avoid_zone_selected_segments_clearance_00415d70` implements the complete
native schedule through an explicit borrowed `CameraAxesCrtAccess`. It clips
each selected edge and measures its nearer surviving **endpoint**. There is no
closest-point projection onto the segment interior. Negative clearance is a
real result. The runtime adapter remains a separate, root-owned integration.

| Routine | Inclusive body | Original ABI | Coverage |
| --- | --- | --- | --- |
| 00415D70 | 00415D70-0041626A; 354 instructions, zero gaps | ECX=head-word address; six stack slots; ST0 return; RET18h at00416268 | complete native instruction schedule, explicit existing CRT boundary |

The six native stack arguments are `center`, `radius`, `normal_a`, `normal_b`,
`sweep_from`, and `sweep_to_pointer`. The last two are never read. The only live
caller, `009EF910..009F00F3`, constructs all six at009EFD20-009EFD47 and calls at
009EFD4A, after the static arc test returned false. Its EDI is the address of
the navigation block's selected-list head at+0A3Ch, established at009EFCD0;
009EFD48 moves that address into ECX. The ST0 result is stored directly at
block+37Ch by009EFD4F. The selected-list cache and gameplay owner are not new
objects in this module.

The nearby `009D57E0` is an enabled-byte wrapper for **00415970**, not a helper
of00415D70:009D57E0 tests byte[ECX],009D57FE adds18h to the object pointer, and
009D5809 calls the arc routine. It returns the non-null result as bool and
consumes four stack arguments withRET10h. The independent arc module already
covers that geometry; this routine does not need it as a link dependency.

## Inputs, production and exact behavior

The existing `AvoidZoneSelectedSegment` is the actual20h record produced by
00415190 and linked by00417630/00417A40. Its endpoints occupy+0/+8, next and
previous pointers+10/+14, next-run pointer+18, and close byte+1C. The last three
bytes remain padding. See `AVOID_ZONE_CLEARANCE.md` and the existing header for
the producer evidence. No duplicate layout or substitute spatial tree is used.

00415D70 reads the head once. It squares radius with x87 and spills to float
even for an empty list, and starts the result at exact float bits7F7FFFFF
(00D7A248). Center coordinates and the first normal are initially captured;
later center-y and second-normal reloads remain in their original order.

For each edge, both endpoint projections onto normal_a are calculated relative
to center and spilled to float. At least one must be **strictly greater than
1.0**:00415E81/00415E85 compare the first againstFLD1, and00415E87/00415E8A
compare the second against float00D7A24C. Unordered comparisons do not pass.
This is not a conventional nonnegative half-plane acceptance test.

After the gate passes, a negative first projection clips the start endpoint to
the plane at projection0. Otherwise a negative second projection clips the end.
Each subtraction, ratio, scaled delta and resulting coordinate keeps its native
float spill. Zero and unordered projections follow their actual COMISS/JBE
branches. The same gate and clipping process then runs against normal_b at
00416025-0041614E. Normals are accepted as given; no normalization or epsilon is
inserted.

The squared distances from center to the two clipped endpoints are spilled at
0041619B and004161B5. Processing continues only when **either endpoint** is
ordered strictly farther than radius squared. If both lie inside or exactly on
the circle, the edge contributes nothing. At004161DC-004161EE, the second
squared distance replaces the first unless the second is ordered greater. This
also preserves the asymmetric unordered selection. The selected squared
endpoint distance is passed already inST0 to the existing recovered square-root
entry00BF7030 at004161F2. The returned root spills to float, then subtracts the
original signed radius and spills again. It replaces the current result only
when the old result is ordered strictly greater. NaN candidates cannot improve
the result; no zero clamp is present.

For example, with center(0,0), both normals(0,1), radius1, and edge
(-10,3)-(10,3), the result is `float(sqrt(109))-1`, approximately9.44030666;
the distance to the segment interior minus radius would be2. With the same
center/normals, radius5, and endpoints(0,2)-(0,10), the result is exactly-3:
the outside far endpoint passes the radius gate and the inside near endpoint
supplies the minimum. These are native behaviors, not inferred geometry policy.

Traversal uses next unless close byte is nonzero or next is null, in which case
it uses next_run. Thus actual cyclic runs terminate through their close marker;
all nonzero close-byte values are equivalent. Endpoints, links, input pairs and
padding are read only. The operation borrows live valid links; it does not own,
validate, lock or free the selected list.

## Source boundary and validation

The private source ABI places the explicit CRT access inEDX and saves it before
the originalC4h frame. Only the five original argument references shift by four
bytes:00415D76,00415D9C,00415DA7,00415DAF,00416202. The native scratch frame and
all x87 instructions are unchanged. ECX at the sqrt call is loaded from that
saved CRT pointer without spillingST0. The typed entry omits the two unread
arguments, so its private callee returns withRET10h, then the public C++ wrapper
returns the float. This is not a drop-in original ABI replacement.

The ignored fixture executes the complete original354-instruction span from the
installed PE at a verified relocated base30000000. Preparation checks each byte
against live Ghidra and records the precise absolute operand relocations; local
relative branches and the relative sqrt call remain intact. BF7030 alone is
redirected to the shared recovered ST0 sqrt. Both sides use borrowed bypass1 and
an exception handler that aborts if reached. This isolates surrounding geometry
and ordering; it does not independently compare original CRT error handling.
The native adapter is noinline so the compiler treats native XMM clobbers across
a real call boundary. Its six pushes exercise the nativeRET18h, including
different invalid bit patterns in the two unread trailing arguments.

Validation passed: Win32Release build, both existing CTest targets, all four
reported direct-call rows, and32/32 original-byte cases with32 equal result words.
The fixture checks result bits, balanced x87 stack, unchanged node/link/padding
bytes, input aliasing, both clipping directions, the two gates, endpoint-only
and negative results, exact radius boundaries, zero/signed/infinite/NaN radius,
selected NaN normals/endpoints, finite square overflow, and multiple cyclic
runs. It uses masked exceptions,53-bit x87 precision and round-to-nearest. Other
control words, original CRT diagnostics, concurrent mutation, malformed links
and mission runtime behavior are outside this proof.

Reproduce from this worktree with `python local/prepare_clearance_probe.py`,
`./scripts/build.ps1`, `cmd /c local\build_clearance_native_probe.cmd`, then
`local\clearance_native_probe.exe`. Preparation needs Python `pefile` and
`capstone`, the unchanged installed binary named byconfig/target.json, and the
verified existing Ghidra server. The build needs the existing Visual Studio
Win32 toolchain; the probe links the freshly builtRelease/bsp_core.lib and embeds
a manifest. No old worker worktree is required. Exact hashes, counts, logs,
source/build scripts, original bytes, relocation manifest and equal result
records are listed in `reports/avoid_zone_clearance_distance.json`.

## Corrections to older descriptions

| Was | Is | Evidence |
| --- | --- | --- |
| selected static segments described as a tree | actual20h linked edge runs | existing00415190/00417630 producers;0041623D-00416251 traversal |
| generic minimum distance of wedge survivors | nearer clipped endpoint distance, with either-outside radius gate |00416185-00416225; original-byte endpoint/negative examples |
| ordinary wedge half-plane acceptance implied | each normal first requires one endpoint projection strictly greater than1; interpolation plane remains0 |00415E75-00415F9F and00416019-00416138 |

No runtime files, broad geometry helpers or Ghidra functions were changed.
