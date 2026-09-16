# Raw Axial particle parser (ORCH4)

The new `native_particle_axial_raw.hpp/.cpp` composes three complete native
bodies. Existing `native_particle_axial_loading.*` interfaces remain intact.
These descriptive names and source interfaces are not original symbols or
drop-in register/FH3 ABI replacements.

| Entry and inclusive body | Bytes | Coverage | Native ABI |
|---|---:|---|---|
| B064A0..B06A33 | 1,428 | Complete parser | ECX actual A4h definition; stacked TextBuffer; RET4/AL=true |
| B062F0..B06499 | 426 | Complete alignment helper | ECX definition; stacked C string; RET4 |
| B05D00..B05DC7 | 200 | Complete axis refresh | ECX definition; RET |

All three complete installed spans match live Ghidra bytes. The whole listings
were inspected, including x87 operands, stack cleanup, register captures, and
all 78 direct calls. B00CE0's current `+8` call at B00EC2 supplies the actual
definition in ECX and TextBuffer on the stack. D5DCC8 and D5DF38 both contain
B064A0; factory B00CE0 publishes final Axial profile D5DCC0 before dispatch.
The parser's two axis calls and one alignment call are the helpers' complete
current code-xref sets.

## Actual domains and retained frame

`NativeParticleAxialRawContext` borrows the existing raw builder, common
parameter, and common property contexts, the actual F8C2C8 scratch, four axis
numeric cells, and the incoming builder-kind stack word. The builder and
property contexts must use the same actual string-pool publications. Runtime
conversion uses the common parameter context's same F8D344 pool. Nested texture
loading retains its actual atlas/cache/renderer/owner domains. No host storage
adapter, synthetic renderer, alignment callback, or fallback parser is added.

The caller owns `NativeParticleAxialRawAcquired`. Its fifteen persistent words
represent native `local_48` through `local_10`. The line header is initialized
to zero; the builder kind receives the explicitly supplied native residue.
Construction leaves kind+C unchanged, and subsequent lines reuse that word.
All other locals remain sparse until their native writes.

The frame embeds an optional `NativeParticleTypePropertyRawAcquired` after its
headers. A later normal property call can replace the completed child. A failed
child is retained while the parser applies its own unwind map. An unresolved
texture/cache child, the Axial parent frame, and their borrowed contexts must
remain alive; they cannot be reset or retried. This preserves the existing
failed-child lifetime contract without allocating a generic callback frame.

## Parser schedule

The parser reads through an opening brace or EOF, then performs another line
read even if that first loop ended at EOF. It stops at a closing brace or EOF,
returns true in each normal case, and ignores non-Param and empty lines.

For each Param line, it first passes suffix 1 to raw B015C0. A recognized common
property skips Axial-specific parsing. Otherwise token 1 is the name:

| Name | Behavior |
|---|---|
| FollowDirection | Signed `atol(token2)>0` replaces the full byte at +80 |
| AxialScaleType | Raw B062F0 receives token2 |
| Other names | Parse percentage, construct/initialize builder, parse curve; ignore curve parse AL |
| Common parameter | Raw B00980 gets actual name, builder, and percentage |
| AngleElevation | First builder value spills to binary32, reloads into +84, then refreshes axis |
| AngleHeading | Same schedule into +88, then refreshes axis |
| Length / Width | Convert through actual runtime pool, multiply percentage, publish at +8C / +90 |

Percentage uses the current CRT atof result in ST0 and spills directly to the
native binary32 local before argument cleanup. Its later B00980 argument uses
the native FLD32/FSTP32 schedule, with the curve-suffix header zeroed only after
that spill. Length/Width conversion precedes the current D7A358 double load;
FLD32/FMUL64/FSTP32 writes the returned payload before publishing its pointer.
Existing pointers are overwritten without a release. A null conversion result
has no invented success path.

Each inline string return captures its native pointer, computes current
C-string length, calls the current pool getter, returns `(pointer,size,1)` via
BD1510/RET0Ch, and zeros only the headers the native code zeros. The final line
return disarms first and leaves its header unchanged. The common-parameter and
AngleElevation success arms use the original inline key-vector free/clear
schedule; remaining curve arms use AF4110. Kind+C survives both forms.

## Unwind evidence

B064A0 handler CBB880 references DF39AC; its map is DF39D0. All eight states are
entered. Each eight-byte action loads the exact header into ECX and tail-jumps
to AEE2A0, except state 5 which tail-jumps to AF4110.

| State | Previous | Local | Action |
|---:|---:|---|---|
| 0 | -1 | line, -44 | CBB840 |
| 1 | 0 | common property suffix, -38 | CBB848 |
| 2 | 0 | property name, -48 | CBB850 |
| 3 | 2 | alignment value, -30 | CBB858 |
| 4 | 2 | percentage suffix, -28 | CBB860 |
| 5 | 2 | builder, -1C | CBB868 |
| 6 | 5 | curve suffix, -20 | CBB870 |
| 7 | 6 | curve text, -24 | CBB878 |

FollowDirection's value, the leading Param token, and the atof value token have
no added caller cleanup state. State transitions precede or follow normal
returns at their recovered points. The source advances to the predecessor
before each exceptional action; a second cleanup exception terminates.

B062F0 handler CBB828 references DF3988 with one map row at DF3980, pointing to
CBB820/41DD20. Its body never arms state 0: every state store remains -1. The
raw alignment helper therefore adds no exceptional cleanup. Center, Bottom,
Top, and Left store 0/1/2/3 at +A0, then return the initially captured data
pointer using the current header length. Right and unknown store 4/0, then
invoke the current-header 41DD20 path.

## Axis arithmetic

B05D00 loads heading, then the current D5DAF8 double, retains the native x87
multiply/exchange/spill schedule, and calculates `CE3830 - scale*elevation`.
It constructs Y, X, then Z rotations using the existing raw numeric kernels;
multiplies Z*X, then that result*Y through 413920; reloads current D7A24C for
`(0,one,0)`; transforms through 4142E0; and stores +94/+98/+9C with FLD/FSTP.
The B064A0 callers consume their prior ST0 result before this call. Probe
coverage uses an empty incoming x87 stack; an artificial extra marker
failed in original native execution, and its failure instruction was not
localized. It is outside the tested precondition.

## Validation and limits

Strict MSVC Win32 `/W4 /WX` build and all three existing CTests pass after native
seed verification. The mechanical report check passes 89 call rows with zero failures. No
permanent test was added. An ignored final-library probe
passes 151 original/source comparisons: 96 parser, seven alignment, and 48 axis
cases. Native parser/alignment children bridge the genuine raw providers; the
axis comparison also executes copied original matrix/affine child bodies.
Three x87 precisions and four rounding modes cover the parser cases; the axis
cases also change the borrowed scale/base cells.

The comparisons cover full A4h owner bytes with relocated pointers normalized,
runtime curve payloads, texture records, text cursor, pool accounting, and the
full x87 status word. Inputs include all Axial-specific fields, common flags
and parameters, Shader/Layer/real-atlas Texture, unknown names, EOF without
braces, and ignored curve failure after a prior valid kind.

This is supported normal-path source evidence. Native FH3 execution, forced
allocation/provider failures, renderer cache misses in this probe, malformed
memory faults, and gameplay remain unvalidated. The report retains prior
ledger records and references `NATIVE_PARTICLE_AXIAL_LOADING.md`, the raw
parameter/property packets, and their existing evidence.
