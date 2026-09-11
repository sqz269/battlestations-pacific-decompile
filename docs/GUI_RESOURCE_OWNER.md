# GUI manager resource owner

`GuiResourceOwner::initialize_00aa5e20` implements the manager's retained texture,
page and child slots, composing `GuiPageRegistry`, `GuiLayoutHost` and the existing
`load_gui_page_00aa5840` / `find_child_by_name_00aa7e00` implementations. Names are
descriptive hypotheses. This is a new C++ interface, not a native ABI replacement.

The target was verified as project `bsp`, program `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`, before the read-only analysis/export
batches. No Ghidra annotation or prototype mutation was performed. Existing
descriptive names are retained in the ledger. `reports/gui_resource_owner.json`
records the current byte comparisons and validation.

## Native sequence and ownership

`00aa5e20` takes the manager in ECX, has no stack arguments and ends with plain
`RET` at `00aa633e`. The manager constructor clears the texture slots but leaves
the page/child slots and byte `+84h` unwritten until this initializer runs. The
initializer has no already-initialized guard.

| Native instructions | Reconstructed operation |
| --- | --- |
| `00aa5e88..00aa5ea4` | Resolve mutable name `data/interface/textures/whiteGui.tga` through `00bdf4c0`. If accepted, call renderer virtual `+64h(name,0)` and assign manager `+28h`. Rejection preserves the old slot. An accepted null renderer result overwrites it. No old-reference release appears here. |
| `00aa5ef0..00aa5f1a` | Load `interface/textures/common/transparent.tga` with flags zero, then decrement the old `+2Ch` texture's atomic count at texture `+4h`. Call virtual `+0h` at zero, clear the old slot, assign the new result. No pointer-identity shortcut and no extra retain. |
| `00aa5f7c..00aa5fb1` | Load/find `_Mouse` through `00aa5840(name,1,0)`, store at `+4Ch`, then dispatch page virtual `+34h(true)`. |
| `00aa5ff5..00aa60a4` | Find `MousePtrFE_Icon`; store the borrowed result at `+54h` and `+50h`. Find `MousePtrGUI_Icon` and store at `+58h`. Dispatch GUI false first (`00aa6099`), then FE false (`00aa60a4`). |
| `00aa60e8..00aa611b` | Load/find `_Highlight` with `(name,1,0)` and dispatch true. Its pointer stays in EDI; there is no dedicated manager slot. The page registry retains it. |
| `00aa615e..00aa62fd` | Find and store `hl_FrameBox` at `+74h`, `hlCircle_FrameBox` at `+78h`, `safezone_43_FrameBox` at `+7Ch`, `safezone_169_FrameBox` at `+80h`. Dispatch false immediately after each lookup/store. |
| `00aa6305` | Write zero to byte `+84h` after all visibility dispatches. |

Pages load with `screen_flag=1`, `add_reference=false`. Reusing a registered page
does not increment its reference count on this path. The registry owns the page
objects and their children. Manager pointers are borrowed identities; neither
the double FE store nor highlight child storage acquires another reference.

`00aa7e00` is a search, not a child factory. Its original ABI is
`__thiscall(parent, NativeString* name, int unused)`, returning the child in EAX;
both exits execute `RET 8` (last instruction starts `00aa7eae`, ends `00aa7eb0`).
The body scans direct children, skips a null scene node at child `+4Ch`, compares
the node name length then `__stricmp`, and returns the first match or null. It
never reads its second stack argument. The existing layout implementation is
reused; this packet adds no second implementation. Its ASCII case folding is the
established model for the installed ASCII widget names; CRT locale behavior for
non-ASCII names is not claimed.

Six blocks in the resource pseudocode are marked unreachable and its stack
recovery drops arguments. The complete assembly establishes every literal,
renderer flags zero, both page argument triples, the visibility order and final
flag write. Fresh exports were saved after verifying the live target. The native
function ranges were compared against the installed executable, independently
of the build tests.

## C++ integration contracts

Construct `GuiResourceOwner(registry, layout, callbacks, initial_state)` and keep
the registry, layout host and callback backing services alive for the owner's
lifetime. `state()` exposes the typed retained slots;
`initialization_completed()` is C++ bookkeeping set only after native `+84h=0`.
An optional initial state permits the observed replacement path to operate on an
existing transparent reference. The new API initializes otherwise-unwritten
fields deterministically; those initial values do not reconstruct the native
constructor.

All five `GuiResourceCallbacks` are required before initialization:

- `resolve_existing_name(std::string&) -> bool` supplies actual mutable VFS name
  resolution, used only for `whiteGui`.
- `load_texture(std::string_view, uint32_t) -> GuiResourceTexture` supplies the
  renderer call and its returned reference. Both texture paths pass zero flags.
- `decrement_texture_reference(GuiResourceTexture) -> int32_t` supplies the actual
  atomic reference decrement. `destroy_texture(GuiResourceTexture)` supplies
  virtual `+0h` when the returned count is zero.
- `set_visibility(GuiLayoutWidget&, bool)` dispatches the actual widget behavior
  at virtual `+34h`. Setting the layout's `visible` field alone is insufficient.

`GuiLayoutHost` supplies page/model resolution and real node/widget binding
through the independently reconstructed loader. The page Lua owner is a separate
module. For scene integration, bind each `GuiLayoutWidget::transform` and
`node_id` to its actual `GuiWidgetSceneFlags` and node identity. The existing
`set_widget_visible(widget.transform, visible, GuiWidgetSceneHost&)` supplies the
base visibility walk, but the host must preserve derived-class dispatch and
provide real node visibility, parenting, and effective-visibility callbacks.
No dummy nodes, synthetic textures, default visibility no-ops or application
startup success are installed by this packet.

Missing callbacks throw `invalid_argument` before any work. A missing page/root
or required child throws `runtime_error` at the corresponding native dereference
boundary, with prior texture loads/stores/visibility calls preserved and no final
ready-byte write. This makes the failed precondition explicit instead of
reproducing an access violation. There is no rollback or null-skipping success.

The bounded owner retains the manager state and performs the observed
transparent replacement release. Texture teardown beyond this routine is still
the caller's responsibility: it does not invent a native manager destructor or
release the raw `whiteGui` slot, pages or borrowed children at C++ destruction.
There is no binary-compatible renderer or manager layout claim.

The older `run_gui_startup` interface remains source compatible. Its Child
operation now explicitly means direct-child lookup, null transparent results
are stored, and missing required GUI objects fail at visibility dispatch. That
legacy callback cannot distinguish VFS rejection from an accepted null
`whiteGui` result. The new owner separates resolution and texture loading and
fully models that distinction.

## Validation

MSVC Win32 Release `scripts/build.ps1` passed with `/W4 /WX`; both existing CTests
(`reconstructed_math`, `native_math_differential`) passed after eight seed ranges
matched the installed executable. No permanent tests were added.

An ignored `local/gui_resources_probe.cpp` fixture parsed the installed
`interface/_mouse.lua` and `interface/_highlight.lua` into the existing layout
model and registered their pages. It used explicit fixture node identities and
callback recorders. It verified the complete visibility/reference transcript,
same-texture acquire-before-release, rejected resolution preserving `whiteGui`,
accepted null overwriting it, cursor aliases, unchanged page refcounts,
case-insensitive direct lookup, no descendant lookup, and missing node-bearing
child preventing final completion. The probe was linked with `/MANIFEST:EMBED`.
It validates retained-state behavior and installed page-table compatibility;
real Lua evaluation, native renderer execution, scene visibility and gameplay
were not validated by this fixture.
