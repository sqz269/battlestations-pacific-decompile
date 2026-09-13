# Actual compiler inputs, reflection and special texture loading

Addresses: 00b35be0, 00b372d0, 00b34e20, 00b3aea0, 00b3a750, 00b5bc60,
00b5b830, 00b5b960, 00b2c2d0, 00b3f2d0, 00b5bf70, 00b5b9e0,
00b5bbc0, 00b5bed0, 00b5bd10, 00b5be10, 00b5df70, 00b5bb20,
00b5df00, 00b5ba80, 00b36800, 00b34aa0, 00b346e0

This integration advances actual shader-builder fields and compiled metadata,
and the cube/volume arms of the native texture loader. The modules operate on
the existing native storage and ownership domains. They do not complete the
material compiler continuation or demonstrate a running rebuilt game.

| Component | Accepted worker evidence | Integration |
| --- | --- | --- |
| System fields | `006672b7`, `NATIVE_SHADER_SYSTEM_FIELDS.md` | Six owned files imported in `5b449fee`; actual B0h builder and 1Ch fields |
| Reflection | `c23ab65c`, `NATIVE_COMPILED_SHADER_REFLECTION.md` | Reviewed merge `4900b5c3`; real D3DX table and existing 88h owner |
| Cube/volume loading | `6ba1b7cc`, `NATIVE_TEXTURE_LOADING_CACHE.md` | Eight owned files imported in `653cd6b1`; shared canonical resource companions |
| System constant registry | `c8e54592`, `NATIVE_SYSTEM_CONSTANT_REGISTRY.md` | Reviewed merge `82ab9e0a`; actual 52 records and shared singleton lifetime |
| Interpolators | `8ac49e42`, `NATIVE_SHADER_INTERPOLATORS.md` | Reviewed merge `1ce1c128`; actual field selection and two-byte mappings |

The system-field constructor clears the actual name header before copying and
publishes scalar fields only after that copy returns. Vertex and pixel appenders
each allocate sixteen independent fields and retain partially acquired storage
on host failures. Native interpolator selection and mapping execute between the
two appenders. Their accepted implementation reads descriptor70 then74, preserves
separate usage cursors across both, and appends to the existing mapping arrays.
These helpers still require explicit composition in the compiler continuation.

Reflection consumes the live registry publication and real D3DX constant table.
It writes system counts before register indices, appends unknown FLOAT records
to the actual owner, and writes the end register and sampler mask before releasing
the table. Its retained operation preserves the table, temporary names and actual
array acquisitions if a borrowed operation fails.

The actual registry constructor produces all 52 records in the existing 20h
layout. Its base publishes the live singleton before initializing the derived
array. Array growth, forward old-name release, reverse shrink and normal terminal
cleanup retain their native order. The same publication and lifetime domain now
feed an original/source reflection fixture; no projected registry is used in that
composition. The registry binding permits one unfinished operation, and retains
the exact current copied source if a borrowed callback fails.

Special texture loading retains the device captured before image inspection.
The retry arm obtains the current renderer for recovery and re-reads stream size
and data while reusing that captured device. Actual cube/volume constructors,
pools and source references join the existing canonical resource domain. Guard
exit precedes common stream release. Volume destruction notifies before releasing
its retained source; cube destruction has the independently documented order.

## Preserved validation

The first combined checkpoint is
`local/checkpoints/653cd6b1/compiler-path-temporary/validation.json`. Its 49
hash-verified artifacts preserve the exact temporary include, production sources,
combined library, build log and five focused fixtures. The production Win32 target
and both CTests passed. That checkpoint deliberately identifies its two temporary
source registrations and is not a default-build claim.

The five fixture programs cover installed-descriptor system fields, original/source
real-D3DX reflection (including the retained-failure child), actual physical-file
cube/volume loading, existing texture record storage, and existing resolution-failure
retention. Across the three component reports, 377 numeric call/transfer rows passed,
including four resolved indirect transfers; symbolic COM/virtual calls retain their
separate evidence limits.

