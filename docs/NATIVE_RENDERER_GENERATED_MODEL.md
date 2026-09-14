# Actual generated-model construction

`create_native_renderer_generated_model_00b4c700` reconstructs the full 462-byte
native body over the existing actual model, mesh, declaration, stream, material
and section owners. The new material-factory overload composes numeric renderer
slot `48` with the substantive `B318B0` cache route. Existing callable-table
material and geometry APIs retain their behavior.

The source builds and the focused actual creator-registration fixture passes.
Full B4C700 execution, cold native material compilation and an active renderer
remain unproved. The cold material compiler continuation is still an explicit
required dependency, not a successful fallback.

## Original ABI and parent fields

`B4C700..B4C8CD` contains 169 instructions. ECX is the model-name `NativeString`
header, EDX is the layout-name header, and the five stack arguments are the
effect-name header, section kind, vertex count, section primitive count and index
count. It returns the constructed model in EAX and uses `RET 14h` at `B4C8CB`.
The current decompiler signature and return expression are incomplete.

`B4C725` calls canonical model-pool allocation `B74EB0`, and `B4C72A` saves the
result at current ESP plus `10h`. `B4C73B` invokes `B75030` on that allocation;
`B4C740` saves its returned model at the same location. `B4C8B9` reloads it into
EAX before returning. The original pool slot is `188h` bytes; the requested
`184h` payload size is ignored by the canonical allocation helper.

Read-only caller captures establish:

| Caller | Factory call | Following publication | Arguments |
| --- | --- | --- | --- |
| `B2BB90` | `B2BC2A` | `B2BC2F`: `[EDI+19E4] = EAX` | `DebugSpheres`, `pf43cc.mvfm`, `debugshader.mshd`, kind 3, all three counts zero |
| `B2B580` | `B2B607` | `B2B60C`: `[EDI+19E8] = EAX` | `2DSprites`, same layout/effect, kind 4, all three counts zero |

Those fields contain actual constructed model owners, not mesh or layout
pointers. This packet establishes no producer for renderer `19E0` and does not
change either caller.

## Body ordering and ownership

The function constructs a model and a mesh, then executes one x87 load from
actual `D7A260` (`BF800000`, minus one). `B4C785` stores the second float with
`FST`; `B4C78D` stores the first with `FSTP`. Both feed
`B75170(model, 0, mesh, first, second)`. The source preserves that one-load x87
sequence. The normal domain uses masked arithmetic exceptions.

The current renderer is loaded separately before declaration slot `38`, vertex
slot `5C` and optional index slot `60`. Its current numeric `D5F0A8` profile must
select `B317E0`, `B287C0` and `B288B0`. A zero vertex count uses flags `1000h`;
otherwise flags are one. Optional indices use flags one and format `65h` for
unsigned vertex count at most `FFFFh`, otherwise `66h`.

The declaration is released after vertex creation. The vertex creator is
released after `B73BB0` assigns it to mesh stream zero. `535320` then creates the
material. If indices were requested, `B73B70` assigns the returned index stream.
**Native B4C700 never releases that index creator reference.** The source keeps
it outstanding in the acquired record even after a successful return.

After `533FA0` creates the section, stores occur in order: kind at `+08`, zero at
`+0C`, vertex count at `+10`, zero at `+14`, primitive count at `+18`. `B865A0`
rebuilds its layout before `B864C0` assigns the material and `B73C60` appends the
section to the mesh. Normal creator releases then occur in order: nonnull
material (`B4C87F`), section (`B4C891`), nonnull mesh (`B4C8A7`). Each release uses
the actual `+04` count and current canonical terminal. One model creator remains
for the caller, along with the optional native index creator leak.

## Allocation unwind and source failures

Handler `CBFA70` selects FuncInfo `DF85C4`, whose two-entry map is `DF85B4`:

| State | Arm / disarm | Cleanup and captured receiver | Next |
| --- | --- | --- | --- |
| 0 | `B4C732` / `B4C752` | `CBFA60` loads ECX from adjusted EH EBP minus `14h`; `B748C0` passes it to `B74750` on canonical model pool `01090054` | -1 |
| 1 | `B4C761` / `B4C789` | `CBFA68` loads ECX from adjusted EH EBP minus `10h`; `B72F70` passes it to `B72DA0` on canonical mesh pool `0108FFF8` | -1 |

