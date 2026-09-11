# GUI layout-finished dispatch and widescreen correction

AA8710 now preserves its native platform-query and float-store gates. The
widescreen shift literal is corrected to the captured constant, and a live
companion adapter supplies the layout pass's final+24 operation for supported
GUI types over the same retained owner and ClipBox fields.

## Native evidence and ABI

Every live query verifies `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. AA8710 is157 bytes through AA87AC, taking ECX=widget,
with no stack arguments. It tail-jumps current virtual+24. A9E0A0 is the single
byte C3 (RET), ignoring ECX. Both saved functions exist and have no call gaps.
The descriptive name for the leaf is a hypothesis, not a recovered symbol.

Captured full code, the shift constant and six table slots match the installed
PE. The slots are Base D5C154, Group D5CBA4, Icon D5C4E4, FrameBox D5D154,
Screen D5BE5C and ClipBox D5D07C. The first five point to A9E0A0; ClipBox points
to ACE120. These targets establish live profile operations; they do not give
GuiWidgetOwner an actual native vtable word or reconstruct teardown phases.

AA8710 processes children before the current widget. At AA8746 it captures
alignment+E0. Zero alignment jumps directly to the authored-position copy and
does not read platform0109CF04. Otherwise it tests platform+0D. With widescreen
enabled,1 subtracts the double at D5C118 and2 adds it; other values skip the
position store entirely. The valid paths all use x87 FLD/FSTP, even a plain
authored-value copy, so signaling NaNs receive the native conversion. Listener
+1C, transform AA7220, bounds AA70E0 and current+24 follow in that order.

## Constant correction

Actual D5C118 bytes are `00 00 00 40 22 22 C2 3F`: double bits
`3FC2222240000000`, exactly `0.1416666805744171142578125`, and promoted float
bits`3E111112`. The former decimal literal `0.14166605472564697265625` encoded
float`3E1110E8`; its accompanying hexadecimal comment was correct but its value
was wrong. The header and both pure/full layout consumers now use the verified
value. No meaning for the fraction is inferred from that correction.

The integrator caught this when strengthening the interrupted worker's fixture:
the original-instruction side now reads the captured bit pattern independently
of the host constant. A guard failed before execution with the old decimal;
the corrected implementation passes the original comparisons.

## Live companion adapter

`GuiLayoutFinishedHost` wraps the existing transform host and forwards its
platform, listener, node-transform and bounds operations with the same widget.
The final callback resolves that transform's existing `GuiWidgetOwner` and
dispatches the supported live profile. ClipBox calls its existing ACE120 update
over its sole field set. The other five captured profiles use the verified RET.
The wrapped host's final callback is not invoked again.

Supported concrete companions are Screen1, Group2, Icon6, ClipBox16 and
FrameBox18. Type tags must agree, Group must be the exact base group companion,
and unsupported/mismatched types fail explicitly. Calls require a serialized
live layout pass with stable, nonnull child payloads and stable owner/layout
lifetime. The adapter adds no retention, native widget storage or global update
dispatcher. Native vtable changes during destruction remain unsupported.

## Validation and remaining work

The combined MSVC Win32 Release build and both existing CTests passed. One
ignored fixture performs80 original-instruction comparisons over a three-widget
tree, checking position bits, child-first call order, listener/bounds gates,
invalid-alignment store bypass and platform-query count. It includes signed
zero, subnormal, infinity and quiet/signaling NaN inputs. Align zero gives the
original code a null platform pointer to verify the skipped access.

The comparison patches external transform/bounds/listener/final callbacks to
controlled event recorders. It does not compare full floating-point status or
execute the companion adapter and real scene/ClipBox callback chain. The
adapter is source-reviewed and build-tested; runtime installation, mutable
native-list behavior, canonical widget lifetime, rendering, original binary
ABI and gameplay remain separate requirements. No permanent test was added.