All five components are now registered in the normal build. The final default
Win32 build at `82a2179e` and both CTests passed. Thirteen focused fixture
programs passed against the registered library. Their original run at
`a52a7891` preceded a names/docs/reports-only merge; all three
linked libraries and all pinned runner/fixture inputs remained byte-identical,
and the default build was rerun after that merge. The final five component reports
have 847 checked numeric transfer rows, including four resolved indirect rows,
with zero failures.

The immutable final checkpoint is `local/checkpoints/82a2179e/compiler-path-default/validation.json` (146 artifacts).
`reports/native_compiler_path_integration.json` records the exact code revision,
worker archives, Ghidra synchronization and validation limits. Local CMD runners
now propagate every nonzero compiler/linker result and check the environment setup;
archived original runners remain unchanged.

## Analysis synchronization and limits

The primary applied all 23 accepted names/evidence comments through the Ghidra
write lock, preserved previous annotations, saved the project, read back the
comments, and refreshed the affected exports. In particular, B3F2D0 is the actual
volume-pool allocation wrapper, not a static destructor.

These are new MSVC Win32 interfaces, not drop-in native entry points. Private FH3
unwinding is not reproduced. Failed operations retain acquired state and prohibit
unsafe retirement instead. Reflection accepts populated successful COM outputs
and in-range system semantics; native invalid-output/out-of-range behavior remains
outside its domain. The texture fixture does not execute recovery, enabled-guard
reentry, cache bootstrap or the complete original loader. Compiler-tail success,
full rendering parity and gameplay remain unverified.

## Native constant headers and vertex compilation

The next integrated batch adds eight actual constant-header/formatting routines
and three vertex-compilation/physical-write routines. The constant header uses
the same 52-row registry, actual builder output, shared scratch and pooled strings.
It preserves signed formatting, captured temporary releases, current count/limit
reads and DWORD cursor wrap. The original/source fixture compares exact text and
allocation traces, including retained failure state. Array-base replacement and
scratch mutation during allocation are assembly-reviewed, not exercised by that fixture.

Vertex compilation captures the actual device before D3DX9_40 compilation and
the VFS manager before name allocation. Physical `.vsa` writes and diagnostic
cleanup precede vertex-shader creation; code/message releases follow it. The real
HAL/VFS fixture compares shader function bytes and 158-byte diagnostics, then
checks code-null messages, retained allocation failure and physical count/carry.
The failure leg can truncate the final `.vsa`; the earlier successful comparison
is recorded in the pinned fixture log. Relocated original instructions, rebound
dependency calls/table entries and shared existing helpers remain explicit limits.

The final default build at `6fbdee9e` passed both CTests. All 15 focused fixture
programs passed at `44d9fdfd`; the intervening merge contains only
docs/reports. All three libraries and all 40 runner/fixture input hashes stayed
unchanged, and the default build was rerun after that merge. Seven component
reports have 936 checked numeric transfer rows and zero failures, including the
same four resolved indirect rows. The 11 new names/comments were applied under
the Ghidra write lock, saved, read back and exported with prior values preserved.

The immutable checkpoint is `local/checkpoints/6fbdee9e/compiler-leaves-default/validation.json` (167 artifacts). The earlier
146-artifact default checkpoint remains unchanged. Pixel compilation and struct
declarations continue in separate leased worktrees. These integrated helpers do
not install the full native source generator or a successful B3B3C0 continuation;
native FH3, full material compilation, rendering parity and gameplay remain unproven.

## Validation after concurrent string-storage integration

