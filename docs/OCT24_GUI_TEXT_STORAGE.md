# Actual Text field authority prerequisite

This packet supplies raw field operations for the SAME completed `1F4h` Text
allocation already constructed by `AB9650` and bound by
`NativeGuiTextReference` (canonical122). It does not admit the active Step1
factory's eight-byte identity prefix as a complete Text. It creates no native
owner, reference count, Model, tree, registry, pool or callable vtable.

`NativeGuiTextReference::storage_context()` only borrows its existing validated
93/118/122 service graph. The new entry points require bound phase, the SAME
canonical registry entry and completed93 identity. They never load, increment
or gate on the Text's native count. Raw fields remain authoritative; invocation
scratch and observations hold no synchronized Text state or ownership credits.

| Native entry | Exact normal body and original ABI | Actual operation |
| --- | --- | --- |
| `00AB8C30` | `AB8C30..AB8CD4`, 165 bytes, ECX Text, actual8B name header on stack, `RET4` | Native name cache `+1C8/+1CC`, actual Font `+108`, alpha `+1D4`, cached shader `+1EC` |
| `00AB6A70` | `AB6A70..AB6A96`, 39 bytes, ECX Text, low argument byte, tail current80 | Store byte `+77`, then current80 with DWORD3 or0; this is vslot7C |
| `00AB7200` | `AB7200..AB7283`, 132 bytes, ECX Text, DWORD state on stack, `RET4` | Current `+77` selects row14C or DWORD-wrap `state*10h+11Ch`; exact state1 x87 additions; current50 |
| `00AB6B50` | `AB6B50..AB6BC3`, 116 bytes, ECX Text, borrowed RGBA pointer on stack, `RET4` | Genuine raw `AA6870`, then current shadow188/geometry/mesh/section/material diffuse-alpha chain |

Live Ghidra prototypes confirm all four exact entries, including the now-defined
`AB6B50` function. Older evidence saying that entry was undefined is historical.
The assembly is authoritative for argument replacement, DWORD wrapping, current
field reloads and x87 spill order; pseudocode alone loses those details.

## Font and shader schedule

`set_native_gui_text_font_name_storage_00ab8c30` accepts a live native8B header,
not `std::string` or a descriptor name. It calls existing `435C40`: compare
lengths first, return equal for two zero lengths without reading either data
pointer, otherwise use the current CRT `_stricmp` boundary. An equal name
returns before allocation, lookup, alpha or shader effects.

A changed name uses actual `41DD40` with the SAME raw string-pool context.
After resize it rereads the input length, then current cache length, current
input data and current cache data in native order. BF7680 is a qualified Source
CRT `memmove` substitution (the established zero-byte-copy omission remains).
The actual cached header, after any callbacks, is passed to the existing
`NativeFontResourceOwners::find_native_font_00ac3570`. Its real10h registry,
circular list and actual Font+0C/+10 native name select the SAME image; custody
metadata only resolves that selected image. No descriptor-name lookup occurs.

The original `7371D0` getter remains an explicit already-ready registry borrow;
this packet does not create or publish its singleton. The selected image must
belong to the SAME actual-owner domain. It is published to Text+108 **without a
retain**, before reading the image's live+1C bits into Text+1D4. A miss uses the
SAME live one cell already bound by constructor93. Font registry/images must
outlive every Text borrow. Text retirement does not acquire a new Font release.

Only then capture current shader1EC. A nonnull shader loads the current CE2220
import, decrements its actual+04 exactly once, and dispatches the SAME canonical
current-zero companion on zero. The live Text1EC slot is cleared after terminal
return, including replacement by a callback. An originally null slot is not
cleared. No implicit rollback, cached-shader restore or second release is added.

## Current style and native material fields

`set_native_gui_text_flag77_storage_00ab6a70` stores the byte before capturing
the current profile/table80. `apply_native_gui_text_state_color_storage_00ab7200`
reads actual state rows with native DWORD address wrapping and has no synthetic
0..3 state bound. Every reached row must be readable actual storage. State1
uses the SAME live read-only original double cells `D5C5D0` and `D7A258`, and the
exact native x87 load/add/store sequence, even when the disabled row is selected.
The quartet is call-frame scratch, passed to current50 after all arithmetic.

