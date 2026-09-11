# Canonical Text property continuation

`gui_text_properties.cpp` reconstructs `00ABB630` **after its existing base
reader and child traversal return**, and supplies the complete `00AB72D0`
shadow preset leaf. It uses one `GuiTextLifetime`, the actual resource-name
setter, existing GUI Lua typed-value/default conversions, and the actual
runtime source/content submission. It adds no semantic `GuiTextHost`, owner,
material cache, lookup callback or factory registration.

| Entry | Native body / ABI | New coverage |
| --- | --- | --- |
| `00ABB630` | end `00ABBE44`; ECX Text, visitor stack; RET4 atBE42/length3 | Derived `00ABB658..00ABBE44`, conditional on completing nested content. Existing AAA710 prefix remains the caller's prerequisite. |
| `00AB72D0` | end `00AB73A8`; ECX Text, preset DWORD stack; RET4 at73A6/length3 | Complete supported same-lifetime field operation. |

Both full bodies were read, with no unread ranges. Text table `00D5C6C8+18`
at `00D5C6E0` contains `30 B6 AB 00`; that is ABB630's sole xref. AB72D0 has
one caller, `00ABBC0A` inside the verified ABB630 body. All names are descriptive
hypotheses and these are new C++ interfaces, not binary ABI replacements.
Repository wrappers verified `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe` for each batch. Ghidra was read-only.

## Base boundary and field order

At ABB64B ESI captures the visitor, and EDI captures Text at ABB651. The call
at ABB653 executes `00AAA710`, including the parent's base fields and its child
enumeration, creation, current74, current18, reader leave and current78 sequence.
The current loader already performs that sequence before
`on_widget_properties_bound`. Calling base again from this new continuation
would duplicate it. Loaded78 and successful factory completion must wait when
the derived result is pending.

The continuation preserves this order and these **native defaults**, which
are not uniformly the preexisting field values:

| Field | Operation/default |
| --- | --- |
| Font | Visitor10, empty temporary initially; **always** call actual AB8C30, even if missing. |
| FontScale | Float, live D7A24C. |
| Multiline | Lua Boolean, true. Numeric0 is truthy under Lua's rule. |
| DistanceBetweenLines | Float, zero. |
| DefaultText | String temporary, empty. |
| ShaderName | Direct string field store, empty; **no AB8E70 invalidation call** occurs here. |
| Align | Empty default; CRT Left, then native helper Right, Center, Justified; values0,2,1,3, fallback0. |
| VerticalAlign | Empty default; CRT Top, then helper Bottom, Center; values0,2,1, fallback0. |
| MISColors | Only if nonnil; enter table, Normal/Focus/Selected/Disabled, set+118 true, leave. Missing scope leaves existing state colors/flag unchanged. |
| DefaultShadow | Integer, -1; values other than -1 call the preset leaf, otherwise explicit shadow fields below. |
| Shadowed | Only when preset -1; store default_shadow=-1 first, then Lua Boolean defaultfalse. |
| ShadowColor | Only when live shadowed byte is nonzero; same borrowed E12FD8 quartet as default. |
| ShadowPos | Empty default; Front ->1, everything else0. |
| ShadowOffset | Float, live D5C5C0. |

The existing GUI converters preserve Lua numeric/string conversion, integer
narrowing through the supplied CRT mode, truthiness and missing numeric color
lanes converting to zero. A nonnil MISColors with no table shape or another
unsupported aggregate fails at that field; it is not silently treated as an
absent key or a replacement default. This packet supports ordinary evaluated
tables. Live Lua metamethod/ref-stack callbacks and visitor allocation ABI are
outside that projection.

Reader table CE44FC was checked: +4 BD8E20, +8 BD7A20, +0C BD68D0, +10 BD6830,
+14 BD5EB0. The callee bodies establish has-key as nonnil, enter/leave scope,
and value-versus-default selection. Actual cleanup establishes four stack
DWORDs/RET10 for visitor10 and six DWORDs/RET18 for visitor0C. Enum comparisons
reuse actual current CRT and the existing full `00425850` header/C-string
comparator; the older ASCII-only enum helper is not substituted.

## Shared color default and presets

Each MISColors row independently rechecks bit0 of the **same** F8BE48 word.
When clear, native first reads D7A24C, sets bit0 (preserving other bits), then
writes all four F8BE38 channels before the field read. The new service borrows
that one mask/quartet, rather than constructing a white constant per Text.
The initial mapped bytes were zero; the in-routine producer establishes the
initialized state. Default field reads borrow the actual quartet, and
ShadowColor defaults borrow E12FD8 directly, without an early scalar snapshot.

Preset AB72D0 first compares current+1D0. Equal means no stores, even if other
shadow fields were changed since the previous application. On change it stores
the preset. -1 only clears the shadow byte. Every other integer, including
negative values other than -1, enables shadow, installs E12FD8, position0 and
D5C5C0. Preset1 additionally uses one captured D7A24C for RGB and CEE07C for
alpha. Native bytes confirm D5C5C0=0.05 and CEE07C=0.75; the implementation reads
the supplied live storage, not those literals. This leaf never invokes style,
scene parenting, resource allocation or content rebuilding.

## Pending submission and cleanup

The DefaultText temporary's **length**, not key presence, gates `00ABBD99`.
Only a nonempty captured value calls actual ABAED0 with localize=true.
Unchanged source returns normally; changed source may complete or retain a
pending actual builder/child frame. The new outer continuation retains Font,
DefaultText, Align and VerticalAlign until the nested submission completes.
Resume is permitted only after the actual missing child tail, under the nested
submit interface's rules; it never treats pending geometry as completed.

ABAED0 owns its final current50 after its converted temporary cleanup. ABB630
does **not** add another current50. Only afterward does its tail destroy
VerticalAlign, Align, DefaultText and Font, in that order, before returning.
The ShadowPos temporary is destroyed before submission. Typed string cleanup
models this lifetime/order; original NativeString pool callbacks, SEH, allocation
failure and raw visitor ABI are not reproduced. Dropping a pending frame is
not completion and must not be used to bypass a missing builder dependency.

Strict MSVC Win32 C++17 compilation passed with `/W4 /WX /EHsc /permissive-`
using new-header overlays and the current root submission header. The report
records exact-call verification. No tests were added. No executable or factory
reaches this module yet; no game/render or original ABI-equivalence is claimed.
The integrator owns source registration and combined build checks.
