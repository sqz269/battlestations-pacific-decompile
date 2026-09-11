# GUI color publication into actual material storage

`set_gui_color_00aa6870` now follows the existing canonical model to its raw
mesh, element zero and section `+20h` material. It resolves that pointer through
the **same** `NativeRenderActualOwners` domain used by the model, requires the
matching live `NativeMaterialReference`, and writes the actual
`NativeMaterialStorage::lighting_38` diffuse quartet. No `MaterialCloneState`
or copied `MaterialLighting` participates in COLOR publication.

This corrects the required material-projection boundary described in
`GUI_MATERIAL_BINDING.md`. Registration remains a separate semantic operation
over `MaterialCloneState`; widget/page disposal coordination, borrowed ClipBox
field lifetime and native parameter registration are still required. This
packet does not expose a complete GUI adapter or validate rendering.

## Native evidence and ABI

Analysis used only the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through read-only `bsp.py ghidra` commands that
verify the target. Saved bytes were compared with the installed PE. The report
preserves the previous Ghidra names and full comments; no annotation was changed.
Descriptive names remain hypotheses.

| Address and full span | Original ABI | Established behavior |
| --- | --- | --- |
| `00AA6870..00AA68E9`, 122 bytes | ECX widget; stack RGBA pointer; `RET 4` at `00AA68E7` | Copy four source DWORDs into widget `+50h..+5Ch`; inspect model geometry and mesh count; reload geometry; copy four source DWORDs into element-zero material diffuse. |
| `00B179F0..00B179F5`, 6 bytes | ECX material; ignored stack slot; EAX pointer; `RET 4` | `LEA EAX,[ECX+38h]`; no load of slot, copy, flag write or arithmetic. |

Read-only support spans are `00B74650..00B7465B` (raw model `+180h`
presence), `00B74640..00B74648` (that same geometry pointer; ignored index),
`00B72B40..00B72B43` (mesh `+58h` count), and `00B732C0..00B732CC`
(mesh `+54h` pointer table indexed by DWORD). These existing helpers were not
claimed or changed.

At `00AA6886`, native code loads node `+4Ch` between the third color load and
store. It then calls presence, geometry, count, geometry again, element zero,
and diffuse getter in that order. The element's material pointer is at `+20h`.
Each quartet uses four ordered DWORD load/store pairs; source words are read
again for the material copy. Self-source and forward-overlapping material
sources therefore retain their native alias behavior. There is no alpha
multiplication, child traversal, lighting-enabled `+10Ch` mutation or named
parameter registration. No reference count changes occur in this routine.

The host already projects widget `+5Ch` into layout color and transform alpha;
that projection is maintained by a word copy. The raw material has one live
17-word lighting record at `+38h`, and the new diffuse getter returns its
existing first-word address. This is a new C++ interface, not an ABI-compatible
replacement for the game function.

## Required canonical boundary

`GuiMaterialBindingServices::actual_owners` replaces the old COLOR-only
semantic material callback. It must be the exact domain referenced by
`NativeModelOwner::environment.retained_owners`; callers retain their existing
registry implementation. The binder creates no registry or reference count.

The pre-existing live canonical model guard remains before any color writes.
An inherited AA6870 vtable slot does not make a plain `cGroup` page root safe:
its `+180h` is a child-array capacity, not model geometry. The additional
same-domain guard also precedes all writes. Once geometry is present and its
count nonzero, the raw section material must resolve to `NativeMaterialReference`
whose storage address is that raw pointer, whose borrowed atomic reference is
the actual storage `+04h`, and whose live raw vtable is `00D5E520`. These are
host preconditions; no equivalent native RTTI check is claimed.

The material remains owned by the actual section. Publication borrows it for
the call and acquires no token. The separate registration callback still runs
AA9F10 then semantic B18A40 and still requires real widget/layout lifetime.
No change is made to material cloning, texture retention, effect ownership,
parameter records or widget deletion ordering.

## Focused validation

The ignored `local/gui_raw_material_color_probe.cpp` fixture is one combined
actual model/mesh/section/material lifetime case. It uses their reconstructed
native pools and canonical companions with the installed model, node, mesh,
section and material tables/constants. Null effect and empty parameters are
legitimate constructor inputs. No effect, shader, stream, layout resource,
registration token or renderer draw is synthesized.

For comparison, the complete original AA6870 byte span calls its five
captured original leaf routines after relocation. The native receiver is a
small controlled widget storage block containing the actual model pointer;
the geometry, section and material are the very same raw objects used by the
host call. Between comparison calls, the fixture restores the material bytes
to the common starting state; it checks all four owner counts before and
after each call.

The case covers external raw color words (including signaling-NaN payload,
negative zero and a subnormal), widget self-source, a source one lighting word
ahead, and source material `+34h` whose forward writes propagate the first
word. It compares the entire 110h material and all four widget color words,
checks flag `+10Ch` remains `81h`, and requires exactly one canonical material
resolution. Empty geometry and zero mesh count stop without material lookup;
a mismatched supplied owner domain fails before changing widget color.

All model/mesh/section/material counts are one at publication. Teardown uses
the existing runtime logical release/destructor composition, releases the
actual model-to-mesh-to-section-to-material chain to zero, and returns the
material slot through the concrete pool. There is no extra retained node or
resource reference protecting cleanup. Shared allocator-list trimming must
leave all five pools with zero slabs and no registered canonical owners.

Final command results and ignored artifact hashes are in
`reports/gui_raw_material_color.json`. Scope is host build, fixture lifetime,
and bounded original-byte agreement; raw widget ABI, native shader/effect
integration, complete parameter ownership and game behavior remain unproven.
