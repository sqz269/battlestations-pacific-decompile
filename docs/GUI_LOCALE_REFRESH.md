# GUI locale refresh and retained page owner

Packet `orch2_gui_locale_refresh`; target `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Each live analysis/export command used the verified
`bsp.py ghidra` client. All Ghidra access in this packet was read-only. Descriptive
names are hypotheses, not recovered symbols.

## Established behavior

`00aa4650` is an application operation, despite its old
`STL_inst_00aa4650` / `stl_instantiation` classification. Its sole indexed caller
is the changed-language tail of `00aa09d0`, after `004c12b0` gets the persistent
GUI manager. It walks the page vector at manager `+18h/+1Ch` in its existing order,
calling `00aa40a0` for every page. The vector belongs to the existing
`GuiPageRegistry` projection, including priority registration through `00aa52a0`.
No separate locale graph or page snapshot is needed.

`00aa40a0` tests the widget's virtual `+5Ch` type result against **3** (Text).
For a text widget it copies the cached narrow source at `+F4h/+F8h`, calls
`00abbe50(widget, ".", true)`, then calls
`00abaed0(widget, saved_source, true)`. It releases the temporary source and
recurses over **all** children in the intrusive list at widget `+68h`, using each
link's widget pointer at `+8h`. This is a pre-order walk; visibility, enabled
state, scene-node presence and page priority are not fresh filters here.

The literal at `00ce3a70` is **a period**, verified as `2e 00 00 00`, not an empty
string. Both calls use localization. For ordinary keys this bypasses the cached
source equality gate and resolves the key again using the new locale. Empty
sources also take the period/empty sequence. A source already equal to `"."`
takes the setter's equality early-outs twice. The intermediate setter's layout
and color effects must not be collapsed into one forced final rebuild.

`00abbe50` is the C-string overload: a null C-string means empty content; matching
content returns early, otherwise a temporary native string is passed to existing
`BSP_GuiText_SetLocalisedSource` (`00abaed0`). The new wrapper reuses that existing
C++ implementation and its required `GuiTextHost`; no font/layout/render call is
replaced with a default implementation.

The `00bf6713` calls in the walks are guarded invalid-iterator paths. They are
not a successful-path CRT free. In `00aa4650`, one is behind `CMP ESI,ESI`, while
the others check vector bounds. Actual temporary string release in `00aa40a0`
uses `00419cc0` and `00bd1510`.

## Constructor and lifetime evidence

`004c12b0` double-checks global `00f8bc5c`, uses the lifetime manager's optional
critical section, allocates `88h` bytes through `00bf681b`, runs `00aa5d70`,
publishes the pointer and registers it through `00bd0c30`. This is not reconstructed
as a native singleton by this packet. A retained application-owned
`GuiLocaleRefreshManager` supplies the locale-relevant persistent state. The
integrator can construct it lazily at the original getter boundary.

`00aa5d70` calls only two allocator helpers. There are no engine, UI, font or
renderer calls in this constructor:

| Address | Allocated object | Established writes |
| --- | --- | --- |
| `00aa2920` | `18h` byte tree node | `+0/+4/+8 = 0`, byte `+14h = 1`, byte `+15h = 0`; returns the pointer in EAX despite the old void prototype |
| `00aa2820` | `14h` byte list sentinel | `+0/+4` self-links; remaining payload untouched; returns the pointer in EAX despite the old void prototype |

The constructor installs the tree node at `+0Ch`, makes its first three links
self-referential and sets its byte `+15h` to 1. It installs the list sentinel at
`+38h`. Exact tree key/value and list payload types remain unestablished; these
containers are not manufactured as empty engine services in the locale owner.

| Manager offsets | Constructor result |
| --- | --- |
| `+00h` | vtable `00d5bfcc` |
| `+0Ch`, `+10h` | tree sentinel, count 0 |
| `+18h/+1Ch/+20h` | empty page-vector begin/end/capacity |
| `+24h/+28h/+2Ch/+30h` | zero |
| `+38h`, `+3Ch` | list sentinel, count 0 |
| `+40h`, byte `+48h`, `+6Ch` | zero |
| `+5Ch/+60h` | float 0.5; live `00ce3800` bytes `00 00 00 3f` |
| Everything else | not initialized by this routine |

In particular `+4Ch/+50h/+54h/+58h` and `+74h` onward are not populated by this
constructor; the subsequent resource loader `00aa5e20` owns their initialization.
No promise of resource-ready GUI construction follows from an empty page vector.
The earlier statement in `APP_INIT_FONTS_GUI.md` that the font phase first creates
the singleton is conditional: a changed-language call with registered table names
can reach `004c12b0` earlier. The fresh native localization manager's empty name
vector bypasses that GUI tail.

## C++ ownership and required bindings

`GuiLocaleRefreshManager` derives from the existing `LocaleGuiRefreshHost` and owns
one existing `GuiPageRegistry`. `page_registry()` exposes it to
`load_gui_page_00aa5840` / `register_00aa52a0`; later registrations are seen by the
next refresh. The page and child structures are the existing `GuiLayoutPage` and
`GuiLayoutWidget` types.

The existing text state is a separate projection, `GuiTextWidget`. `bind_text`
ties a Text node to that live state and a real `GuiTextHost`. Both are borrowed
and must remain alive until unbound. Host calls still require actual localization,
glyph layout and color application. A text node with no binding throws an explicit
`logic_error`; it is never silently skipped. Null page roots/children are rejected
as malformed projected state. Graph/binding structural mutation during the walk
is outside the valid native-iterator contract.

This supplies a real empty traversal immediately after construction, and a real
populated traversal when the existing loader and text services supply the graph.
It does not load pages, evaluate GUI scripts, instantiate scene nodes, create
fonts or textures, implement the whole `88h` native object, reproduce its
allocator/SEH behavior, or register a native singleton lifetime. These remain
explicit integration requirements rather than successful placeholder callbacks.

## ABI and status

| Address | Original ABI / inclusive body | Packet status |
| --- | --- | --- |
| `00aa4650` | ECX manager, no stack args, `RET`; `00aa4650..00aa46a7` | reconstructed as `refresh_locale_00aa4650` |
| `00aa40a0` | ECX manager, stack widget pointer, `RET 4`; `00aa40a0..00aa41aa` | reconstructed as `refresh_subtree_locale_00aa40a0` |
| `00abbe50` | ECX text widget, stack C-string and localization flag, `RET 8`; `00abbe50..00abbf20` | reconstructed by delegating to existing text setter |
| `00aa5d70` | ECX allocation, returns EAX=this, `RET`; `00aa5d70..00aa5e18` | full constructor analyzed; only persistent empty page-vector state reconstructed |
| `004c12b0` | cdecl, no arguments, EAX singleton, `RET`; `004c12b0..004c136f` | analyzed lifetime boundary |
| `00aa2920` / `00aa2820` | no consumed arguments, EAX allocated node, `RET` | analyzed STL allocation helpers; exact template names unresolved |

All relevant routine bodies include their actual returns; no no-return repair or
unowned code-gap recovery was required. `verify-seeds` matched all eight original
byte ranges; `scripts/build.ps1` then passed Win32 compilation and both existing
CTest checks, including `native_math_differential`. One ignored GUI fixture passed
the retained registry, priority/pre-order traversal, period/empty/equality gates,
intermediate/final color calls and missing-binding failure. The native differential
check covers the existing math seeds, not these GUI routines. Fixture paths and
validation results are recorded in `reports/gui_locale_refresh.json`. These are
new C++ interfaces, not ABI-compatible binary replacements. No game or visual
validation is claimed.
