# Actual Shader descriptor reader

`read_native_shader_descriptor_00b43b00` executes the normal parser body over
the existing actual 110h `NativeShaderDescriptorStorage`. It opens a real 4C8h
Lua owner with native library mask1, runs the actual B69D40 file/override path
with flag0, obtains `Shader` through tracked native Lua objects, and writes
descriptor fields before invoking the existing actual table readers. It never
constructs a ShaderLuaCode/CompiledMaterialEffect overlay or synthesizes a
successful descriptor. The separately created callable descriptor binding and
the same raw descriptor allocation remain unchanged.

The original ABI is ECX descriptor, stack actual8h filename and unsigned
generation, RET8 at B44682. No useful return value is established. B43700 must
already have constructed the string/array headers. This reader does not zero
unproduced bytes or clear existing arrays. The four callers are B45F5E,
B46104, B46528 and B465C3 inside B45EE0; all pass generation3. Generations below
3 are supported because the actual assembly explicitly selects those defaults.

## Producer order and preserved storage

| Order | Native field | Lookup/default |
| --- | --- | --- |
| 1 | DWORD04, DWORD08 | PipeID=0, Priority=0 |
| 2 | string0C, byte15 | VertexFormat=`simple.mvfm`, ReceiveShadows=false |
| 3 | string100, byte14 | ShadowShader=empty; byte14 from current string length |
| 4 | byte16, float18 | FinalLODFadeOut=false, FinalLODFadeOutRange=0.01f |
| 5 | DWORD108, byte44 | RTCount=1, literal misspelling VisilityFade=true |
| 6 | bytes1C/1D/1E/1F | AlphaToCoverage=false, NoBandingFix=false, DisableAlphaToCoverage=false, CompressedVertices=true |
| 7 | DWORD20, string28 | CompressedElemCount=999, InstanceGenerator=empty |
| 8 | bytes30/31/32/45 | PixelPositionRegister=false, OutputAlpha=true, LoResBlend=false, WriteDepth=false |
| 9 | strings34/3C | VSVersion/PSVersion; generation<3 selects vs_2_a/ps_2_b, otherwise vs_3_0/ps_3_0 |
| 10 | pairsB8 | RenderStates through B579B0, preserving existing unique states |
| 11 | strings48[14] | Combiners through B439C0; selected modes only |
| 12 | actual ownersC4 | Samplers through B41830 and its real sampler/list pool |
| 13 | actual string ownersD0/DC | VertexInput then Interpolators through B419B0 |
| 14 | stringsE8/F0/F8 | Constants, VS, PS; empty defaults |

Exact native NUMBER/BOOLEAN/STRING gates come from the existing native Lua
getters. Version selection first gates and releases one lookup, then performs
a fresh lookup before the successful string read; it does not reuse the gate
value. The render table also has separate gate and value lookups. Publication
of descriptor fields precedes releasing the corresponding tracked Lua value.
String/default results are actual pooled8h headers and are released before
the Lua value. Native fixed names are constructed and released around each
combiner/field reader. At successful exit the Shader object is destroyed before
the actual Lua owner is closed.

The parser leaves vtable00, byte17, DWORD24, byte33, bytes46/47, DWORD10C,
unselected mode strings and array preimages intact. It writes only three bytes
of word30. Existing arrays append under their established child contracts;
re-reading another file is not replacement of all descriptor-owned resources.
Neither PipeID04 nor any descriptor member becomes an intrusive count.

Assembly was required here: Ghidra reports nine unreachable blocks in string
construction/copy/release, aliases local headers, and shows the generation
comparison as a return-address value. The saved instruction listing contains
the real blocks and unsigned stack-generation comparison. All 805 instructions
were inspected in bounded pages. The only x87 sequence uses original D7A238
bits3C23D70A, passes float32 to B66330, and stores the returned float32 at18.
The source retains those bits without adding extended arithmetic.

## Real dependencies and retained failure

Existing implementations reused directly are B66BD0/B6A020 bootstrap, B69D40
file execution, B67980/B67800 native stack objects, scalar/string predicates
and getters, B67700/B669A0 cleanup, B579B0 render states, B439C0 combiners,
B41830 samplers and B419B0 fields. Sampler parsing uses the existing actual
state-definition publication0108FE90 and SAME sampler class binding, state-list
pool and string lifetime. The existing combiner/field readers still require
explicit original stack preimages when required ordinals are absent. No zero
defaults or heuristic field inference were added for those cases.

`NativeShaderDescriptorReadOperation` derives from the program loader's
`NativeMaterialProgramChildFrame`. Allocate and retain it before calling the
reader. It contains the stable actual Lua owner, globals/Shader/current-value
objects and temporary string. It records phase and active native call site.
The program adapter passes the same descriptor and stable original name header;
the reader stores those borrowed identities without moving their storage.
The shared bootstrap/file services must install the same DoFile callback.

On normal return all parser locals/Lua state have been released, and deleting
the child frame is metadata-only. On a C++ error, the frame records failure and
keeps its own live Lua/string state and the descriptor's already-published
effects; it neither retries nor destroys the descriptor. A live failed frame
cannot be discarded. This retained host failure model is not native FH3 unwind
equivalence. Existing callees retain their documented behavior, including their
own local cleanup and unowned allocation/stream behavior on errors. In
particular, B66CA0's unprotected Lua call, ignored load result and early stream
retention are reused rather than repaired. Process-fatal Lua panic, arbitrary
allocator/VM reentry, and complete exception-path acquisition recovery are not
claimed by this packet.

B69D40 needed no duplicate reconstruction. Its saved Ghidra body ends at
B69DCC, but the existing source and inspected24-byte tail continue through
B69DE4, including RET8 at B69DE2. The full execution order is first B66CA0,
then current VFS BDEF90, captured begin/end override iteration, and complete
native string-vector cleanup. The worker does not mutate that saved body.

## Validation scope

The focused ignored fixture is adapted from the previously reported actual VFS
composition harness, after checking its source SHA256 against that report. It
uses original read-only native profile data, explicitly seeded actual mount
records, real physical HANDLE streams, native pooled strings, the actual
fundamentals singleton, and the existing Lua5.1.1 implementation. It reads the
installed `common/debugshader.shfx` and `common/alphablend.shfx`, including their
real nested `dx9_lua.inc`, into the SAME descriptor. It checks both generation
defaults, retained mode entries, appended field/state/sampler owners, source
strings, untouched raw bytes, stable callable identity, direct descriptor
deletion, actual state-list pool return and canonical domain shutdown.
The fixture passes. Debug produces two recognized render-state pairs
(`ZWRITE` and `ZTEST`; its `ZENABLE` key is absent from the native registry),
zero samplers, two vertex fields and one interpolator. Alpha then leaves five
state pairs, one sampler, five vertex fields and two interpolators in that
same descriptor. Both generations' default profiles and the unselected mode
strings survive as established by their producer paths.

Exact results, source/asset hashes and call rows are recorded in
reports/native_shader_descriptor_reader.json. The new production source is
compiled directly alongside the fixture with MSVC Win32 W4/WX. The separate
scripts/build.ps1 baseline build passed, including its existing one CTest.
That build does not establish CMake registration of this
new source: cmake/startup.cmake is leased to another harness, so the integrator
must register it and run the combined build. No permanent tests, native ABI
replacement, compiled material program, shader drawing or game validation is
claimed.
