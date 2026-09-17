# Application shader source and compilation domains (R101)

The application renderer now supplies `material_compiler_sources()`: the
existing native shader generators and vertex/pixel compilation bodies share
its string pool, system-constant registry, renderer publication, VFS manager,
VFS dispatch and loaded `d3dx9_40` module. R100's owner bindings remain the
construction/retirement domain for the resulting shaders and passes.

## Process data

`GameNativeShaderProcess` now owns stable source equivalents for:

| Original address | Source storage and evidence |
| --- | --- |
| `00E13078` | Mutable DWORD initialized to 77 (`4D 00 00 00` in the image), borrowed as a live volatile reference by constant-header generation. |
| `0108D6F8` | Shared loader-zero formatting scratch, 1,024 bytes up to the independently used texture counter at `0108DAF8`. |
| `0108D6F2` | Distinct loader-zero source fallback byte. |
| `00E17654` | Distinct image-zero instance-name fallback byte. |

The scratch extent is bounded by the neighboring established global; it is
not a recovered C array declaration. The source uses separate process members,
not fixed-address native aliases. Formatting retains the original unbounded
`vsprintf` contract: normal inputs must fit the scratch region, and concurrent
or reentrant formatting is unsupported. No reset occurs between calls.

## Native VFS output

The vertex compiler always writes `.vsa` disassembly after successful D3DX
compilation. The pixel compiler writes `.psa1` or `.psa2` when both mask pointers
are supplied. `GameVfsHost::mount` exposes the existing `BE1890` path through
the same retained runtime and its failure-retention wrapper; it adds no new
provider, factory, canonicalizer or lifetime implementation.

The diagnostic mounts a workspace directory at virtual `shaderfx/debug`, with
priority 400 and the native manager's normal mount ownership. It verifies a
unique local sentinel through the actual VFS before compiling. All six native
disassembly files were written there. No `r101*` output appeared in the original
installation. The mount follows the ordinary manager drain.

## Installed-material execution

One local diagnostic links all 52 production application objects and three
current libraries. It reads the installed `shaderfx/common/3dlightning.shfx`
through the application descriptor reader, resolves its `dummy.shfx` mode
descriptor to `shaderfx/lights/dummy.shfx`, and exercises both populated modes
(indices 0 and 1, generation 3, policy F).

It runs the existing native vertex-input, system-field and interpolator
helpers, then the preliminary pixel generator/compiler and disassembly usage
parser. It rebuilds the selected mapping, generates/compiles the vertex shader,
then generates/compiles the final pixel shader with the original 500 sentinel.
The source is generated from descriptors; this diagnostic does not load or
reuse the shader cache. It records 29,574 bytes of generated HLSL and six native
disassembly files. The diagnostic engine-name prefix `r101_` isolates outputs;
neither tested name reaches the pixel compiler's special `shore` flag branch.

All four resulting COM `GetFunction` byte arrays match the independently read
installed shader-cache records **byte for byte**:

| Cache record | Bytes | SHA-256 |
| --- | ---: | --- |
| `3dlightning0F3.vso` | 1020 | `7523bf0a07b3a5d6c83361da5d94d7adad8a7dfc8879e5b8860c5bb9ce802799` |
| `3dlightning0F3.pso` | 648 | `e8def2bd794f2ee6eb394da54672db0b64e82ee2ff569dd6fc07022b2c76c482` |
| `3dlightning1F3.vso` | 1020 | `7523bf0a07b3a5d6c83361da5d94d7adad8a7dfc8879e5b8860c5bb9ce802799` |
| `3dlightning1F3.pso` | 404 | `461086913fa023379df7784fd4369150aeb10f75e10aec2f88bac3106385bb84` |

The generated shaders are reflected, wrapped, registered and attached to real
native passes through R100's application bindings, then retired through the
native pass terminal. This verifies composition of the new source services
with the existing application owner graph.

The diagnostic preserves the compiler's count-clear behavior, which abandons
the old `fields_1C` entries. It records those six identities and retires them
after generator/builder execution, outside the copied native schedule. It also
releases the two preliminary pixel shaders at that diagnostic cleanup boundary.
Those actions are not a fix or fidelity claim for the original parent's leaks.
The diagnostic does not invoke the full `B3B3C0` parent or sampler phase.

## Validation and limits

Seven original bodies plus the initial data and scratch boundary were checked
against the PE and live `bsp.gpr` analysis: 9,747 bytes with no unlisted gaps.
Existing function names and comments are preserved when appending evidence;
prior values, save/readback and refreshed exports are retained. This packet adds
application composition, not new reconstructed native function bodies.

The strict Win32 build and all three existing CTests pass. The diagnostic and
unmodified application each complete two ticks, one Present, singleton drain,
worker join and final device/API COM counts of zero. No permanent test was added.

Byte equality covers these four shader records. It does not prove all materials,
the `shore` path, native stack/register ABI, native CRT/new-handler behavior,
FH3/SEH, exceptional whole-compiler cleanup or rendered-scene/gameplay parity.
The combined application material/compiler graph, procedural sampler services,
first sampler stack value, `73BF80` startup preload and application vector
lifetime still need integration and validation. R98's unknown-value gate remains
unchanged; no sampler stack value has been guessed.
