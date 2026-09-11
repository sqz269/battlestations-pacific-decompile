# GUI page execution and retained layout data

Addresses: 00AC6600, 00AA5840, 00AAA710. This packet implements the private
Lua lifecycle behind page construction using the existing VFS, Lua owner, and
Lua 5.1.1 script runtime. Names are provisional and interfaces are new C++, not
the native object layout or register ABI. The real application GUI remains
blocked on the explicit scene and per-widget integration contracts below.

## Assembly evidence

The existing `bsp` project (`C:/Users/sqz269/bsp.gpr`), program
`/battlestationspacific.exe`, was verified through the configured CLI before the
read-only batch. No Ghidra functions, names, prototypes, comments, or bytes were
written. Exported assembly, rather than corrupted constructor pseudocode, is
the source for this reconstruction.

`00AC6600` takes ECX=the screen, then `(NativeString* name, model*, byte flag)`
on the stack; its inclusive final instruction is `00AC6876: RET 0Ch`.
`00AC66AB` reads the model, `00AC66A4` reads the flag, `00AC66EE` writes the flag
to screen+120h, and `00AC670F` binds the model through `00AA6720`. The earlier
name-ledger description of a second-argument share flag and unused third argument
is superseded by these stack reads and caller `00AA593B..00AA5944`.

The recovered sequence is:

1. `00AC6718..00AC672B`: construct the frame Lua owner and open mask65h,
   base/table/string/math, using existing `PcStorageLuaOwner` bootstrap.
2. `00AC675A`: run `interface/_Common.lua` and its existing ordered VFS overrides.
3. `00AC67A0`: run `interface/<page>.lua` and its existing ordered VFS overrides.
   Both calls pass obfuscation=false to `00B69D40`.
4. `00AC67DA..00AC6804`: obtain globals and descend into key `{0,"GuiScreen"}`.
   The literal is at00D5CB24, not00D5CB20.
5. `00AC6812`: call the screen's property reader at virtual+18h, with Lua alive.
6. `00AC681F..00AC6825`: ascend, then call `00B6D890(node,0)`. Existing
   `propagate_native_node_root_00b6d890` shows that this clears the node subtree's
   root-list binding. It does not clear the node's parent.
7. `00AC682A..00AC6857`: destroy the reader before closing the private Lua owner.

`00AA5840` is ECX=manager, `(name*, int screen_flag, byte add_reference)`,
final instruction00AA59A9. Its low screen-flag byte reaches the constructor's
third argument. The registry's already-present path returns without opening Lua.
Its root model is either the fetched resource instance's+0Ch node or the plain
188h object from00B8F450/00B8F5E0. It is not the child's184h00B75030 node.

`00AAA710` is ECX=widget, `(visitor*)`, final instruction00AAAECC. The child is
allocated through00AA6560, node assigned at00AAADC4, appended to the list before
parenting at00AAAE24, then virtual+74 at00AAAE31, reader descend, child virtual+18
at00AAAE50, reader ascend, and virtual+78 at00AAAE63. A null node does not skip
the child or the parenting call. The projection now preserves that call order.

## Interfaces and ownership

`VfsGuiPageScripts::begin_00ac6600` returns a `GuiPageLuaEvaluation`, which owns
the private state and a `GuiLua51Host`, runs both files, and retains a data
snapshot of GuiScreen. `evaluate_00ac6600` is a data-only convenience that closes
Lua immediately after taking that snapshot.

`GuiPageScriptLayoutHost` implements `GuiLayoutHost`'s evaluator and final page
callback. It retains the evaluation through all widget callbacks and the
required root-list clear, then closes it. Subclasses may obtain the live reader
with `live_reader_host()`. The common free `load_gui_page_00aa5840` owns an unwind
guard and calls the required noexcept `on_page_load_failed` after any exception
on a cache miss. The Lua adapter retires its active state there, including when
a property hook throws, so direct resource-owner calls and the `load_page`
convenience wrapper both clean up and allow a subsequent retry. Startup must not
consume the partially constructed scene after a failed load.

The page stores `screen_flag` as a byte. `script_table` owns the snapshot and
keeps every widget's `source` pointer valid after Lua closes. The snapshot stores
strings, numbers, booleans, tables, positive integer array entries, and lua_next
string-key order. Shared acyclic subtables keep shared identity. Source order is
still used by the optional old static parser; Lua does not promise stable
iteration order across interpreter states.

Lua metatables, cycles, non-data values, non-string/non-positive-integer keys,
embedded-NUL names, depth over128, or excessive snapshot size produce an explicit
host error. This deliberately does not claim parity for functions or dynamic
metatable lookup. The live state remains available to supported widget hooks,
but mutations made by those hooks are not resnapshotted. The shipped successful
pages did not require these unsupported forms.

Base numeric, boolean and vector binding now reuses the recovered Lua value
conversion switch at00BD63B0: zero is true, numeric strings coerce, and missing
aggregate lanes become zero. Priority uses the recovered integer conversion.
Derived properties are still outside the base projection.

## Required integration

No scene IDs or no-op widget hooks are supplied. A subclass must implement:

- Model lookup/instantiation and real root-node selection/binding
  (00B80D70, resource virtual+8,00B8F5E0,00AA6720).
- Real child node creation/parenting (00B74EB0/00B75030,00B6E680) and widescreen state.
- Actual widget virtual+74/+78 dispatch and per-class property projection. The
  generic `on_widget_properties_bound` boundary runs after the base fields and
  before children; this is not proof of every derived reader's native ordering.
- Root-list clearing through00B6D890 while the interpreter is still alive.

Existing `gui_widget_scene.cpp` has the binding/visibility contracts and base
virtual+78 behavior `deactivate_and_refresh_bounds` (00AA7170).
`native_node_destruction.cpp` supplies the root-list propagation. `gui_text.cpp`
and `gui_icon.cpp` supply class-specific property reconstructions. They need a
shared real widget/node owner and field mapping before application activation.
The base layout's type tag and transform are not a reconstructed native widget
instance or an implementation of the18-class00AA6560 factory.

## Validation

MSVC Win32 Release build and both existing CTests passed after seed verification.
An ignored manifested Win32 fixture used the real physical VFS mount and all97
installed interface Lua files (one common library and96 pages). Of96 page
executions,95 produced retained GuiScreen tables, including helper-generated
menus, arithmetic and concatenation that the static parser could not evaluate.
The sole failure was the shipped malformed `_debugtexts.lua`: its comma after
the Pos assignment and statement-level `["Template_Text"]` cannot compile.
The runtime ignores load status as native00B66CA0 does and reports
`attempt to call a string value` from calling the compile-error string.

The focused ignored fixture also verified ordered VFS override execution,
private globals across two page loads, shared subtable identity, Lua alive
through property hooks/root clear then closed, retained widget sources, bool
and sparse-vector conversion, priority/flag preservation, cache/refcount return,
runtime errors, metatable rejection, and a missing page's absent table. A failing
property hook through the free loader closes Lua, leaves the registry unchanged,
and permits a successful retry through that same free function.
Its scene nodes and hooks are fixture objects, not game/render validation.

Native Lua script errors panic and terminate the process. The existing runtime
uses a protected host boundary to throw C++ errors and release local resources;
this packet preserves that explicit adaptation and does not convert failures
into empty successful pages. Missing/empty files retain the native silent skip.

No new permanent tests were added. The game installation and application hosts
were unchanged; rendering, actual native widget ownership, ABI compatibility,
and in-game behavior remain unvalidated.