The later shared checked-string/input-settings code was merged before another
default build and a fresh run of all 15 focused fixture programs. Both CTests and
all 15 fixtures passed at `fa6211f1`. The final validated revision is
`fa6211f1`; all seven accepted component source/header/report hashes remain
unchanged. No result from the earlier library was substituted for this fresh run.
The immutable final checkpoint is `local/checkpoints/fa6211f1/compiler-leaves-final/validation.json` (174 artifacts), including
the changed shared storage sources. The earlier 167-artifact checkpoint remains
preserved. This refresh changes neither the reconstruction scope nor the stated
native ABI, FH3, full compiler and gameplay limits.

## Validation after concurrent shared-source integration

The later shared source code was merged before another
default build and a fresh run of all 15 focused fixture programs. Both CTests and
all 15 fixtures passed at `7a060521`. The final validated revision is
`7a060521`; all seven accepted component source/header/report hashes remain
unchanged. No result from the earlier library was substituted for this fresh run.
The immutable final checkpoint is `local/checkpoints/7a060521/compiler-leaves-final/validation.json` (184 artifacts), including
the changed shared storage sources. The earlier 167-artifact checkpoint remains
preserved. This refresh changes neither the reconstruction scope nor the stated
native ABI, FH3, full compiler and gameplay limits.

## Actual field declarations and pixel compilation

Four native line/field/struct routines and the full normal B61280 pixel compile
path are now integrated. Struct emission uses the actual 1Ch fields, 0Ch lists,
B0h builder and pooled strings. It preserves self-append/current source pointers,
unsigned 711370 width/index formatting, live list reads and post-row descriptor
vPos flags. Existing helper cleanup can leave stale output headers; retained
operations keep that distinction and require the documented diagnostic cleanup.

Pixel compilation creates the actual shader before diagnostics. With both mask
pointers present, disassembly sets the returned HRESULT. The texcoord 500 sentinel
is read separately for filename selection and post-stream parsing. Parsing preserves
captured line pointers, per-component OR writes and the final captured full-string
release. Root review corrected newline searches to subtract CURRENT full-string
data after strstr and changed the inline copy to the original overlap-capable
contract. Those CRT-adjacent re-reads are assembly evidence, not live mutation
coverage from the real-CRT fixture.

The default Win32 build and both CTests passed at `0114668b`. All 17 focused
fixture programs passed at `a3c7f7f7` against the same three
registered libraries and 45 pinned runner/fixture inputs. The new struct fixture
compares exact text/allocation traces, aliases, live list/descriptor changes and
retained/borrowed-helper failure state. Six pixel fixture cases compare real
D3DX/HAL shader bytes, 312-byte diagnostic output, masks and string traces,
including two post-cleanup sentinel transitions and retained-failure guard 77.
This remains synthetic-HLSL and relocated-instruction/shared-helper evidence.

Nine component reports have 1,096 checked numeric rows, zero failures, including the
same four resolved indirect rows. Five new names/comments were saved under the
Ghidra write lock, read back and exported; old values remain in the archived
journals. The immutable checkpoint is `local/checkpoints/0114668b/native-source-default/validation.json` (213 artifacts),
with the earlier 184-artifact checkpoint preserved. Samplers, field initialization
and interpolator source emission continue in separate leased worktrees. Full
native source generation, B3B3C0 continuation, FH3, rendering and gameplay remain
unproven.

## Actual sampler, field, interpolator and shadow helpers

Ten native helper bodies now operate on the existing actual builder and pooled
string domain. Sampler declarations preserve stage-byte selection, shared slots
and current descriptor/list reloads. Field decoding captures its unsigned prefix
limit once and rereads field pointers after swizzle allocation. Interpolator
pack/struct/unpack preserves signed group arithmetic, current mappings and the
original captured-versus-current temporary release lengths. Shadow helpers read
builder byte AA only after the intro append returns, then retain that branch.

The default Win32 build and both CTests passed at `743847de`. All 21 focused
fixture programs passed at `743847de`; the three library hashes
and 192 runner/transitive input hashes remained unchanged.
The new fixtures compare original/source text and pooled traces, selected live
mutations and retained failure/retirement behavior. The interpolator packet also
received an independent review of all 587 owned assembly instructions.

