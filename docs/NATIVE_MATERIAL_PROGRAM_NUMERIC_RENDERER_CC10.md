# Numeric renderer domain for material secondary passes and pruning

This packet supplies the existing program loader with an explicit numeric
renderer domain for B45E00 secondary construction and B5F160 pruning. It reuses
the complete B44B10 texture-cache constructor and B1FF50 capabilities accessor
in `native_material_compiler_providers`. Original D5F0A8 table words select
those implementations; they are never invoked as host function pointers.

The four changed source/header files are `native_material_pass_copy.hpp/.cpp`
and `native_material_effect_programs.hpp/.cpp`. The optional trailing domain
keeps existing callable-renderer callers valid. No application startup,
material compiler, cold effect cache, or B107F0 gate is activated.

## Native evidence and call boundaries

The saved project is `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; every live query used the verifying BSP wrapper.
Ghidra was read only. Nine captured blocks match the installed PE exactly:

| Block | Half-open byte interval | Bytes | Last instruction |
|---|---|---:|---|
| B45E00 secondary builder | [B45E00,B45EE0) | 224 | B45EDF RET, 1 byte |
| B5F160 pruning | [B5F160,B5F3ED) | 653 | B5F3EC RET, 1 byte |
| reused B44B10 constructor | [B44B10,B44BE6) | 214 | B44BE5 RET, 1 byte |
| reused B1FF50 getter | [B1FF50,B1FF57) | 7 | B1FF56 RET, 1 byte |
| D5F0A8 +64 | [D5F10C,D5F110) | 4 | original word B319B0 |
| D5F0A8 +104 | [D5F1AC,D5F1B0) | 4 | original word B1FF50 |
| existing B5F6A0 finalizer | [B5F6A0,B5F6E0) | 64 | B5F6DF RET, 1 byte |
| existing B46950 variants | [B46950,B4699E) | 78 | B4699B RET4, 3 bytes |
| B45EE0 secondary call | [B463B6,B463BB) | 5 | B463B6 CALL B45E00 |

B45E00 and B5F160 use ECX actual effect/pass and plain RET. B44B10 uses
ECX fresh 88h pass and returns that pointer. B1FF50 is exactly
`LEA EAX,[ECX+1B18]; RET`: it performs no capability read itself. These source
interfaces remain C++ adapters, without an original ABI/FH3/SEH claim.

The report contains all 57 call instructions in those code blocks: 55 direct
rows passed `verify_report_calls.py`; the two indirect rows are B44BA8
renderer+64 and B5F333 renderer+104. Their targets are separately established
by the captured original table words. The verifier explicitly skips indirect
rows; its direct-call result is not presented as indirect-call verification.

## Borrowed domain and retained operation

`NativeMaterialProgramNumericRendererDomain` borrows the application's same
`NativeMaterialPassConstructionAccess`, genuine `NativeTextureCacheContext`,
and original D5F0A8 table. It owns no native resource, pool or registry. The
construction access supplies the canonical 0108FBF8 pass pool, string domain,
state registration and current F8D394 publication cell. The texture cache and
its loading context must use the same strings, current renderer and canonical
owner domain, with their real installed-file/cache providers and backing.

Before native effects, numeric secondary admission checks a fresh frame,
construction/copy lifetime identity, actual string identities, current-renderer
cell identity with texture loading, and the required registration callback.
Program admission and direct finalization check the exact construction-access
object and current-renderer cell against `NativeMaterialEffectProgramsContext`.
These checks establish source composition, not the lifetime of arbitrary raw
storage supplied by a caller. Canonical registration still requires the real
same-domain callbacks; a table token alone is insufficient.

Each of the two retained `LoadFrame` instances owns an independent
`NativeMaterialSecondaryPassFrame`, including the existing complete
`NativeMaterialCompilerPassConstructionFrame`. It exists before B45E00 is
entered. `secondary_pass_frame(0/1)` exposes immutable acquisition diagnostics
for either invocation, including after the variant loader advances to its
second load. It returns null for an unused/out-of-range frame. A completed
frame is metadata; its published native pass is owned through the canonical
actual +04 reference count. No extra credit is added.

The numeric domain, contexts, original tables, effect/descriptor/name backing
and complete program operation must remain alive through all dependent uses
and native owner retirement. In particular, B46950's first and second loads
cannot share scratch constructor storage. Existing descriptor bindings and
operation retirement obligations remain in force.

## Construction, publication and failure ordering

B45E00 tests the current primary C8, allocates through B41210 at B45E39 and
captures the raw pool slot before B44B10 at B45E4C. That existing constructor
builds the actual base and three registered state owners, constructs the real
`white.tga` temporary, then loads through current F8D394/profile+64/B319B0.
Its result transfers to fallback84 without an extra retain. The temporary's
data is captured before fallback publication and released afterward.

Only after the constructor returns does B45E00 reload C8, publish the result
at secondary100, register the complete pass metadata and call B455C0. The
source capture therefore precedes publication and the metadata callback. Each
following setter reloads current secondary100: F=0, 1B=1, 13=5, 14=6, followed
by a fresh current pointer for root08=1. The remaining thirteen secondary
slots are cleared; absent primary C8 clears all fourteen. Prior secondary
owners are not released. The shared normal body preserves the legacy path's
same schedule and legacy constructor-unwind slot return.

The numeric constructor has a different source failure contract: it retains
the actual raw pass, partially or fully constructed base/states, temporary and
texture acquisition. The enclosing numeric frame records failure and never
returns that slot to the pool. Copy/setter failure similarly preserves earlier
publication and registration. Native-site, raw-slot, constructor-state,
captured-source, publication/registration and completed-slot diagnostics are
retained. Running/failed frame destruction terminates; neither a destructor
nor a catch fabricates cleanup, retries the operation or marks it settled.
The original native exception-state numbers are diagnostics, not execution of
the original EH transport or a proof of complete failure cleanup.

## Current capabilities and state finalization

B5F160 performs the six original conditional removal groups first. At B5F325
it captures current F8D394, at B5F32B the current raw profile, and at B5F32D
the profile+104 word. Numeric dispatch reads each once in that order, validates
D5F0A8/B1FF50, and calls the recovered accessor. B5F335 then reads the actual
returned byte3D (renderer+1B55). A nonzero byte removes state9A; zero removes
stateB5. The original thirteen unconditional removals follow. No copied
capability structure or earlier profile preflight changes that native schedule.

B5F6A0 still prunes before its three real canonical cache children. It captures
the next input before publishing the preceding result at18/1C/20 and reloads
the current renderer around each call. This packet does not replace those
children with identity callbacks or logical material projections.

## Verification and limits

Strict MSVC Win32 `scripts/build.ps1` and the existing three CTests pass. No
tracked test was added. The single ignored current-application probe was
rebuilt from current game_main, 70 current application objects and three
current libraries with `/MD /fp:strict /link /MANIFEST:EMBED`, then launched
only through `tools/run_game.ps1`. It reuses old fixture forwarding/setup,
never an old executable.

Its success mode constructs an actual B407A0 effect and B44B10 primary with
real error/white textures, then exercises numeric B45E00 and B5F160. The fixture
explicitly supplies controlled primary slot inputs and secondary preimages;
it does not claim those values came from a compiler/program load. It verifies
fourteen secondary outputs, a distinct real pooled pass, canonical owner
registration, actual live capabilities byte3D=1, conditional pruning, real
effect/pass/state/fallback retirement and native pool returns. The application
then presents two frames, skips one present, drains its singleton domain and
reports final device/API COM counts 0/0, exit0.

Its separate failure mode deliberately removes the borrowed original table
binding. This reaches the existing source binding validation at B44BA8, after
the actual base/states/string and persistent texture child have been acquired.
It verifies constructor diagnostic EH state1, retained canonical state
registrations, pool free slots31->30, unchanged secondary100 and no publication
or cleanup. The logged `ExitProcess(0)` intentionally bypasses failed-frame
destruction and CRT cleanup. This is a reached source-binding failure test,
not a native hardware fault, original exception transport or cleanup proof.

Full B45EE0/B46950 composition is build checked, not executed by this leaf
probe. Cold compiler/effect-cache invocation remains separate: the actual
installed descriptor first-source0 sampler stack preimage, procedural sampler
runtime and actual F8BBF0 shadow-target owner/publication still need admission.
No guessed policy byte, dummy compiler, sampler residue or texture callback is
introduced. Bloom's fourth borrowed DWORD backing, distortion cleanup
preimages, and full B107F0 runtime admission remain unchanged.

`reports/native_material_program_numeric_renderer_cc10.json` indexes the
ignored native bytes/listing, build/call logs, exact probe inputs/current
dependencies, executable, both process logs and preparation scripts by SHA-256.
