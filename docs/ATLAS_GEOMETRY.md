# Atlas to native GUI consumer boundary

Read-only bounded follow-up, 2026-09-09 UTC. Project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, image base `00400000` verified before
each Ghidra batch. No functions were created, renamed, saved or implemented.
Raw exports are retained under ignored `exports/bsp/functions/`.

## Result and next dependency

A real stateful GUI consumer is established:
`00ab44b0 -> 00aa2660 -> 00aefb20`, followed by
`00ab2690 -> object vtable+0x80` for the active state. This provides a smaller
next reconstruction than an entire GUI object: port `00aa2660` with supplied
atlas lookup, texture dimension and reference callbacks. It handles native
texture selection, atlas UV assignment, mirroring and default logical size.

**This route uses the atlas float fields directly.** It does not read item packed
words at +24h..+2eh or divide them by 65535. A packed-word decoder and a native
quad stream format are not established by this investigation. The existing
four-vertex diagnostic probe remains an adapter. Replacing its floats with
packed-word decode cannot be called native GUI parity on this evidence.

## Texture/UV resolver 00aa2660

Proposed name `BSP_GUI_ResolveTextureAndAtlasUV`. Assembly confirms four stack
arguments `(filenameString*, float4* uv, float2* size, float scale)` and `RET 10h`;
no receiver input is used, so this is a stdcall-style interface. EAX returns the
referenced texture. The filename is the engine length/pointer string pair.

The function calls `00aefb20` with global atlas manager `00f8c26c` and filename
pointer. On a match, it first records signs of input U2-U1 and V2-V1, then copies
item floats +14h/+18h/+1ch/+20h to the UV output. Negative input U extent causes
U1/U2 to swap; negative input V extent causes V1/V2 to swap. Thus caller UVs can
request mirrored atlas coordinates.

Only when **both** size components compare equal to zero, it queries texture
vtable+3ch and +40h (width/height), treats returned dimensions as unsigned32,
and writes:

- width = scale * abs(U2-U1) * textureWidth / 960
- height = scale * abs(V2-V1) * textureHeight / 720

Constant doubles at 00cec380 and 00cef1b8 were read live: 960 and 720. Texture
pointer is item+8. The function increments its reference count at texture+4 and
returns it. On no atlas match, it invokes renderer `00f8d394` vtable+64h with
(filenameString, 0), returning that result. The inspected fallback does not
assign UV/size. Assembly anchors: lookup00aa2679; UV sign checks00aa268c onward;
size arithmetic through00aa27b3; AddRef00aa27bd; success RET00aa27c9;
fallback00aa27da and RET00aa27de. x87 expressions need preserved precision/order
when reconstructed, not mechanical pseudocode transcription.

For the first selected installed item (1024-square texture and float UV
0,0,0.25,0.25), zero requested size and scale one predict logical size
256/960 by 256/720. This is a calculated expectation, not original runtime proof.

## Atlas manager lookup 00aefb20

Proposed name `BSP_TextureAtlas_FindItem`. ECX manager, one stack char pointer,
RET4, EAX borrowed item/null. It rejects null/empty name, removes a dot suffix via
reverse helper00467cf0, transforms slash separators, strips leading slashes and
normalizes case through004bcc00. Follow-up resolves the installed ASCII domain:0043bbf0 resizes at the dot
(the20h argument is a fill character),004cad40 replaces backslashes with
slashes, and004bcc00 folds only A..Z. It scans manager item array at+4/count+8 through00aee0f0: whole-name
equality is case-insensitive, while slash-delimited suffix comparison is
case-sensitive. Both alternatives apply in this first scan. If the first pass fails and a last slash is found, it retries the
basename against full stored name or a slash-delimited stored suffix. This means
basename collisions can select an earlier matching item; a filename-only map is
not an established replacement for the native ordered lookup.

## Stateful GUI transfer

`00ab44b0`, proposed `BSP_GUI_SetStateTexture`, uses ECX object plus three stack
arguments (state index, engine filename string, float4 pointer). It indexes a
0x40-byte state vector at object+f4h. A gate at object+101h controls whether it
resolves immediately. In the active resolution path, initial UVs are 0,0,1,1;
it calls00aa2660, then00ab2690 and releases its temporary returned texture ref.

`00ab2690`, proposed `BSP_GUI_SetStateTextureAndUV`, is ECX object with three
stack arguments (state index, texture pointer, float4 pointer), RET0ch. It writes
texture to state+0, AddRefs replacement and releases old texture, and copies UV
floats to state+0ch..18h. Additional flip conditions use state+1ch..28h. It
compares state index against signed16 current-state at object+ech; if equal,
it calls object vtable+80h with the index (call00ab278d). That dispatch is the
concrete next geometry/update boundary. Resolve its actual concrete vtable and
method before claiming an original quad builder, vertex stride or index order.

A separate inspected GUI class (vtable00d5c7f8) has geometry method00abced0
using `SimpleColor.mvfm`, logical stream locks and float UV writes. Its position
arrays and tessellation show that it is not sufficient evidence for a universal
four-corner icon path; it was not reconstructed. Another texture consumer
00b01350 uses `D3DXFloat32To16Array` on atlas floats: half-float conversion is a
different representation from the parser's scaled 16-bit words. These examples
reinforce that packing must be traced per consumer.

## Evidence status

Resolver, lookup and state-transfer functions are exported; key ABIs and float
field accesses are assembly-inspected. No native quad creation path or packed-UV
consumer is claimed resolved. `reports/atlas_geometry_evidence.json` records the
bounded findings and proposed names. No C++ implementation, tests, build or
visual/game validation was performed in this investigation.

Integration follow-up: the four proposed resolver/lookup/state-transfer names
and evidence comments were saved in Ghidra and exports refreshed. No GUI
consumer implementation is claimed yet.

## Resolver and lookup integration

The typed resolver in src/gui_texture.cpp and ordered lookup in
src/texture_atlas_lookup.cpp now feed the existing atlas draw. Resolver services
are supplied callbacks; the probe connects lookup to parsed records and
reference/dimension operations to its real D3D9 texture. The installed ASCII
name domain is implemented; locale-dependent high-bit names and malformed
native string storage remain outside this interface.

Win32 uses inline x87 for unsigned-dimension conversion to float32 and for
size arithmetic, preserving float32 UV-difference spills and mul/div/mul order.
The implementation does not claim original FP exception-status or ABI identity.
The portable fallback has explicitly limited precision equivalence.

Validation passed the MSVC Win32 build, both existing CTest checks and the
installed-atlas D3D9 probe. The fixture covers normalized uppercase/backslash
name lookup, basename fallback, both-zero size (0.266666681,0.355555564),
mirrored UVs, fixed size, miss preserving outputs and exactly one retain on
hits. The GPU atlas image remains produced successfully from resolved UVs.
No native differential execution or complete GUI object lifecycle is claimed.
Evidence ranges and checks are in reports/gui_texture_evidence.json.

The concrete geometry dispatch is now resolved separately in
GUI_GEOMETRY_DISPATCH.md; its cropped quad writer is the next implementation.
