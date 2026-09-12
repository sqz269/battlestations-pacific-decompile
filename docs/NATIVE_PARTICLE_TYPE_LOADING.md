# Native Sprite/Floating particle loading and curve bounds

Addresses: 00B08AC0, 00B07D60, 00B00980, 00AFC1C0, 00B08870,
00B001A0, 00AFFE90, 00AFFE20.

AQ implements the complete Sprite and Floating text-parser bodies, their shared
parameter-property dispatcher, integer extraction, Sprite Size publication, and
the three routines that estimate a runtime curve's upper bound. The descriptive
names are hypotheses. Original installed bytes match the saved Ghidra program
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; spans, direct call sites,
original ABIs, hashes and validation are in the accompanying JSON report.

Both parsers search for an opening brace, then process normalized `Param` lines
until a closing brace or EOF. Common properties run before curve parsing. The
percentage token is converted through the CRT and spilled to float before its
temporary strings are released. The actual 10h builder is constructed, seeded
with two endpoints, and parsed through AFC470; its result is intentionally
ignored. The native builder constructor clears only offsets0/4/8. Its initial
kind+C is explicit input and the same word is reused across subsequent lines.
This preserves malformed-line behavior without inventing an initialized kind.
Runtime conversion occurs before reading the current percentage double.
Repeated properties overwrite native pointers without disposing prior values.

Sprite clears byte64 on entry; Floating leaves it alone. Both publish
InitialRotation+80 and RotationSpeed+84. Floating publishes Size+88 directly.
Sprite publishes Size+88 first, then caches B001A0's bound at+8C, or positive zero
for a null parameter. That cache is not a sample at time zero. AF80F0 in the
separate Object packet does sample at zero and has different semantics.

B00980 handles BornRatio/TerminateAfter as the first float, the ten runtime
parameter properties through the actual shared parameter pool, and animation
frame limits through AFC1C0's CVTTSS2SI. The first five name comparisons call
the CRT without a null guard; later comparisons use the existing pooled-text
comparison. Unknown properties return false, after which the caller still
destroys its builder and property text and continues reading.

B001A0 distinguishes Const, 14h linear segments and 1Ch cubic segments by the
native type word+A. It scans to the current end-time double at00D7A220, then
scales the selected maximum by multiplier/end-time plus current00D7A210.
AFFE90 obtains candidate stationary points through AFFE20 and clamps them to
the segment span. It retains native polynomial evaluation, comparison operand
order and NaN behavior. AFFE20 returns its float discriminant, retaining both
output words when the discriminant compares negative/unordered. Its roots use
the existing genuine CRT sqrt service and current00D7A328; derivative construction
uses current00D7A2B0.

The private assembly bridge borrows these actual global pointers and CRT access
in EBX, a register unused by all three original kernels. The wrapper preserves
the caller's EBX, and the kernels retain their original ECX/EDX, stack, x87
operation and spill schedules. Current globals are loaded at the original
instruction points; no copied constant table or separate owner graph is used.
Public interfaces are new C++ bindings. Original FH3 exception ABI, invalid
storage/stack-overflow faults, binary replacement and gameplay are unvalidated.

The installed-code bound fixture passed570 comparisons covering all four bound/
Size functions, three x87 precision settings, changed globals, aliased root
outputs, signaling/quiet NaNs and complete output/status/control/sentinel state.
Its scratch executable uses an embedded manifest and links genuine repository
services. Final parser, current-library and combined-build evidence is recorded
in the report rather than inferred from these math comparisons.
