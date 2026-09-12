# Native Axial particle loading

Addresses: 00B064A0, 00B05D00, 00B062F0, 00B64640, 00B646E0, 00B64780.

The loader operates on the actual A4h Axial definition and shares the existing
TextBuffer, pooled text, NativeString, parameter builder and physical parameter
pool domains. It exposes a new C++ interface. Descriptive function names remain
hypotheses; this is not an original binary entry-point or game-validation claim.

| Routine | Coverage | Original ABI |
| --- | --- | --- |
| 00B064A0..00B06A33 | complete | ECX definition; stack TextBuffer; RET4; AL true |
| 00B05D00..00B05DC7 | complete | ECX definition; RET; incidental EAX unused |
| 00B062F0..00B06499 | complete | ECX definition; stack nonnull C-string; RET4 |
| 00B64640..00B646DE | complete | ECX destination, EDX float pointer; RET; EAX destination |
| 00B646E0..00B6477E | complete | ECX destination, EDX float pointer; RET; EAX destination |
| 00B64780..00B6481E | complete current-global overload | ECX destination, EDX float pointer; RET; EAX destination |

Each end includes the entire final RET instruction. Live Ghidra bytes were
compared with the installed executable before Capstone decoded calls, literal
operands and globals. The executable has no PE relocation directory. Capture
and probe files are under `C:/Users/sqz269/bsp-aq-axial`; the retained report
records exact span and binary hashes. Ghidra was read only in this packet, using
the configured `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` wrappers.

## Definition and text ownership

The existing B058E0 producer confirms FollowDirection at+80 as one byte,
elevation+84, heading+88, Length parameter pointer+8C, Width pointer+90, and
alignment+A0. B05D00 produces axis XYZ at+94/+98/+9C. Untouched bytes remain
unchanged; there is no typed replacement owner or whole-object reset.

B064A0 searches normalized lines for `{`, then consumes until `}` or EOF. It
returns true even when either brace is absent. Empty lines and non-Param lines
are ignored. For Param, the suffix after the first token goes to B015C0 before
Axial-specific processing. FollowDirection uses signed CRT atol >0. Alignment
constructs a real temporary NativeString and maps case-insensitive Center,
Bottom, Top, Left, Right to0..4; an unmatched or empty string stores0.

Numeric properties parse the percentage token through the current CRT atof,
construct the actual10h builder, insert zero endpoints, and parse the remaining
pooled suffix. The parse return is deliberately ignored. The incoming builder
kind+C is supplied as `initial_builder_kind_0c` and its storage is reused across
lines, just as the native stack slot is; rejected syntax must not silently
become Const. B00980 handles common parameter names first. AngleElevation and
AngleHeading publish the builder's first float and refresh the axis. Length
and Width convert through AFBF60, write the x87 float32 result of percentage
times current double D7A358 to the actual pooled payload, then publish its
pointer. Replacing an old curve pointer does not return that old slot.

One false no-return annotation hides B068CD..B068E0 in saved pseudocode. The
installed/live bytes decode as `ADD ESP,4`, three zero stores to the builder's
+0/+4/+8, then `JMP B069BF`. The common-property and elevation paths therefore
free builder storage, clear those words, release the name and continue the
loop. They do not return early. The host RAII wrapper follows this cleanup;
it does not claim original FH3 exception metadata or arbitrary invalid pointers.

## Rotation and current globals

Assembly B05D0C..B05D3A retains an x87 scale operand while spilling heading to
float, then computes the elevation expression. Both D5DAF8 and CE3830 are
double operands; treating CE3830 as float was a pseudocode-induced error.
Native stack setup at B05D43..B05D7B plus each matrix multiply's RET8 proves
the order is `(RotZ(0) * RotX(base - scale*elevation)) * RotY(scale*heading)`.
The original then writes `(0,current-one,0)` to definition+94 and transforms
it into disjoint scratch before three ordered x87 copies publish the result.

The existing exact `multiply_native_camera_matrices_00413920` and
`transform_point_004142e0` provide their actual domains here. All matrices and
the transform destination are disjoint local storage. No scene, renderer,
geometry callback or guessed angle switch is used.