Thirteen component reports have 1,249 checked numeric rows and zero failures,
including the same four resolved indirect rows. Ten names/comments were saved,
read back and exported; prior values remain in the checkpoint journals. The
immutable checkpoint is `local/checkpoints/743847de/native-generation-helpers-default/validation.json` (408 artifacts), with the earlier
213-artifact checkpoint preserved. Full vertex/pixel source generators continue
in separate leased worktrees. Full compiler continuation, native FH3, rendering
and gameplay remain unproven.

## Complete actual vertex and pixel source generation

Six more normal bodies are reconstructed: B39110/B39880 source generation,
B347E0/B34890/B34920 compiler lookups/state application, and the B20190
vertex-texture/render-target format query. They use the existing actual
storage and preserve captured versus current reads and retained failure state.
B34C70 is separately defined and named as an analyzed record producer.

The default Win32 build and both CTests passed at `e2abc28f`. All 25
focused fixtures passed at `e2abc28f` against unchanged
library hashes and 208 runner/transitive input hashes.
Seventeen reports contain 1,547 checked numeric call rows and zero failures.
Seven names/comments were saved, read back and exported with old values retained.
The checkpoint is `local/checkpoints/e2abc28f/native-compiler-source-default/validation.json` (448 artifacts); earlier checkpoints remain.

The vertex fixture reads installed descriptors and compiles byte-identical
source to identical vs_3_0 bytecode. Pixel fixtures compare ten complete
original/source outputs and pool traces over actual producer storage populated
locally; the no-fog depth output compiles as ps_3_0. Both fog outputs produce
the same missing-cFogDirColor error with comment-only descriptor Constants.
Descriptor/pipeline context remains unresolved and its negative evidence is
preserved. The format query also agrees with actual Direct3D9 for two formats.

Full compiler continuation, sampler loader prerequisites, concrete shader
deletion providers, native FH3, rendering and gameplay remain unproven.
The separate CBF400 post-free tail repair still lacks full stored-body coverage.

## Actual shader construction and terminal ownership

Twenty more complete normal bodies are reconstructed: three pass-slot setters/
removal, three descriptor-sampler leaves, eight shader construction/registry
helpers, and six actual destructor/scalar-delete/renderer-unregister bodies.
They share the actual wrapper storage, raw+04 atomic, renderer arrays, pooled
strings, singleton allocation domain and current COM interface. Both pass-slot
setters now exercise a concrete canonical terminal provider. Eight lifetime
original/source comparisons use constructor-produced wrappers and real HAL COM
objects; wrapper acquisitions and companions are cleaned after use.

Default Win32 compilation and both CTests passed at `05e953d4`. All29 focused
fixtures passed at `05e953d4` with three unchanged library
hashes and 256 unchanged runner/transitive fixture inputs.
Twenty-one reports contain1,674 checked numeric call rows and zero failures.
Twenty-two names/comments were saved, read back and exported with old values
retained. The checkpoint is `local/checkpoints/05e953d4/native-shader-ownership-default/validation.json` (688 artifacts).

The first combined attempt passed27 fixtures but found missing staged binary
inputs in the construction fixture. Three inputs were restored from the pinned
worker archive and strict open/length checks were added. Construction and
lifetime then passed against the unchanged production libraries. The27 earlier
results were retained after verifying their inputs/logs were unchanged; the
failed-run trace, diagnosis and corrected inputs remain in the checkpoint.

The earlier incomplete CBF400 repair is now superseded: official recreation
preserved its existing compiler name and comment, saved the complete11-byte
body through CBF40A, and exported all five instructions with no gap. Earlier
checkpoints remain unchanged. Source exception cleanup evidence does not prove
original FH3 delivery. Full sampler/event/online-manager and compiler closure,
shader binding/drawing/readback and gameplay remain unproven.
