# Native GUI group bounds

`set_native_gui_group_bounds_00b8e6c0` writes the existing live
`NativeGroupOwner` allocation. `gui_screen_root_radius_00ac5f00` supplies the
numeric operation required by the Screen adapter using the existing CRT sqrt
implementation and the actual conversion-mode global. Neither creates a bounds
owner, duplicates native storage, or substitutes a constant radius.

## Native bodies and storage

| Address/span | Original ABI | Boundary |
|---|---|---|
|00B8E6C0..00B8E6EB,44 bytes|ECX group, stack float32[4] source, RET4|Complete direct explicit-sphere writer|
|00BF7420..00BF743B,28 bytes|ST0 input, EAX signed32 or EDX:EAX signed64, RET|Library selector/SSE2 path; existing00BF7456 supplies fallback|
|00AC5F00..00AC5F43,68-byte probe span|interior of ECX Screen acquire78|Numeric schedule and final bounds call; only numeric operation exposed separately|

Every live batch used the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The complete writer and conversion selector have
zero listing gaps. The report records old Ghidra names/comments, exact spans,
hashes and installed-image matches. Ghidra remained read-only; ledger names are
pending annotation. `BSP_Group_SetExplicitLocalSphere` is a descriptive
hypothesis. Existing CRT library names are preserved.

The decompiler misrepresents the writer as four integer assignments. Assembly
is12 instructions: clear node+138 mask0x30, load the source pointer, write group
byte+175=0, then four **FLD float/FSTP float** pairs, in forward order, into
node+08/+0C/+10/+14. These are not raw copies. Signaling NaNs can be quieted,
denormals and invalid operands affect x87 status, and unmasked faults follow
the current hardware policy. An overlapping source is reread after each prior
store. A source overlapping+138 sees the already-cleared flag word. The naked
internal kernel preserves that exact instruction schedule; the public C++
wrapper verifies that node/tail views belong to one live native cGroup.

The writer calls no helper, current virtual or enclosing-owner notification.
It must not be replaced by `notify_native_group_bounds_00b6dbc0`, which clears
a different mask and can notify an enclosing owner. It leaves transform cache
flags+5C, world sphere+13C..148 and every untouched group-tail byte intact.

The byte+175 interpretation is grounded in read-only00B8F100: zero selects the
supplied local sphere. When+138 mask0x30 is clear, that getter refreshes an
invalid world transform if necessary, transforms local+08..17 into world
+13C..148, then sets mask0x30. Nonzero+175 instead invokes child aggregation
00B8EBE0 when the same cache mask is clear. This packet implements neither
getter nor aggregation; clearing the mask schedules their existing lazy work.
The group constructor initially writes+175=1.

## Radius and shared CRT entry

The numeric adapter loads the actual double at00D7A308 (installed value2.0),
calls the existing CRT sqrt kernel, spills its result to float32 and reloads it,
then reads the actual supplied DWORD0109EEA4. A nonzero value selects the
native FSTP-double/CVTTSD2SI-EAX path. Zero transfers to the existing public
`native_x87_truncate_st0_00bf7456`, preserving its x87 conversion/status behavior
and consuming only EAX afterward. CVTSI2SS interprets that EAX as signed32.
The returned float supplies the Screen's final sphere lane. No `std::sqrt`,
C++ float-to-int cast, cached selector or constant1 replaces these operations.

`native_crt_sqrt_st0_00bf7030` is a narrow public forwarding entry in
`system_camera_axes`: ECX supplies the existing `CameraAxesCrtAccess`; input
and output remain in ST0. It saves EBX, binds that existing access, calls the
unchanged private CRT kernel and restores EBX. It adds no floating operation or
spill. The original vector/axis callers and all CRT body/handler code remain
unchanged. The supplied actual0109DD78 binding and87-exception handler retain
the existing CRT contract. `GuiGroupBoundsCrtAccess` adds the actual0109EEA4
address, read after sqrt, so a service-side change is not hidden by a snapshot.

The C++ float return adds only an exact load of the already-spilled finite
integer-derived float. These interfaces are semantic adapters, not drop-in
Screen ABI replacements. They retain the original FP environment rather than
resetting it around operations.

## Focused validation

MSVC Win32 Release `/W4 /WX` and both existing CTests passed after all eight
native seed spans matched the installed executable. One ignored fixture under
`local/gui_group_bounds_probe.cpp` was compiled with `/MANIFEST:EMBED`.

The fixture creates a real group owner using existing pool/type/node services.
It executes the verified original44-byte writer on a copied18Ch allocation,
then the reconstruction on the canonical owner. Full allocation bytes and full
x87 status match for one external sphere containing negative zero, a signaling
NaN and a subnormal, plus two overlapping sources (including flags+138). The
entire comparison includes untouched base/tail bytes and the trailing pool ID.

For radius, the probe executes the original68-byte GUI caller fragment and
the original conversion block, relocating its actual global pointer and calls.
It compares that against the adapter followed by the reconstructed writer in
eight combinations: two selector values and four x87 rounding modes, using
double2.0. Full group bytes, x87 status, MXCSR and preserved control words
match; the radius is1.0 in these cases. Both paths use the **same existing
reconstructed sqrt dependency**. This proves the bounded caller/conversion
composition under those conditions, not new original-CRT equivalence or
unmasked-handler behavior. No rendering or game validation is claimed.

## Read-only frontend correction for the integrator

`include/bsp/frontend_entry.hpp:81,90..100`, `src/frontend_entry.cpp:110`,
`docs/GAME_FRONTEND_ENTRY.md:144` and the old0057BEC0 comment incorrectly call
00BF7420 a clock/counter and describe a dead128 multiply/x87 stack leak.
Live0057BED5 loads original progress;0057BED9 loads the old value; FCOMIP pops
the old value and leaves **original progress** in ST0. The SSE path independently
chooses the maximum float.0057BEEF multiplies ST0 by128 and0057BEFA converts and
consumes it through00BF7420. Thus+2C takes the signed maximum of its old value
and the conversion of original progress*128. The multiply is used and ST0 is
consumed. Those files/annotations belong to a separate packet and were not edited.