These return constructor allocations to their issuing pools. Mesh failure does
not chain into model cleanup. The second state remains armed through the first
`FST` and is disarmed before `FSTP`; native unmasked-x87/SEH behavior is outside
the source exception domain. Completed objects gain no broad parent rollback.

The shared `535320` body retains the existing allocation/constructor/release
order. Its `D95D3C` unwind entry sends state zero through `C6C240`, loading the
captured raw material from adjusted EH EBP minus `10h`, then `B17D70/B17A80`.
Allocation or constructor failure does not release the acquired effect.
Completed material is now exposed to a caller-owned output before the normal
effect release, so a terminal exception cannot hide that creator. A null effect
is rejected as an unsupported source domain: native would later access null
plus four, and neither those allocation effects nor its access-fault cleanup
are modeled as a successful material.

New acquired-output mesh, section and material helpers preserve completed
owners and any companions when host metadata allocation or canonical binding
throws. They add no native retain, release or automatic rollback. These are
additional source failure boundaries, not original native branches. The old
no-argument geometry helpers keep their existing failure policy. The new
stream-registration overload shares the old registration body but borrows only
vertex/index contexts, avoiding an unrelated projected physical-lock mapping.

## Borrowed contexts and remaining dependency

`NativeRendererGeneratedModelContext` reuses `NativeInstanceGeometryAccess` for
its established canonical model companion preparation/binding, geometry,
material and string services. It adds declaration and effect cache contexts,
index creation, concrete `GuiTextNativeLayoutServices`, and the actual renderer
and raw AA0 publication cells. The model callbacks perform host bookkeeping
only; the provider itself calls the real native model constructor.

The graph must share actual AA0, actual AA8/AA4 name storage, the same native
allocator list, renderer publication, synchronization, owner registry and child
terminal contexts. The source checks shared service references and physical
owner `SoundLifetimeAccess` identity without a manager lookup. The private
publication bindings inside `ActualNativeStringPoolStorage` are a caller
precondition: that storage must have been constructed with the same raw cells.
No `GeneratedModelLifetime` object is overlaid onto native model storage.

`NativeRendererGeneratedModelAcquired` starts empty and is not replayable.
Completed model and optional index creators survive success; interrupted
creators, companions and effect-loader child state remain inspectable on
failure. Borrowed contexts and any child-retained name identities must outlive
those failed operations. Clearing diagnostic fields does not repair them.

The raw `535320` overload captures the current renderer and selects actual
`D5F0A8+48 = B318B0`. A cold cache miss can reach
`B318B0 -> B31090 -> B2EBB0 -> B45EE0/B46950 -> B3C3A0`. Its existing
`NativeMaterialProgramCompilerTail` still requires the `B3B3C0` continuation at
`B3B513` or `B3B536`. This packet supplies neither a projected cache nor a
D3DX-result conversion to bypass that dependency. Earlier inner EH and retained
failure limits remain in force.

## Evidence and validation

Identity-checked wrappers matched 1,241 captured bytes against the installed PE:
both complete owned bodies, allocation cleanup leaves/handlers/maps, profile
words, the x87 literal, names and both caller creation prefixes. The report has
21 passing numeric direct/tail-call rows including the read-only caller sites;
16 indirect/IAT sites are explicitly outside the mechanical call gate. Handler
jumps are byte evidence, not silently counted as function-owner checks.

The strict Win32 build verified eight native seeds before configuration and
passed both CTests. The final manifested `/MD` fixture links only against the
complete rebuilt libraries. It initializes actual AA0/AA8 string services and
canonical mesh/section pools, exercises normal creator registration and a real
mesh-to-section retain/terminal cascade, then injects two canonical-bind
failures. Both completed native owners and companions remain available at
reference count one; explicit real terminals retire them and return their
slots. Canonical AA0 teardown then drains the actual string pool.

That fixture validates the changed creator-service failure policy; it does not
execute the complete B4C700 path, raw material cold compilation, original FH3 or
an active renderer. New C++ arguments, canonical host companions, native null
faults, mutable original stack aliases and game/visual parity remain explicit
boundaries. `reports/native_renderer_generated_model.json` and the ignored
immutable input/validation manifests distinguish native evidence, source,
built artifacts, tools and modules actually loaded by the fixture.
