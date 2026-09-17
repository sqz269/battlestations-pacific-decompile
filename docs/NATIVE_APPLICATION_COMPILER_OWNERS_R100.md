# Application material-compiler owner domains (R100)

`GameNativeRendererApplication::material_compiler_owners()` now supplies the
actual application's pass construction, shader reflection, shader construction
and companion-registration contexts for `B3B3C0`. This closes a production
binding gap previously covered only by the private R90 compiler fixture.
It does not yet connect the complete material compiler or R99 preload loop to
ordinary startup.

## Shared native storage and lifetime

The new graph borrows the existing application string pool, raw string context,
system-constant publication `0108FE94`, resource-support publication `0108FEDC`,
renderer publication `00F8D394`, owner registry and pass pool `0108FBF8`.
Reflection resolves `D3DXGetShaderConstantTable` from the same `d3dx9_40` module
already loaded by the application's device graph. The module outlives these
contexts and their completed native cleanup.

Registration installs one host companion over each original owner's existing
atomic count at `+04`: the pass, its three state owners, reflection metadata and
the D3D9 shader wrapper. It performs no native initialization or extra retain.
The current original numeric profiles select the established native terminal
bodies. After each terminal finishes, its callback removes that exact binding
and disposes only the host companion. The graph retains a separate host count
to reject destruction with surviving companions; this is not a native count.

The API requires the renderer's ready phase. Callers must keep their operation
frames and borrowed domains alive through failure and exclude owner retirement
until required cleanup is resolved. The registration callbacks are nonthrowing;
a metadata-allocation or binding contract failure terminates instead of
continuing with an unregistered native owner. No exception rollback or native
FH3 behavior is invented.

## Original evidence

| Original routine | Bytes checked against live Ghidra and the PE |
| --- | ---: |
| `B44B10` pass construction | 214 |
| `B5F720` pass-base construction | 622 |
| `B3AEA0` compiled-shader reflection | 667 |
| `B5F9B0` pixel-shader wrapper construction | 320 |
| `B5FAF0` vertex-shader wrapper construction | 320 |

Also checked six original `B3B3C0` call sites and the reached renderer, pass,
state, reflection and shader profile words: 2,349 bytes total. Existing native
names are preserved and comments extended under the Ghidra write lock; prior
values, save/readback and refreshed exports are retained. This packet introduces
application bindings, not additional reconstructed native function bodies.

## Runtime verification

One local diagnostic replaces only the production entrypoint's startup-host
forwarder. It links all 52 current application objects and the three current
libraries, then borrows the application's real device, VFS, cache and contexts.
It adds no fake renderer, private constant registry, preseeded shader cache,
replacement compiler callback or private pass pool.

The actual cache loader read all 1,628 installed `shaderfx/shaders.bin` rows.
For every row, the diagnostic constructed a native pass and loaded `white.tga`
through the actual texture route, constructed/registered reflection storage,
ran the complete `B3AEA0` reflection body, created a D3D9 shader on the actual
device, constructed/registered its wrapper, attached it through the matching
native pass shader-slot setter, and retired the caller and pass references.
Each of the six compiler companion identities was absent after pass deletion.

Results: **814 vertex shaders, 814 pixel shaders, 1,114 material-constant records**.
The aggregate of names and bytecode was `2733cb68f3747826`, independently matched
to the 2,250,700-byte installed file (2,205,720 payload bytes). The cache cursor
stayed at zero because this diagnostic visits rows directly; this is not proof
of the compiler's forward cache lookup or native preload ordering.

The initial diagnostic assumed the registry would lose only the pass and its
three state companions relative to its post-construction count. Actual
`white.tga` had one reference and was also retired: the registry fell from six
to one. That failed diagnostic is archived. The final check verifies the six
specific compiler identities and stable shared-owner count instead. No
production change was made to accommodate that diagnostic error.

The probe and unmodified application each completed two ticks, one Present,
normal singleton drain, worker join and final device/API COM counts of zero.
The strict Win32 build and all three existing CTests pass. No permanent test
case or test target was added.

## Remaining integration and evidence limits

These source interfaces retain their explicit C++ ABI. This run does not prove
original register ABI, native CRT/new-handler behavior, FH3/SEH or exceptional
whole-compiler unwinding. It did not execute the full `B3B3C0` compiler, assemble
complete materials, draw them, or run the `73BF80` preload loop.

Full application compilation still needs shared shader-source/compilation and
procedural sampler services, the material-effect loading graph and the first
sampler stack-value contract. The original compiler tail includes a D3D9 COM
Release before the first sampler; its stack residue is not established by the
game listing alone. R98's unknown-value gate remains in force. No guessed zero
or fixture-only residue has been installed. Startup preload/vector lifetime,
rendered-scene behavior and gameplay remain unvalidated.
