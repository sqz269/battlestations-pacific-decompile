# Native model numbering

Addresses: `00711510`, `00711A20`, `00711BE0`; supporting release `00710630` is
analyzed and composed through the existing actual-owner release helper.
Descriptive names are hypotheses, not recovered symbols.

| Body | Source operation | Evidence boundary |
| --- | --- | --- |
| `00711510..00711A1F`, 1296 bytes | `set_native_section_numbering_00711510` | Complete supported normal caller; initialized vertex formats and current concrete renderer/stream services required |
| `00711A20..00711BD7`, 440 bytes | `set_native_node_numbering_00711a20` | Complete recursive normal caller; actual node type dispatch remains required |
| `00711BE0..00711BF4`, 21 bytes | `set_native_model_numbering_00711be0` | Complete wrapper over model `+160` and holder `+0C` |

The section caller receives mesh in ECX, section in EDX and an unsigned number
on the stack, returning with RET4. The recursive caller receives node in ECX
and number in EDX, with plain RET. The model wrapper receives model in ECX and
number on the stack, with RET4. These source interfaces add explicit services
and diagnostic acquisition state; they are not binary replacements.

The section must have a positive signed texture count at material `+34`, a
nonnull slot-zero texture, and a texture name containing `num_0` or `shipnumber`.
Its captured logical stream at section `+3C` must have normal, UV and packed
colour offsets different from FFFFFFFF. Numbers above 999 are clamped unsigned:
a negative signed Numbering value passed as a DWORD therefore becomes 999.

The caller requests encoded declaration `pf43nf43uf44cc.mvfm` through current
renderer virtual `+38`, then requests a vertex stream through current `+5C`
with the live section `+10` count, flags 1 and that declaration. The format is
parsed by the existing encoded-format resolver; this is not a file load.
The tokens establish position float3, normal float3, UV float4 and packed
colour: offsets 0,12,24,40 and stride 44. See
`docs/VERTEX_FORMAT_RESOLUTION.md` and `docs/NATIVE_VERTEX_DECLARATION_CACHE.md`.

Declaration release occurs after stream creation and before mapping. The new
stream maps first with `(0,0,0)`, then the captured old stream with `(0,0,1)`.
The initial `num_0.tga` atlas lookup publishes its actual texture into the
current section material before testing the live vertex count. No stand-in
texture, buffer, owner, declaration cache or reference count is introduced.

Each vertex passes position and normal through the existing readers and the
caller's additional x87 float32 stores. Normally the UV reader supplies two
components. When the source UV declaration type is 3, the caller instead
reads the preserved pair at source-map `+20/+24`, advancing by 44 bytes.
This is the repeated-numbering path for the already converted float4 UV field.

Colour is copied to destination `+28` before selecting a digit. Byte 2 at least
250 selects hundreds, otherwise byte 1 at least 250 selects tens, otherwise
byte 0 at least 250 selects units. Red takes priority over green and blue.
Hundreds are hidden when zero; tens are hidden only when both hundreds and
tens are zero. Units always select a digit, including zero. With none of these
markers, nonzero hundreds select `num_0.tga`; otherwise atlas UV is zero.

Visible digits construct `num_<digit>.tga` through the existing native string
constructor, unsigned conversion and two concatenations. The four intermediate
strings are released before the atlas lookup; the final name is released after
both transformed UV stores. The atlas owner is reloaded for every lookup.
Original UV values are preserved at destination `+20/+24` after name release.
The loop reloads section `+10` after each iteration.

Atlas arithmetic preserves the observed x87 stack sequence, including the
float32 minimum spill before `minimum + uv*(maximum-minimum)`. No FMA, clamp,
finite-value filtering or rounding-mode reset is added. Hidden atlas UV uses
positive-zero bit patterns. Unsupported reader formats throw an explicit
source-boundary error after prior effects; native uninitialized scratch is not
fabricated. Fault delivery and unmasked floating exceptions remain unproved.

After the loop, the old stream unlocks before the new one. The caller publishes
the new stream into mesh slot zero, releases its creator reference, and rebuilds
the section layout using the existing actual owner and layout services. Failed
source operations retain acquisition state and cannot be replayed. This does
not reproduce native FH3/SEH cleanup or authorize rollback of partial effects.

The recursive caller captures the node's current profile before reading the
live model type ID at `01090034`, then dispatches actual virtual `+0C`. A model
has at most one geometry: `00B74650` is the boolean test of raw `+180`, and
`00B74640` returns that raw geometry. The section count, selected section and
material are reloaded at the native call sites. An effect name must contain
`shipnumber`; its first texture name must contain `shipnumber` or `num_0`.
Matching sections are fetched again before the section caller. Visibility is
set to zero for number zero or one otherwise, without recursive visibility,
only if a section matched. Child `+34` and subsequent sibling `+3C` links are
read after the relevant calls. No separate graph or geometry list is created.

The ignored oracle executes the original vertex loop and digit/atlas branches
from `007116E6`, with one exit transfer at `0071180E`. It executes the original
AD reader bodies. String and atlas call sites use recorded native-ABI adapters
to the same existing source callees, with CRT-backed string storage explicitly
supplied for this fixture. Their implementation is not independently proved by
this comparison. The actual D3DX import is bound, but the selected loop cases
use raw source formats and do not execute half conversion.

All 128 loop comparisons pass: eight number values, ten marker patterns, both
UV paths, and 24/53-bit x87 precision under four rounding modes. They compare
1280 vertices, 256 complete guarded destination images, unchanged source bytes,
control word/MXCSR and status masked by `3A7F`. One additional source check
covers zero count with null mappings. An independent audit verifies every
declared operand edit and the sole exit transfer; all other retained bytes
match the original. BSS fallback/publication cells are verified against PE
virtual zero fill rather than described as file-backed bytes.

Win32 compilation and both existing CTests pass. Full renderer creation,
mapping, ownership cleanup and recursive traversal are not executed by this
oracle. Unit health's Numbering provider must bind these same live services;
runtime admission, gameplay and rendering parity remain unproved. Exact call
rows, artifacts and integration results are in
`reports/native_model_numbering_ae.json`.

## Integrated compatibility validation

Combined commit `755470a9b91d3a573067f04086af84eae09b4336` passes the Win32 build and both existing CTests. Its preserved executable has SHA-256 `0223562b72049b99c307d722eb941948617eda10e7d4363266c7e3f0196fdfd9`. The existing 120-frame USN01 check exits successfully in 10.451 seconds, with 18,557 finite trajectory rows, 241 unchanged Airfield2 samples, 2,400 avoidance queries, 10,080 generic ticks, 420 world-list nodes and the existing observer/pending-owner teardown checks. The production numbering object remains byte-identical to the native fixture object. This mission check establishes compatibility of the combined build; execution of these numbering callers through a complete model-numbering owner is not established. The report retains separate immutable native-fixture and integration manifests. No workers were dispatched for this closeout.
