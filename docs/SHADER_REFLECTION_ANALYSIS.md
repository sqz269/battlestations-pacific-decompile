# Pixel shader component usage producer

Read-only bounded investigation of `00b61280`, with project `bsp`, program
`/battlestationspacific.exe`, x86 LE32 and image base `00400000` verified through
`tools/ghidra_export.py` Client.verify before every batch. Existing source exports
were refreshed in ignored `exports/bsp/functions/`. No C++, shared metadata,
Ghidra names, project saves or game files were changed.

## Contract established for filtered interpolators

Proposed name: **BSP_PixelShader_CompileAndCollectInputUsage**, address `00b61280`.
This routine compiles a pixel shader, creates the native D3D9 pixel shader object,
and optionally parses compiler disassembly text for declared input components.
It does not use a D3DX constant table: the final constant-table output argument
to D3DXCompileShader is null.

Assembly confirms a fastcall-style interface:

| Location | Argument |
|---|---|
| ECX | Name/debug-key engine string object (length at+0, char pointer at+4) |
| EDX | Shader profile C string |
| Stack+4 on entry | Source C string |
| Stack+8 | Pixel-shader output pointer |
| Stack+0c | uint32 TEXCOORD component-mask array, or null |
| Stack+10 | uint32 COLOR component-mask array, or null |
| Return | EAX HRESULT-like status, RET 10h |

The two masks are **arrays of one DWORD per semantic register**, each DWORD
containing low four component bits. They are not byte arrays, packed nibbles,
field-local masks, register counts or D3DX structures. Caller `00b3b3c0`
initializes ten TEXCOORD DWORDs at its frame+0a8h..0cch and two COLOR DWORDs at
+78h/+7ch to zero (assembly00b3b625..00b3b66f), then passes them to00b61280 at
00b3b6c1. The same arrays become the two optional filter arguments of00b36800 at
00b3b910. This is direct producer-to-consumer evidence for the filtered API.

The producer never clears arrays or receives capacities. Repeated declarations
OR into existing words. The caller supplies the zero-initialization. In the
consumer, flattened component index `k` addresses DWORD byte offset `k & ~3`
and component bit `1 << (k & 3)`, consistent with register word `k / 4`.

## Compilation and extraction sequence

Helper `00b1fef0` returns renderer+1a10h (the D3D9 device pointer); renderer is
global00f8d394. This getter is ECX input and EAX output.00b61280 passes source,
strlen(source), null macros/include, entry `main` (live constant00d582a0),
profile from EDX, flags, compiled/error buffer outputs, and null constant-table
output to imported D3DXCompileShader (call00b6132b via00c2e00a).

Compile flags are zero if the name's char buffer contains lowercase `shore`
using case-sensitive strstr; otherwise 0x1400. Their SDK symbolic decomposition
was not needed for the usage-layout finding and was not independently checked.

When compiled buffer exists, buffer vtable+0ch provides bytecode and device
vtable+1a8h creates the pixel shader (call00b61358). Output-pointer setup is
visible before the GetBufferPointer call; pseudocode misleadingly attaches the
output parameter to that getter. The actual device call has device/bytecode/out
arguments, as the preserved stack pushes show.

If either usage pointer is null, extraction is skipped. With both nonnull,
D3DXDisassembleShader receives bytecode, color-code=false, comment=null, and an
output buffer (call00b6139e via00c2e004). Native file service0109ceec then writes
that disassembly to `shaderfx/debug/<name>.psa1`, or `.psa2` for the sentinel path,
using service vtable+4 open mode0x35 and file vtable+5ch. This write is a native
side effect found in assembly; it was not executed in this investigation.

If TEXCOORD[0] equals decimal500, parsing is skipped after disassembly/write.
The second caller at00b3be30 deliberately passes the same temporary pointer for
both arrays with first DWORD500. Thus500 is a control sentinel for this producer,
not a component usage value to feed to the filtered selector.

## Exact text-to-mask behavior

Starting at scan offset zero, the routine searches case-sensitively with strstr
for `dcl_texcoord`. It extracts exactly one character at match+12, passes that
substring to CRT atol, and uses the result as the TEXCOORD array index.
It slices from match+14 to the next LF and scans that substring independently
for lowercase `x`, `y`, `z`, and `w`:

| Character present | OR mask |
|---|---|
| x | 1 |
| y | 2 |
| z | 4 |
| w | 8 |
| None of the four, or null substring | 15 |

Then it resumes the search at the LF position. The COLOR loop is analogous,
searching `dcl_color`, reading one index character at match+9, and inspecting
the substring from match+11 to LF. There is no bounds check on the index and no
lexical recognition of the declaration operand or swizzle. It searches letters
anywhere in that sliced line. Multi-digit semantic indices are not fully parsed.
This is a narrow parser for the compiler's expected disassembly spelling, not a
general shader assembly parser.

Assembly anchors: TEXCOORD search00b61670, one-character index slice00b6168d..9c,
atol00b616ae, line-end search00b616ee, component ORs00b617b5/00b617d7/
00b617f9 and00b6181b, full-mask fallbacks00b6182f/00b61843. The COLOR loop begins
00b61850 and reaches matching ORs/fallback through00b61a51. Absent LF and
malformed substring handling depend on generic engine substring00469840;
they are not a proven safe failure contract. Ordinary compiler lines such as
`dcl_texcoord0 v0.xy` yield TEXCOORD[0]|=3 and `dcl_color1 v1` yields COLOR[1]|=15.
These are deductions from the inspected instructions, not executed fixtures.

## Status and ownership caveats

Compilation failure with no compiled buffer returns the compiler HRESULT.
Successful shader creation saves its HRESULT, but requesting disassembly
replaces that saved result with D3DXDisassembleShader's HRESULT. Therefore a
single success code cannot stand for all three operations. There is no visible
failure/null gate before dereferencing the disassembly buffer or opened native
file. The output shader is not released by this helper. The compiled buffer,
disassembly buffer, and compilation message buffer have COM Release paths.
Failure cleanup paths and engine exceptions have not been reconstructed.

The first caller checks its output shader pointer rather than trusting just the
status. The two compile passes, generated source regeneration and selected-field
arrays surround this helper; that larger shader builder lifecycle is not ported
by this analysis.

## Recommended integration boundary

Feed00b36800 explicit ten-word TEXCOORD and two-word COLOR arrays initialized by
the caller. A future reconstruction can separate the deterministic declaration
text parser from compiler/device/file services through explicit callbacks,
retaining native index/OR semantics for compiler-produced text. Any added length,
index, malformed-line or null checks should be labelled adapter policy. Do not
import the native file writes merely to claim mask extraction parity, and do not
substitute a D3DX constant table as evidence for this input-usage path.

At this handoff00b61280 is exported and assembly-inspected, not reconstructed,
build-tested, native-fixture-tested or game-validated. Only this document was
added to tracked source scope; the parent owns00b36800 implementation/integration.

## Parser integration follow-up

The parent ported the parser fragment as `parse_pixel_usage_00b61280`. Owning
C++ vectors preserve OR behavior and the500 sentinel; malformed lines without LF
and out-of-range indices fail without changing outputs. Embedded NUL terminates
the text. These are explicit bounded interface choices, not native failure ABI.
The existing probe now disassembles its compiled PS3 with D3DDisassemble, obtains
COLOR0=15, selects two output fields and regenerates the VS3 before drawing.
CenterFF407FBF/outsideFF000000 and state restore pass. Win32 build and both existing
CTests pass; no new test target. This validates one full-color input, not sparse
components, the native D3DX output format or full two-pass builder execution.
The full00b61280 remains unported; only the parser is counted as a fragment.
