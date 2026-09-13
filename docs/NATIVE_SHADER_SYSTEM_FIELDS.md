# Actual shader system-field appenders

Addresses: `00B35BE0`, `00B372D0`, `00B34E20`.

The new source appends actual heap-allocated 1Ch field records to the existing
B0h material-program builder's owning pointer arrays. It uses the same
`NativeShaderFieldStorage`, `NativeShaderDescriptorArray` and native string
storage as the existing descriptor reader and compiler. The older
`shader_system_fields` value vectors remain a semantic projection, not native
builder storage or an ownership provider. Names are descriptive hypotheses.

| Entry | Inclusive range / bytes | Original ABI | Coverage |
| --- | --- | --- | --- |
| B35BE0 | B35BE0..B367FC / 3101 | ECX builder, RET | Complete normal readable-domain body, 985 instructions |
| B372D0 | B372D0..B37EEA / 3099 | ECX builder, RET | Complete normal readable-domain body, 985 instructions |
| B34E20 | B34E20..B34E84 / 101 | ECX field; name/scalar/width/semantic/index/mask on stack; EAX field; RET18h | Complete normal constructor, 36 instructions |

All three saved Ghidra bodies include their complete return instructions.
No new binary ABI entry is provided. Unreadable pointers/extents and signed
overflow outside allocated native storage are outside the source domain.

B3B3C0 is the only caller of each appender: B3B515 invokes vertex preparation
after B35930; B3B531 invokes pixel preparation after B36800 and B34AA0. The
caller reads globals108D6F0/F1 to select this path. Neither appender reads a
descriptor, builder mode flag, or those globals. Calling the two new functions
adjacently does not reconstruct the intervening compiler work or a pass.
B34E20 has 33 native call sites: the 32 fixed fields and B3686B, whose arguments
are the same ScreenSpacePos tuple. No call-site operand is inferred from a
semantic field name.

| Vertex order, builder+10 | Width | Semantic |
| --- | --- | --- |
| ScreenSpacePos, ObjectSpacePos, WorldSpacePos | 4 | 0 |
| Normal | 3 | 3 |
| Tangent | 3 | 5 |
| BiNormal | 3 | 4 |
| CustomValue0..3 | 1 | 2 |
| CustomVec2_0..3 | 2 | 2 |
| CustomVec4_0..1 | 4 | 2 |

Pixel order at builder+34 is DiffuseColor, SpecularColor, EmissiveColor
(width4), SpecularPower (1), Normal (3), Depth (1), then the same ten custom
fields. Every pixel semantic is2. Both lists use scalar0, index0 and mask0.
All 192 CALL instructions and 32 constructor push sets were checked from the
complete listings. EBP becomes1 at B35C20/B37310 and remains1 until epilogue;
it supplies scalar-width1, count increments and string-release arguments.
Each BF681B call cleans4 bytes; B34E20 returns18h, B34680 returns4. Literal
contents, row order, call sites, publication sites and native hashes are pinned
in the report and ignored extraction artifacts.

For every row native order is: allocate1Ch, initialize the temporary8h name
from a literal, copy that name into the independent field, append its pointer,
increment the current array count, then release the temporary name. B34E20
clears only name00/04 before copying; its successful scalar stores are
08/0C/10/14/18. Failure before those stores leaves their preimages intact.
There is no field vtable or +04 reference counter: +04 is the name buffer.
The same builder destructor B3A7E0 releases field names and raw records.

Reserve uses the existing actual B34680 helper only when count equals
capacity, requesting signed max(capacity+5,10). That helper allocates/copies
pointer slots, frees the old buffer before publishing the replacement and
capacity, and has no throwing host call after its successful allocation on
valid accessible storage. Slot publication reads count, then data; count
increment reloads the actual header. Repeated calls append another16 fields;
they neither clear nor deduplicate. Existing BF681B allocation throws on
failure, so its native null branch is not replaced by a successful fake field.

`NativeShaderSystemFieldsOperation` is one persistent caller frame. It records
the same builder/string provider, immutable source-name identity, row/site,
current actual field, temporary-name header, publication and cleanup states.
The enclosing compiler must publish it before calling the appender. On a
host exception, the current raw field and temporary allocations remain tracked,
including a name-copy failure before the field is appended. Destruction of a
failed/running frame terminates. The enclosing caller must retain the frame
and exclude builder retirement until the failure is resolved. Diagnostic-only
acknowledgement after explicit cleanup is distinct from native completion and
does not permit replay. No second builder or owner registry is installed.

This retention intentionally differs from native FH3 unwind. FuncInfos DF6AF4
and DF6D4C each have48 states and no catches. Each row has three states:
raw allocation cleanup to -1; temporary-name cleanup followed by raw cleanup;
and temporary-name cleanup only after the field constructor has returned.
The raw cleanup frees the captured slot at local-1C; the name action tests its
row bit then destroys the one local8h header at local-18. Thus native failure
during reserve does not delete the completed unappended field. Source does
not invent that deletion, but also does not claim native FH3 equivalence.
Supporting handler stubs CBE45D..CBE466 and CBE77D..CBE786 are ten-byte
non-function gaps; each loads its FuncInfo then jumps BF6B43. They are evidence
only, unmodified and outside the reconstructed entry points.

The focused ignored fixture compares all6301 original bytes (2006 instructions)
with source using actual installed shader descriptors and the existing actual
builder/field/string services. Its original calls use concrete CRT, string and
pointer-reserve bridges, not numeric native addresses as host callbacks.
It checks field names/scalars, growth, append retention, untouched builder
bytes, actual builder destruction and one retained second-row name-copy
failure. Original exception handlers, interpolator selection, code generation,
full compiler success and game rendering are not exercised. Build/fixture
results and exact source/library dependencies are recorded in the report.
