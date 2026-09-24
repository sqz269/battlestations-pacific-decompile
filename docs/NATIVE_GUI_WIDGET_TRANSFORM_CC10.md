# Actual GUI widget local transform

Addresses: 00AA7220

`recompose_native_gui_widget_transform_00aa7220` covers the complete 583-byte
normal body `[00AA7220,00AA7467)`. The stored function ends with the one-byte
RET at AA7466. The original takes an actual widget in ECX and has no stacked
arguments or native EH frame. Its descriptive name is a hypothesis, not a
recovered symbol. The explicit C++ scratch/bindings interface is not a binary
replacement. It does not create or consult a logical `GuiWidgetOwner`.

The actual widget producers establish the consumed layout. AA9390 clears
position at +0C/+10/+14, pivot at +18/+1C, size at +20/+24, rotation at +48,
and node at +4C; it copies the borrowed one word to scale +28/+2C. AA6720
publishes the actual node at +4C. The direct setters AA78D0, A9E0B0, AA7950,
AA7970 and AA7930 confirm those fields, and AAA710's descriptor rows name
Pos, Pivot, Size, Scale and Rotate.

The source retains the original mixed x87/SSE sequence:

- Load the current one word first with MOVSS. Load position Y and the current
  E12FC4 operand into x87; duplicate that operand, multiply Y, and retain the
  other copy on the x87 stack through the later pivot-Y calculation.
- Read position X/Z as raw MOVSS words. Rotation is `SUBSS` of current
  D7A208 minus the current widget +48 value, not an abstract unary negation.
- Pivot X is FLD/FCHS/FMUL size-X followed by a float32 spill. Pivot Y uses
  FLD/FCHS/FMUL size-Y and the still-live original y-scale, then its own spill.
- Initialize scale, pivot and translation matrices with the native MOVSS
  order. Positive zero comes from XORPS; one is the earlier captured word.
- Call the four-argument current-global B64780 provider, then the existing
  exact raw 413920 three times: `((pivot * scale) * rotation) * translation`.
  The first helper retains FSIN/FCOS spills and its separate later global
  reads. The naked multiplication entry retains native x87 caches and aliases.

Saved Ghidra and installed PE words currently agree on D7A24C=`3F800000`,
D7A208=`80000000`, E12FC4=`3F400000`. The bindings borrow the actual current
cells, so those initial values are not substituted as constants.

Let S denote native ESP after saving ESI and EDI. The caller's 1DCh-byte
scratch begins at S+8. All offsets below are relative to that scratch:

| Offset | Storage |
|---|---|
| 00 / 04 / 08 | rotation angle / pivot-X spill / pivot-Y spill |
| 10..4F | scale matrix |
| 50..8F | pivot matrix |
| 90..CF | translation matrix |
| D4..D7 | separately retained position-Y spill |
| DC..11B | final matrix |
| 11C..15B | second product |
| 15C..19B | rotation matrix |
| 19C..1DB | first product |

The holes C..F, D0..D3 and D8..DB remain untouched. Supply initialized
caller-owned bytes and keep their addresses stable. Matrix storage and field
access remain raw; no input snapshot, blanket scratch initialization, alias
repair or finite-number checks are introduced.

Dispatch has two distinct captures. AA7255 reads the initial widget+4C node;
AA72C2 reads that node's vtable. After the math calls, AA7455 reads the current
widget+4C receiver, and only then AA7459 reads target +38 from the earlier
vtable. AA745C passes the actual final matrix pointer with ECX=current receiver
and one stacked pointer; its native target must consume that argument (RET4).
The required `NativeGuiWidgetTransformDispatch` binding receives exactly the
target DWORD, current receiver and matrix pointer. It supplies no default,
null guard, forced B6DB10 target, logical-owner substitute or no-op.

`set_raw_local_matrix_00b6db10` is an available concrete provider for a binding
that resolves that target. Its canonical profile resolver remains necessary
when attached-node notifications require it. Other installed targets must be
implemented by the actual application dispatch binding. This packet does not
claim their behavior or install itself into the application.

Validation: all 583 body bytes and the 159-byte B64780 provider span matched
saved Ghidra against PE. Strict Win32 `/W4 /WX /fp:strict` and all three
existing CTests passed. One ignored `/MD` probe with an embedded manifest
passed six original/source pairs. It compared all 476 scratch bytes, callback
matrix words, actual raw B6DB10 node effects, masked x87 flags and MXCSR under
nearest/down rounding. It covered zero/denormal/NaN operands and nondefault
borrowed globals. Two genuine storage aliases prove dispatch order: a widget
at scratch+138 has its +4C receiver changed by rotation while the old vtable
is retained; a vtable at scratch+14C has its +38 target changed by rotation
after capture. No synthetic math-provider mutations are used.

The copied original calls B64780 and 413920 were rebound to the same existing
production providers. Its virtual target invokes the same raw B6DB10 provider
through the probe's explicit binding. This proves this caller's orchestration
for those cases, not an independent callee differential, unmasked-fault,
native ABI, game or visual result. No tracked tests were added. Exact call
rows, hashes and the frozen local archive are recorded in
`reports/native_gui_widget_transform_cc10.json`.
