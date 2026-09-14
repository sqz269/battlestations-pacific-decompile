# Native normal, UV and colour readers

These are dependencies of the model-numbering setter `00711510`, reached from
unit initialization through `00711BE0` and `00711A20`. They also serve existing
geometry consumers. Descriptive names are hypotheses, not recovered symbols.

| Address and body | New source operation | Coverage |
| --- | --- | --- |
| `0070FDB0..00710056`, 679 bytes | `read_native_vertex_normal_0070fdb0` | Raw float3 and all initialized decoded types 2,4,5,7,8,10,12,13,16; other decoded types explicitly unsupported |
| `007100A0..00710266`, 455 bytes | `read_native_vertex_uv_007100a0` | Raw float2 and all initialized decoded types 1,6,9,11,15; other decoded types explicitly unsupported |
| `00476180..0047626E`, 239 bytes | `read_native_vertex_colour_00476180` | Complete packed and float-component branches for valid backing with masked floating exceptions |

All three native interfaces take the stream in ECX, output then index on the
stack, return the output pointer in EAX and end with `RET8`. The source APIs use
explicit output references and have a new C++ ABI. Normal and UV return false
without writes for types whose native branches consume uninitialized local
words. They remain partial projections outside the initialized-format domain;
no fallback format or fabricated scratch values are supplied.

The implementations extend the existing `native_vertex_position_read` module.
They borrow the same raw logical stream, its mapped pointer at `+08`, stride at
`+0C`, and current decode-record allocation at `+50`. There is no new stream,
vertex copy, mapping, material owner, lock or reference-count operation. The
existing byte, short and normalized-short helpers, nine-bit unpacker and exact
`d3dx9_40.dll!D3DXFloat16To32Array` binding are reused.

Producer `00B61E20` establishes the normal offset/type/record-index fields
`+1C/+20/+24` from declaration semantic 3, occurrence 0. Semantic 5 supplies UV
`+28/+2C/+30`. The constructor initializes `+50` to null; `00B61D90` later adopts
the metadata described in `docs/NATIVE_VERTEX_POSITION_READ.md`. A record is
20h bytes: four scales followed by four biases. Readers reload the current
record index and allocation after decoding, including the actual half import.

With null metadata, each float uses an x87 load/store before the next source
lane is read. Overlapping output therefore retains sequential-copy behavior.
With metadata, decoded lanes and transformed lanes are staged before publishing
output. No clamp, finite-value guard, FMA or rounding-mode reset is added.
Normal type 4 and type 8 both normalize the first three bytes by double 255.
Normal type 3 is uninitialized, unlike the existing position reader's type 3.
Type 13 retains the observed `base + 4*(stride*index + attribute_offset)` address
and nine-bit masks at shifts 0/10/20. Normal's first scale multiplication loads
the decoded component before the scale; subsequent components and the position
reader load the scale first. The x87 operand order is preserved.

UV types 6,9,11 read two DWORDs and convert three short components, although only
two output components are returned. The unused third normalized component can
still set the x87 precision flag. The fixture isolates that effect with exact
first and second components and an inexact third component. UV half conversion
requests two elements from the actual named import.

For colour, nonnegative signed `+34` selects one packed DWORD. Otherwise offsets
`+38/+3C/+40/+44` supply float values for output bytes 2/1/0/3. Multiplication uses
double 255 under the incoming precision/rounding mode. Each FISTP temporarily
sets x87 rounding to truncation, keeps only the low byte, and restores the saved
control word. Values are not clamped. The native base constructor initializes
these four float offsets to FFFFFFFF and only establishes packed `+34` when
semantic 10 has size 4. A producer of valid separate float offsets is still
unresolved; the conversion branch is fixture-tested with explicitly supplied
valid offsets, not claimed admitted by the constructor or game runtime.

The fourth callee initially investigated, `00710630`, releases an intrusive
reference through a slot. It is not a format-copy routine and is not ported or
renamed by this packet. The remaining material-numbering and runtime ownership
providers are still required; these readers do not substitute for the complete
numbering setter or unit initialization.

Validation uses six original-byte bodies (the three readers, the existing
position reader, the nine-bit helper and the original half-import thunk), plus
their verified tables and constants. Sixteen spans match live Ghidra and the
pinned original PE. The clone changes only 49 absolute operands/table entries
and the loader-bound half-import slot; relative calls and all other bytes are
independently checked unchanged. No handwritten half decoder or native-call
stand-in is used. The actual installed x86 DLL and named export are recorded.

The single ignored fixture passes 200 new-reader and 72 position-regression
comparisons. It checks complete storage images, output-pointer returns, native
stack balance, exact control word/MXCSR and x87 status masked by `3A7F` (exception
flags, C1 and TOP). It covers 24- and 53-bit x87 precision under all four rounding
modes, raw/metadata overlap, packed formats, NaNs and colour truncation/wrapping.
All 544 retained native/source storage images match. Two source-only cases check
explicit unsupported-format rejection; native uninitialized paths are not run.
Seventeen direct call rows and every incoming argument setup are recorded.

Win32 compilation and both existing CTests pass. This is reader reconstruction
and fixture evidence, not native ABI replacement, unmasked exception-delivery
equivalence, model-numbering runtime admission or game/render parity. Exact
artifacts and final integration evidence are in
`reports/native_vertex_attribute_read_ad.json`.

## Integrated compatibility validation

Combined commit `687ffdb238e51675e3533dc6e99acaab93e10418` passes the Win32 build and both existing CTests. Its preserved executable has SHA-256 `5c8ef5f6c3c5ccbfae2127cf25201971094ab84e462df44dc2b2da5ec3e98306`. The existing 120-frame USN01 check exits successfully in 9.872 seconds, with 18,557 finite trajectory rows, 241 unchanged Airfield2 samples, 2,400 avoidance queries, 10,080 generic ticks, 420 world-list nodes and the existing observer/pending-owner teardown checks. The production reader object remains byte-identical to the native fixture object. This mission check establishes compatibility of the combined build; execution of these new readers through a complete model-numbering owner is not established. The report retains separate immutable native-fixture and integration manifests. No workers were dispatched for this closeout.
