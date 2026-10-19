# oct18b-nC high-band rows

The 36 assigned rows from `00AFD860` through `00C34CF0` contain 35 game
implementations and one MSVC native-string heap wrapper classified
`library_instantiation_out_of_scope`. Exact row identities, source paths,
original ABIs, uncertainty and 25 direct call sites are recorded in
`reports/oct18b_nc_high.json`. This is build-only reconstruction; no host was
wired and no game was started.

Existing source families were extended for particles/named lists, scene nodes,
matrix arithmetic, camera direction, mesh resources, ocean virtual defaults,
and the in-house Dyn geometry block. The new `oct18b_nc_high` source holds the
unordered pointer-array removal and three default virtuals whose owning class
remains unresolved. Descriptor getters return original image identities as
32-bit address values; they do not fabricate live pointers.

Assembly resolved the decompiler's missing ESI output buffer in the four
polygon clippers, EBX count in matrix expansion, fourth stack argument in the
node animation walker, and second stack argument in the camera direction
setter. The clippers retain the unordered x87 comparison branch and the float
spill of the intersection fraction before multiply/add. Matrix blend retains
three rounded intermediate matrices. Texture forwarding leaves are naked tail
jumps, since the destination owns the unresolved stack and result ABI.

The animation walker copies identity over its sampled matrix when the sampler
returns a nonzero AL. With a zero result it retains sampler writes; without an
animator it leaves the native stack region untouched. The meaning of the result
byte is unproven. The new interface requires a callback providing its entry scratch
contents, including distinct storage for recursion; it introduces no fallback
for that indeterminate path. Native failure-path behavior and unmasked FPU
exception ordering have not been tested. The C++ interfaces are not drop-in
binary replacements.

Validation receipts are recorded in the JSON report after the combined Win32
build. The report's 25 direct call-site claims passed live Ghidra listing checks.
Annotation tooling verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`, preserves prior names/comments in the local
annotation log, saves analysis, and refreshes the selected exports.
