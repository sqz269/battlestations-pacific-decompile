# Native shader descriptor loading boundary

Bounded read-only investigation, 2026-09-09. Client.verify() verified project
`bsp`, program `/battlestationspacific.exe` before every Ghidra batch. Raw
pseudocode and assembly are in ignored `exports/bsp/shader_descriptor/`.
No metadata, Ghidra names/state, C++, installation files or runtime were changed.
Function descriptions below are reconstruction hypotheses except existing named
functions. No shader descriptor was executed during this investigation.

## Actual runtime

The executable contains `Lua 5.1` at 00d57fd0 and the identification string
`$Lua: Lua 5.1.1 Copyright (C) 1994-2006 Lua.org, PUC-Rio $` at 00d57fe8.
Its PE import directory contains no Lua DLL. Calls enter internal Lua code in
the 00a6xxxx range: this is an embedded Lua 5.1.1-labelled runtime, not a DLL
that the diagnostic host can simply load. The string does not prove the
runtime has no local patches.

These internal routines use register arguments, unlike a stock public Lua C
API build. A host using an actual Lua implementation should call its own API;
it must not assume binary interchangeability with game lua_State or callbacks.

| Address | Assembly-backed interface / interpretation |
|---|---|
| 00a6a260 | Create state helper; returns pointer in EAX, calls 00a68950 and installs a panic handler through 00a67390 |
| 00a67390 | Panic handler setter; caller passes ECX=state, EDX=handler |
| 00a6a160 | Load buffer helper: ECX=state, EDX=bytes, stack length then chunk name, RET8; calls 00a681f0 with one-shot reader00a6a140 |
| 00a68090 | Unprotected call helper: ECX=state, EDX=argument count, stack result count, RET4; calls00a69580; -1 requests all results |
| 00a680e0 | Protected call path used for initialization strings; exact full error contract not reconstructed here |

Do not reimplement a partial Lua grammar for shfx files. They are executable
Lua chunks, with global includes, expressions, tables and long strings.

## Reader control flow and ABI

`00b43b00` / existing `BSP_ShaderDescriptor_ReadShaderTable` takes
ECX=descriptor, stack path-string object then unsigned profile generation,
and ends RET8 at 00b44682. The decompiler's single-argument prototype and
`unaff_retaddr` profile input are incorrect; assembly shows two stack args.
The source path object is the native string pair (length, data pointer), not
a raw C string. The descriptor is allocated as 110h bytes by caller00b45ee0.

1. Construct local script wrapper through00b66bd0 (owned state flag+0,
   lua_State pointer+4, further stack/reference bookkeeping).
2. Initialize it through00b6a020 with library mask1.
3. Execute source through00b69d40(path, decodeFlag=0).
4. Obtain the Lua globals reference through00b67980, whose index is
   FFFFD8EEh (-10002, Lua 5.1 globals pseudo-index).
5. Look up global `Shader` through00b67800 and read descriptor fields.
6. Release table references through00b67700 and destroy script wrapper
   through00b669a0.

Both cited call sites 00b45f5e and00b46104 belong to00b45ee0. The first
reads the main descriptor into material+C4h; the second reads a combiner
into a fresh descriptor. Both pass generation3. The containing routine
handles 14 combiner slots and later passes descriptors to00b3c3a0 for
shader generation/creation. This is additional work beyond reading Lua data.

## Initialization and DoFile

`00b6a020`: ECX=script wrapper, stack library mask, RET4. It creates state,
installs panic handler00b669c0, and opens selected libraries from the table
at00d62bb8 (name/function pairs): base00a67210, package00c2ff90,
table00a66010, io00a652b0, os00a64190, string00a63910,
math00a61c50, debug00a61620. The base entry is always opened; mask1
therefore opens only the base library on this reader path.

It executes `PC=true`, sets X360COMP to true/false from0108ff20, and may
set REGION from0108ff28. It registers global **DoFile**, case-sensitive, as
native callback00b69e00. Finally it loads a shared buffer obtained through
00884770, using chunk name `Scripts\fundamentals.lua`, and calls it with
zero arguments and all results. The installed fundamentals file defines
platform-selection functions. The provenance and lifetime of the cached
00884770 buffer remain outside this trace; installed-file identity is not
proof it is the live cached buffer.

`00b69e00` wraps the calling state without taking ownership, converts its
first Lua argument to a filename, calls00b69d40 in the same state, releases
references, and returns zero Lua results. Its callback state is passed in ECX
in this native build. A stock Lua runtime adapter needs its own correctly
compiled C callback, not a direct pointer to00b69e00.

`00b69d40` first calls00b66ca0 for the requested file, asks the file manager
through00bdef90 for additional content filenames, then executes returned paths
in order with the same decode flag. Those content/overlay resolution rules
are not fully reconstructed. DoFile delegates to this same route rather than
opening an arbitrary OS-relative file directly.

