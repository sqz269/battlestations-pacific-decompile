# Native shader sampler declarations

This packet appends sampler declarations to the existing actual B0h builder's
string at `+4C`. It consumes the same actual 110h descriptors at builder `+70`
and `+74`, their `+C4` pointer arrays and `+C8` counts, and actual 2Ch sampler
records. It reuses `append_native_shader_line_00b35110` unchanged. No semantic
descriptor projection, private output string or replacement formatter is added.
Descriptive names are hypotheses, not recovered symbols.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| `B37EF0` pixel declarations | ECX actual builder; no stack arguments; plain RET at `B38078` | complete normal body, 393 bytes |
| `B38080` vertex declarations | ECX actual builder; no stack arguments; plain RET at `B38208` | complete normal body, 393 bytes |
| `B35110` dependency | cdecl destination, format, variadic arguments; RET | existing native formatter reused unchanged |

`B39110` calls the vertex helper at `B39355`; `B39880` calls the pixel helper at
`B39A67`. Both set ECX to the builder and supply no stack argument. These are the
known direct callers. Each helper calls `B35110` twice in its body: pixel at
`B37F98/B38058`, vertex at `B38128/B381E8`. Every call is followed by
`ADD ESP,10h`: destination, format, name pointer, signed register slot.

The record layout is established by `B57B50`: `B57B80..B57BA0` initializes the
2Ch owner, its name header at `+04/+08`, and the stage byte at `+0C` while
preserving padding `+0D..0F`. The Name copy targets the `+04` header at `B57BD1`;
the Type integer is stored at `+10` by `B57C62`; the VertexSampler Boolean is
stored as AL at `+0C` by `B57E46`. `B41830` appends those actual sampler pointers
to descriptor `+C4`, increments count `+C8`, and uses capacity `+CC`. Existing
`NativeShaderSamplerStorage`, `NativeShaderDescriptorStorage`, and builder
storage declarations are used directly; no second record schema is introduced.

Both helpers visit descriptor70 then mode_descriptor74; `+74` is not the
material effect pointer at `+78`. Pixel selects a ZERO byte at sampler `+0C`;
vertex selects a NONZERO byte. This is stage selection, not generic resource
enablement. Unknown selected types still consume a register slot without text.
Unselected records consume no slot. The DWORD slot starts at zero and continues
across both lists. Types 1, 2, 3 and 4 emit `sampler1D`, `sampler2D`,
`samplerCUBE` and `sampler3D`. Names use the actual name pointer at `+08`; a null
pointer selects the borrowed empty literal at `0108D6F2`. Exact tabs and signed
`%i` formatting are preserved, and `B35110` appends the separate newline.

The current descriptor pointer is reloaded from the builder after EVERY row,
including skipped/unknown rows. The unsigned index then increments and compares
against the CURRENT count. The next iteration reads the CURRENT array data and
pointer at that index. Replacing a descriptor, its array base or count during a
formatter callback is therefore observable. The second descriptor is first read
only after traversal of the first finishes. No count snapshot, deduplication,
sampler limit, sorting, validation or clear operation is added. Actual accessible
descriptors, records, names and array extents remain caller preconditions; null
descriptors/records are not interpreted as empty lists.

Each call uses a persistent `NativeShaderSamplerDeclarationsOperation`. The
existing native formatter's operation is an embedded optional child. A completed
child can be replaced for the next emitted line; a failed child is retained with
its actual pooled temporary and output preimage. The parent retains descriptor,
array/record/name pointers, sampled fields, index, slot and original call site.
Slot/index/row completion updates happen only after formatting returns. Replay is
rejected before further native effects. Running or failed frames terminate on
destruction. Diagnostic acknowledgement frees nothing and is allowed only after
the caller resolves all child acquisitions. It also acknowledges the failed child.
The caller must keep all borrowed owners/domains alive and exclude their retirement;
this is not an installed native owner-admission guard or private FH3 unwinder.

The original helpers contain only four jumped-over alignment gaps:
`B37F0A..B37F0F`, `B37FC8..B37FCF`, `B3809A..B3809F`, and `B38158..B3815F`.
There is no missing post-CALL continuation. Ghidra was read only in this worker;
the existing project/program were verified by the BSP wrappers. The two helper
bodies plus the existing formatter are pinned to live Ghidra and installed PE
bytes. Normal original instructions are relocated with unchanged internal branch
offsets and audited direct CALL targets rebound to the copied formatter or the
established actual string resize/pool and host CRT boundaries.

One focused fixture compares original/source exact text and pooled allocation/
release traces using actual initialized builder, descriptor and sampler storage
and the canonical actual pooled-string owner in one lifetime domain. It covers
both stages, all four known types, selected unknown types, byte-sized stage reads
with nonzero padding, null name data, empty lists, existing output text and untouched
builder fields. A release callback replaces both descriptors, then replaces live
array data/count. A source-only allocation failure retains the existing formatter
child and parent traversal state; replay and failed retirement are checked. Fixture
arrays are borrowed native pointer-array storage, populated from the verified
schema; the Lua sampler reader is not executed by this fixture. The fixture does
not execute enough rows to observe slot wrap; the DWORD arithmetic is assembly-
reviewed. Full source generation, shader compilation, texture sampling, rendering,
native binary ABI replacement, original FH3 and gameplay remain unproven.

Default MSVC Win32 build, existing CTest, numeric CALL verification, and exact
fixture artifact hashes are recorded in `reports/native_shader_sampler_declarations.json`.
