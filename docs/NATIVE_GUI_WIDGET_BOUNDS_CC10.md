# Raw widget local bounds

Addresses: `00AA70E0`; existing providers `00B74640`, `00B732C0`, `00B855B0`.

`refresh_native_gui_widget_local_bounds_00aa70e0` reconstructs the complete normal
139-byte body over the actual widget storage. Original ABI: ECX widget, plain RET
at `00AA716A`; inclusive body `[00AA70E0,00AA716A]`. Source adds caller-owned
`GuiWidgetBounds` float4 scratch and a borrowed `const volatile double&` for the
live `00D7A280` constant. It does not create or cast a logical `GuiWidgetOwner`.
Descriptive names remain hypotheses, not recovered symbols.

## Raw providers

| Call site | Native target | Actual storage contract |
| --- | --- | --- |
| `00AA70F3` | `00B74640` | ECX model, stacked unused DWORD, RET4; return model+180 |
| `00AA70FA` | `00B732C0` | ECX geometry, stacked index, RET4; return array at geometry+54 indexed by 0 |
| `00AA7161` | `00B855B0` | ECX element, stacked float4, RET4; sequential x87 stores to element+24/+28/+2C/+30 |

The two zero pushes before the first call are separate arguments: the inner zero
is consumed by B74640, and the outer zero remains for B732C0. Source calls the
existing genuine raw providers in `gui_widget_owner.cpp`. Its B74640 interface
takes `NativeModelTailStorage`, which begins at actual model+174; its geometry
field at tail+0C is exactly model+180. Widget+4C is read as the current actual
model address. There is no node registry lookup, host geometry substitution,
retain/release, allocation, callback, null fallback, or missing function provider.

## Arithmetic and memory order

Byte widget+74 is the sole gate. Zero returns before accessing the model chain,
dimension fields, half constant, or scratch. Any nonzero byte takes the full path.
The model and element are resolved before the first width load.

The native listing prints both multiplications as `FMUL ST1`, but their bytes
identify opposite destinations:

| Site / bytes | Exact instruction and effect |
| --- | --- |
| `AA710B: DC C9` | `FMUL ST(1),ST(0)` multiplies width by half while keeping half in ST0 |
| `AA711E: D8 C9` | `FMUL ST(0),ST(1)` multiplies height by the retained half |

Width loads from widget+20, then the double at D7A280 loads once. The observed
double is `0.5` (`000000000000e03f`), but source reads the actual borrowed binding
each enabled call. XORPS creates positive zero. Scratch writes are z at +8, x at
+0, then y at +4. Separate FLD/FSTP pairs reload width and height from widget+20/24
into comparison spills. FCOMIP compares width with height; JBE selects height for
ties and unordered values. MOVSS selects the exact float32 spill, the retained
half multiplies it, and the radius is spilled to scratch+C.

Only after that does B855B0 perform its four ordered x87 transfers. Element/widget
overlap is supported without early output stores or input snapshots. Caller
scratch represents the original automatic local frame and must be distinct from
widget, model, geometry, element, and the half binding. It has no imposed initial
value; the disabled path preserves its entire preimage. All other actual widget,
model, geometry, and element bytes remain untouched unless they alias those four
final output DWORDs.

## Verification and limits

The live project/program was `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. All 139 function bytes and the original provider
bodies (9, 13, and 30 bytes) match the installed PE. No Ghidra write occurred.
The report records exact inclusive boundaries, byte hashes, and all three call
rows. Existing provider symbols and comments remain unchanged.

`scripts/build.ps1` passed the strict MSVC Win32 build and both enabled CTests.
One ignored probe, `local/output/cc10_bounds_probe.cpp`, executes the relocated
139-byte original and all three original leaf bodies with no primitive bridges.
It relocates the three calls and the actual double address only. Its exact
compiler command, source/executable hashes, and assertion guard are in the report.
`NDEBUG` is rejected at compile time.

168 original/source pairs compare the full actual widget/model/geometry/element
context byte-for-byte, source float4 scratch, x87 status, and MXCSR across three
x87 precisions and four rounding modes. The focused cases include ordinary
values, finite ties, both signed-zero ties, NaN on either side, masked signaling
NaN, infinities, subnormals, live half changes, overflow, element/widget alias,
and gate zero with an invalid model address. Gate zero also preserves scratch.

This is a source interface for valid actual storage, not a binary ABI hook.
Enabled calls require the current model tail, geometry pointer array and writable
element to exist. Scratch must satisfy the distinct-frame contract. The probe
masks FP exceptions; unmasked trap delivery, CPU EFLAGS/register identity, full
machine-state parity, application binding, and game behavior are not claimed.
