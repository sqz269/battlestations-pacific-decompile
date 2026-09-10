# Material entry renderer reloads

`00B44750` and `00B43410` consume the live renderer pointer at `00F8D394` at
specific points. A renderer reference captured at dispatcher construction or
`bind_and_draw` entry loses replacements made by required owner operations or
device callbacks. The typed environment now borrows the actual pointer slot as
`D3D9StateCache* const volatile&`; a null owner at a required capture is an
explicit binding error. The slot and every captured renderer must remain alive
across callbacks. This does not introduce a replacement cache or renderer.

## Native capture schedule

The assembly supplies the schedule; the decompiler hides several saved-register
lifetimes. `00B44750` takes ECX=selected pass, stack entry/override, and returns
with `RET 8`. `00B43410` takes ECX=pass, six stack arguments, and `RET 18h`.
The C++ interfaces remain semantic projections, not those memory layouts/ABIs.

| Native load | Consumer and lifetime |
| --- | --- |
| `00B447A2` / `00B44810` | Reload for each instanced/noninstanced stream bind. |
| `00B447C4` | Reload for tagged instanced stream frequency. |
| `00B447DD` | Capture before `00B855A0` instance-count getter; use saved EBP for frequency at `00B447EE`. The typed getter is a direct live section-word read. |
| `00B4482F`, `00B4483E` | Independent reloads for noninstanced frequencies 0 and 1. |
| `00B4485A` | Save the plane owner in stack+20 before the effect-change and plane-owner/math chain. Restore uses this capture at `00B44A56`; append reloads the saved stack value at `00B44A4A`, then calls `00B25040`. |
| `00B44A62` | Reload after the plane operation for vertex layout binding. |
| `00B44A96` | Capture after index-base getter and before vertex-base getter; use saved EBX for index binding at `00B44AB6`. Typed getters read retained stream fields. |
| `00B43414` | Capture in EBX for render block, sampler block, VS shader, and PS shader binding at `00B4342C`, `00B43437`, `00B43460`, `00B4346B`. Preserve this owner even if those calls replace the live slot. |
| `00B4344C` | Independent reload for nonnegative material+104 render-state override. |
| `00B434F8` | Reload for each selected texture after any effect-texture callback. |
| `00B4355C` | Reload for VS constants after required constant construction. |
| `00B43594` | Reload for PS constants, including a replacement from the VS device callback. |
| `00B435B8` | Reload after optional material+08 callback for either draw path. |

The required plane interfaces receive the captured `D3D9StateCache&` explicitly.
Adapters must use its actual camera-frame companion throughout the plane
operation. Re-reading the environment slot inside the final append would lose
the native saved-owner behavior.

## Reusable material-tail upload

`upload_material_entry_constants_00b43541` extracts `00B43541..00B4359F` without
changing its register arithmetic:

```cpp
bool upload_material_entry_constants_00b43541(
    const CompiledMaterialPass& pass,
    MaterialEntryConstantState& constants,
    D3D9StateCache* const volatile& renderer,
    HRESULT& first_failure,
    std::int32_t& vertex_count,
    std::int32_t& pixel_count,
    std::string& error);
```

The pass is the actual retained compiled pass, and the constant state borrows
the actual `00E13078` start and shared VS/PS blocks. Both diagnostic counts are
the original signed 32-bit end-minus-start differences. After an attempted VS
upload, the helper reloads the shared start (`00B43575`) and PS end for the gate
(`00B4357A`), while passing the **original** PS count (`00B4358C`). VS and PS
capture their renderer separately. A failed HRESULT is accumulated through the
provided reference and does not suppress PS, the later callback, draw, or
diagnostics. Missing owners or insufficient actual storage are binding errors.

The helper does not build system constants or choose a tail start. A caller
which has completed the separate system-prefix operation can pass the same
live state whose actual start is 77 or later. It must not substitute a copy of
the start, pass metadata, renderer slot, or constant blocks.

## Proven render-state correction

`00B43453` pushes `0x39` (57), which is `D3DRS_STENCILREF`, for material+104.
`00B24460` forwards that state index directly to device virtual+E4; there is no
enum translation. The previous C++ `D3DRS_ALPHAREF` (24) was incorrect and is
now `D3DRS_STENCILREF`.

Getter `00B17320` is exactly `MOV EAX,[ECX+104h]; RET`. Its currently saved name
`BSP_Material_GetAlphaReferenceWord` is misleading. A conservative proposed
name is `BSP_Material_GetWord104`, with a comment describing this proven
stencil-reference consumer; broader property meaning remains unestablished.
The C++ field remains `word104`. The historical
`docs/MATERIAL_ENTRY_DISPATCH.md:93` and
`reports/material_entry_dispatch_audit.json:239,327-329` contain the older
ALPHAREF interpretation. The integrator owns their correction and any shared
Ghidra/ledger annotation; this packet does not mutate either.

## Verification and limits

Target verification through `bsp.py ghidra` confirmed project `bsp`, program
`/battlestationspacific.exe`, and configured project file
`C:/Users/sqz269/bsp.gpr`. Live native bytes for `00B44750` (948 bytes) and
`00B43410` with its local jump table (624 bytes) exactly matched the installed
executable preimages. Hashes and annotation preimages are recorded in
`reports/material_renderer_reloads_audit.json`.

`scripts/build.ps1` passed MSVC Win32 Release `/W4 /WX /fp:strict` compilation
and the existing `reconstructed_math` test. The existing ignored selector
fixture was adapted to the new environment/plane API: 21 native differential
cases produced 24 matching selected program/override identities. Its existing
COM-failure continuation case also passed.

One additional ignored differential scenario executes copied native
`00B44750` into `00B43410`, with explicit synthetic dependencies, and compares
19 ordered events against the dispatcher. Stream and frequency calls replace
the live owner; the plane-builder callback replaces it before append; state
and shader calls replace it around the saved block/shader owner. The VS upload
fails after replacing renderer B with C, moving start 77 to 78 and PS end 81 to
79. PS still uploads the original four vectors on C at start 78. The material
callback replaces C with A, and the draw runs on A before original counts 3/4
reach diagnostics. This scenario also exposed and now verifies the exact
render-state index 57.

The fixture redirects required native renderer/owner callees. The unresolved
plane math interval is represented by its required callback: the call at
`00B448A3` returns a synthetic plane token and the instruction at `00B448A8`
jumps to the unmodified saved-owner append sequence at `00B44A4A`. This tests
the capture across that dependency without claiming its math is reconstructed.
The selected fixture path is noninstanced/nonindexed; the other capture points
are assembly-reviewed. Shader/device objects and geometry are synthetic.
There is no full pipeline, original-ABI replacement, GPU, or game validation.