`00b66ca0`: ECX=script wrapper, stack native path string then decode flag,
RET8. Uses file-manager global0109ceec virtual+4(path,2); checks the returned
file's virtual+18h and nonzero virtual+30h size; allocates and reads bytes with
virtual+24h; optionally decodes; loads bytes with00a6a160 using the path as
chunk name; releases the file; calls00a68090(0,-1); frees bytes. The optional
decoder replaces bytes through the first01h marker with spaces, then rotates
each remaining byte's nibbles. Shader reader supplies false, so this decoding
is not part of the installed plain-text debug path.

Load status is not tested between00a6a160 and the unprotected call at
00b66dc4. Missing/unreadable/empty files exit this helper without execution.
A host error-result adapter can be useful, but must be labelled as such;
this trace does not establish recoverable native syntax/runtime error results
or permit claiming panic/exception parity.

## Concrete installed debug route

Paths below are relative to the installed game root. Sizes and SHA-256 were
read directly from disk; no binary content was added to the repository.

| File | Bytes | SHA-256 |
|---|---:|---|
| shaderfx/common/debugshader.shfx | 806 | d45e91ef1a3f11c654b97ac3a9d293b9a24b597b68fb315d611638d3b6933476 |
| shaderfx/dx9_lua.inc | 3117 | 7766adc9c116a9e396b3dd77e9d97b3ea6d55c84601cb2a5a49a64362605b11f |
| shaderfx/lights/dummy.shfx | 114 | 3768df78e6a57ee28563ab40cfb9cf223569c12eb62e8576458365deb0866c5b |
| scripts/fundamentals.lua | 657 | 20e128ebe9a1cb7678e3324b0baae93adab4f34f4d96bd081fb7b86505f63526 |

The debug shader calls DoFile("shaderfx/dx9_lua.inc"), then assigns Shader.
The include supplies constants: RPID_POST_0=23, FLOAT=0, POSITION=0, COLOR=1,
RM_NORMAL=0, RM_UNDERWATER=3, RM_REFLECTION=1, RM_REFRACTION=4.
The shader specifies Priority23, OutputAlpha=false, four combiners using
`dummy.shfx`, and ZENABLE/ZTEST/ZWRITE all zero. VertexInput has Position and
Color, each FLOAT4; the Color interpolator is FLOAT4. Its VS uses cViewProjMat
and passes color; its PS assigns SYS.DiffuseColor. Neither VSVersion nor
PSVersion is present, so generation3 selects vs_3_0/ps_3_0.

`dummy.shfx` is located under **shaderfx/lights**, not shaderfx/common. It
also includes dx9_lua.inc and supplies PS text assigning SYS.DiffuseColor to
FinalColor[0]. A host must resolve this actual file; appending a combiner
basename to the main descriptor's directory fails on the installed asset.
Native basename registry/file search is not replaced by that observation.

## Smallest useful next implementation

Use a real Lua 5.1-compatible runtime to execute the installed descriptor in
an explicit host state, with a DoFile callback using a supplied asset resolver.
For a bounded debug fixture the resolver can explicitly map the verified
files above and report unsupported content overlays. Such a fixture tests
actual Lua evaluation rather than extracting assignments with regex. Pinning
Lua 5.1.1 provides the closest labelled version; runtime source/build selection
and any patch differences still need review before claiming compatibility.

Then reconstruct typed extraction of the resulting Shader table. Existing
profile selection is only one fragment. Immediate native dependencies are:

- 00b439c0 ->00b437f0: Combiners table entries into 14 path slots at+48h.
- 00b419b0 ->00b573f0: VertexInput and Interpolators table elements into
  descriptor arrays+ D0h/+DCh; table iteration order and element conversion
  need assembly checks before a faithful port.
- 00b579b0: RenderStates conversion into descriptor+B8h.
- String/default scalar extraction, including Constants+E8h, VS+F0h, PS+F8h.

Typed getter behavior matters:00b66380 accepts Lua number type3, otherwise
returns its supplied default;00b662f0 accepts Boolean type1, otherwise returns
default;00b660a0 tests string type4 exactly. Lua's general truthiness and
number-to-string conversion are not interchangeable with those native checks.
The integer helper uses00bf7420 after numeric conversion; exceptional numeric
conversion behavior is not established here.

This route can produce the actual debug descriptor and dummy combiner inputs
for the existing source generator. Full native material loading additionally
requires registry/path resolution, content overlays, state/element conversion,
combiner creation and ownership. No arbitrary Lua interpreter, unresolved
native globals, or fabricated fallback stubs should be introduced to link it.

Follow-up: the parent integrated stock Lua5.1.1 execution and the recovered field
conversion rules for the installed debug/dummy fixture. Real evaluated strings
and fields now feed source generation and pixel readback. See
`SHADER_LUA_ADAPTER.md` for implemented scope, adapter differences and remaining
full-loader work.
