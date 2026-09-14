# Complete numbering caller execution

Addresses: 00711510, 00711A20, 00711BE0.

The reconstructed section, recursive node and model-wrapper numbering callers
now run through the existing real D3D9 renderer, declaration, mapping, hardware
layout and canonical owner services in a retained fixture. This closes the
runtime evidence gap left by the vertex-loop-only AE check. No production
algorithm change was needed.

The fixture uses the AF pool constructors over the actual declaration, logical
vertex/index and physical-buffer storage. It also uses the existing hardware
layout, mesh, section, material and parameter pool lifecycles in one shared
allocator list. The declaration registry is initialized and destroyed through
its reconstructed routines. The hidden 64x64 D3D9 HAL device uses software vertex
processing; buffer maps and hardware declaration creation use real COM calls.

Twenty-five comparisons pass: eight numbers through each of the three APIs,
plus FFFFFFFF through the model wrapper to check unsigned clamping to 999.
The recursive topology has a model root with geometry, an empty child and a
sibling sharing the root's mesh. It executes 42 section updates and 51 actual
model-type leaf calls. Model descriptor initialization uses the reconstructed
bootstrap and current0C dispatch calls its existing 006EF860 implementation.
Number zero hides the matching models; the empty child's visibility is retained.

Each case starts with the ten retained AE vertices, initialized UVtype3 and
control word 027F. All 11,000 returned buffer bytes match the previously sealed
original-machine-code vertex-loop outputs, including the unsigned-clamp case.
The full source calls also verify mapping release, mesh/section publication,
44-byte stride, real hardware declaration offsets, texture reference transfer
and canonical stream retirement. The material terminal returns its real pool
slot; the declaration registry destructor and final releases leave no canonical
owners or renderer stream entries. Actual hardware-tree CRT shutdown is observed
after main returns.

An initial fixture assertion incorrectly treated an old raw address as a unique
stream lifetime. A recursive second update reuses the slot retired by the first.
The corrected check records actual canonical vertex-unbind events and verifies
each retired generation, while allowing a new live owner at the same address.
The failure trace is retained alongside the passing run.

These are complete reconstructed source caller executions. The original
machine code supplies the vertex-loop reference outputs; the original complete
caller bodies and FH3 do not execute here. The renderer prefix, empty VFS tree,
model-node topology, effect prefix and atlas are fixture inputs. Texture named
construction and retain/release are real, while external texture/effect creator
references remain alive. Other node profiles, allocator faults, full resource
startup, live model/atlas loading and gameplay/render parity remain unproved.
The mission runtime still requires this service composition to be connected.

`reports/native_model_numbering_runtime_ag.json` records the byte spans, original
call rows, output hashes and exact retained fixture scope. The fixture sources,
raw outputs and copied original references are under
`local/model_numbering_runtime_ag/`. No workers were dispatched.