The existing semantic X/Y functions in `world_entity_update.cpp` use
standard-library sin/cos and fixed constants. The new actual-storage entries
instead preserve native separate FSIN/FCOS input reloads and float32 spills,
SSE subtraction, ordered sparse stores and CURRENT D7A208/D7A24C loads.
Y writes destination+8 before loading one. Z writes cosine to destination+0
and+14 before loading one. These orders matter when a supplied global pointer
aliases output. X loads both globals before writing destination.

The four-argument `build_gui_rotation_z_00b64780` overload adds this global and
alias contract. The existing two-argument GUI entry remains unchanged. It keeps
the established Ghidra name BSP_Matrix_BuildRotationZ; this packet refines its
interface evidence rather than inventing a second recovered native function.

## Direct provider evidence

| Call sites | Provider and checked contract |
| --- | --- |
| B064D2/B064F8/B069D6 | AF5740 normalized line, actual TextBuffer cursor and shared scratch |
| B06544/B06605/B06630/B066C3/B06766 | AEE3C0 token construction in the same sized string pool |
| B065A5/B06754/B06810/B06823 | AF44C0 suffix construction and temporary releases |
| B065B2 | B015C0 real shared particle-property body |
| B067E6/B06800/B06832 | AFBED0/AFC360/AFC470 existing actual parameter builder |
| B068BA | B00980 real common parameter body, stack(name,builder,percentage), RET0C |
| B068FB/B06936 | AFC1B0 first stored float; no new curve evaluation policy |
| B0696C/B0699F | AFBF60 current physical parameter pool conversion |
| B06910/B0694B | B05D00 axis refresh after individual angle publication |
| B066D2 | B062F0 alignment string setter, stack one argument, RET4 |
| B05D3E/B05D54/B05D6D | B646E0/B64640/B64780 rotation matrices |
| B05D74/B05D7B | 413920 full4x4 multiply; RET8 consumes destination/right |
| B05DAB | 4142E0 affine point transform; RET8 consumes output/matrix |
| B06315/B0637E/B063D4/B0642A/B06457/B06482 | 41E870 actual NativeString, 425850 equality, 41DD20 storage release |

The report carries all decoded direct call-site rows, including inline pool
releases and genuine CRT boundaries. Providers were read before assigning
contracts. B05D00 and B062F0 have only B064A0 as a direct caller in the live
database. The general matrix helpers also have independent callers outside
this packet; their bodies, not those callers' names, establish their contracts.

## Verification and remaining boundaries

Full Axial source compiled with MSVC Win32 `/MD /W4 /WX /O2 /fp:strict`.
The independent native-byte math probe passed354 observations: all three
rotation helpers across nine input bit patterns, four disjoint/input/global
alias configurations and three x87 precision controls, plus30 complete axis
observations. It compared all64 matrix bytes or allA4h definition bytes and
x87 exception/status bits. Cases include signed zero, sNaN, qNaN, infinity,
large FSIN inputs and changed current scalar values. Existing shared matrix
providers were linked from the root build; copied native math callees remained
original bytes on the comparison side.

The parser probe passed28 observations: seven text cases under all three x87
precision controls and seven direct alignment strings. It compared complete
A4h definition bytes with parameter addresses normalized, all produced payload
fields and Const/Linear/Hermite segment bytes, cursor/EOF behavior, and9303
ordered string/array allocation-release events. Cases exercise both angle
orders, all common scalar/curve names, unknown properties, missing braces,
mixed-case alignments, and an ignored rejected curve after a previous Const
left kind0 in the reused stack slot. All temporary string and array storage
was released. The fixture destroys segment payloads through AFFDF0 before
returning their B00090 slots; this cleanup is separate from the loader itself.

The native side uses captured original B064A0/B062F0/B05D00 and original
matrix/transform bytes. Both sides call the same real reconstructed common
property and parameter providers. The probe therefore checks Axial orchestration
and its native math, without claiming an independent differential validation
of the shared B015C0/B00980 providers. Resource/renderer property paths were
not invoked. The primary integrator runs the repository build after merging.

A passing fixture establishes only its listed inputs and normal exception-free
paths. Original FH3 unwind,
unmasked floating exceptions, CRT-version differences, invalid or concurrently
modified storage, renderer/resource paths and game execution remain outside
this packet's runtime evidence.