`NativeGuiTextFieldCurrentBindings` is a mandatory pure actual-profile/table
view. The returned table must be the SAME captured-profile address, cover at
least84h and stay live. It cannot reenter or mutate. Known target80=`AB7200` and
target50=`AB6B50` execute these actual bodies. Any other reached target requires
its genuine provider body; there is no default no-op, fabricated table or call
through a numeric original-code address.

Raw color50 calls existing `AA6870` on the actual Text allocation. That body
reads the actual main Model4C, including when it has no geometry. The caller
must have supplied this live Model; a null Model is not silently repaired.
Color50 then rereads shadow188 at the original call sites, reads its actual
Model tail at174, and uses the existing B74640/B72B40/B732C0 and B179F0 bodies.
Only shadow material diffuse alpha changes. Native x87 multiplies current
Text174 by the borrowed original RGBA alpha, captures section+20 material, spills
the product before B179F0 and reloads it afterward. No `GuiWidgetOwner` or
`GuiLayoutWidget` receiver is cast over raw storage, and no wrapper material is
introduced.

## Calling and lifetime contract

The new header is `bsp/native_gui_text_field_storage.hpp`. Create one borrowed
`NativeGuiTextFieldStorageContext{font_resources, current_bindings}`. Each call
takes the existing full-storage `NativeGuiTextReference&`, its original native
argument, that context, and distinct caller-held `NativeGuiTextFieldStorageFrame`
and fresh `NativeGuiTextFieldStorageAcquired` backing. Nested calls need separate
retained frames/records before callbacks. Frames contain only the native local
quartet and spilled product; no preinitialization is required.

The complete existing93/118/122 creator, strings, Models, materials, profile,
imports and owner graph remain independently alive. Scratch/diagnostics cannot
alias this graph, native inputs, actual resource backing or each other. Known
region checks do not prove opaque/transitive extent separation or provenance.
The caller serializes the Text lifetime; no provider may retire the receiver
before the remaining native accesses. Pure metadata queries cannot reenter.

An escaping Source call marks the observation failed and preserves all reached
raw mutations and outstanding native release obligations. Keep the SAME frame,
record, raw backing and full provider/creator graph alive for failure disposition;
never reset/retry, infer a live object from a diagnostic pointer, clean up a
partially updated graph or drain dependencies implicitly. A callback may already
have retired a resource before throwing. These source interfaces and checks are
not original thiscall/private-stack/FH3/SEH or arbitrary FP-exception equivalence.

## Remaining composition boundary

The existing raw constructor93, sections85, cursor, glyph-vector retirement118
and canonical122 are reused, not reimplemented. The active
`GuiTextRuntimeFactory` still constructs only its raw identity prefix and keeps
content/style/layout in `GuiTextRuntimeImplementation`; it is not migrated or
admitted by this packet. Full raw content/properties, glyph child publication,
layout/geometry and factory/lifetime association remain explicit work. In
particular, pending/main raw sections must remain the SAME existing geometry,
and glyph children require their actual full-storage references and current
dispatch, not typed Text fields copied into a raw body.

The GUI Model environment's raw-name construction mode is separate integration
work. This packet neither flips an existing environment nor creates a second
environment, registry, Model or count. It does not activate a Page or manager,
claim all18 classes, change Step1 runtime behavior, or provide a raw Text factory.

Validation: `scripts/build.ps1` passed Win32 Release with the new TU privately
registered; existing `reconstructed_math` and `tool_tests` passed2/2 (16.28s).
The prior project-hook cache entry was absent and restored absent. SHA-256 of
tracked `CMakeLists.txt` and `cmake/startup.cmake` matched before/after; no tracked
CMake edit is included. Local evidence is `local/gui-text-field-build.log` and
`local/gui-text-field-cmake-restoration.json`. No new test, executable/game run,
runtime, visual or binary ABI proof is claimed.

`ghidra_annotate.py --apply` verified the configured `bsp.gpr` and
`/battlestationspacific.exe`, retained names and prior comments, appended this
bounded evidence, and saved the project. Previous annotations are recorded in
`local/ghidra-annotations-20261024T122030Z.json`; all four exports were refreshed.
