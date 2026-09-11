# Canonical Text construction and derived lifetime

Addresses: `00AB9650`, `00AB8250`, `00AB8EE0`, `00AB73B0`, `00AB83D0`, `00AA8320`.

`GuiTextLifetime` owns one existing `GuiTextWidget`, one live shadow pointer,
one glyph-child vector, one actual cached shader identity and the verified
extension fields absent from the earlier Text projection. Content and style
bindings refer directly to those members. The base widget, layout tree, model,
geometry, material and reference owners remain the existing owners. This is a
new C++ Win32 interface, not a `1F4h` native object or native Text pool slot.
Names are hypotheses, not recovered symbols.

| Routine | Native ABI and bounds | Coverage |
| --- | --- | --- |
| `00AB9650` | ECX Text, no stack arguments, EAX this; `RET` at `00AB98EE` | Partial: same-owner derived construction and actual `00AB8530`; caller must already perform base `00AA9390(3)` at `00AB9671`. Native vtable/SEH and native container headers are unrepresented. |
| `00AB8250` | ECX Text, no stack arguments; `RET` at `00AB83CF` | Partial: actual shader/shadow/child effects and typed container destruction through the phase ending `00AB83AE`; base/SEH tail `00AB83AF..00AB83CF` remains caller-owned. Native five pooled string returns and CRT vector allocation are projected by C++ allocations, not allocator-equivalent. |
| `00AB8EE0` | ECX Text, flags DWORD stack, EAX original this; final `RET4` at `00AB8EFD`, end `00AB8EFF` | Analyzed only; scalar deleting destructor and native Text pool return are not implemented. |
| `00AB73B0` | ECX Text; final `RET` at `00AB73CD` | Complete for the supplied actual model lifetime domain. |
| raw `00AB83D0` | ECX unused, descriptor DWORD stack; final `RET4` at `00AB83F5`, length 3, inclusive end `00AB83F7`, body length 40 | Complete ordered comparison against three live descriptor words; no saved Ghidra function when inspected. |
| `00AA8320` | ECX widget; `RET` at `00AA8396` | Partial extension `00AA8362..00AA837F`: Text secondary release between existing child and primary-node phases. Shared owner hook implemented by the coordinated buffers packet. |

The Text vtable bytes at `00D5C6C8` resolve `+04` to `00AB8EE0`, `+0C` to
raw `00AB83D0`, and `+20` to **base** `00AA8320`. `00AB73B0` is therefore
the Text secondary-node helper, not a separate virtual20 override. The
child virtual20 call is `00AA8352`; `00AA8343` is a bounds-check branch.
After the child loop, `00AA8368` loads the current descriptor through
`00AB6A30`, `00AA8372` calls current type0C, and `00AA837B` conditionally
calls `00AB73B0`. Primary release is later at `00AA8387`. The new owner
hook is invoked at precisely that phase; Text must supply the live lineage
table, whose three words are zero in the uninitialized PE image. No zero or
fixed-descriptor default is supplied.

After derived teardown, the same C++ hook stops dispatching Text: native
`00AA9730` replaces the vtable with the base table before calling `00AA8320`.
Keeping a Text companion alive through base retirement must not reintroduce
its shadow-release branch, including when a child-deletion callback has
rebound the shadow slot after the earlier release.

Construction takes required live constants for `D5C5C0`, `E12FD8..E12FE4`,
`CE3E18`, `D7A24C` and `CE3800`. Current image values are shadow offset
`0.05f`, shadow `(0,0,0,0.75f)`, normal RGB `0.7f`, one and disabled alpha
`0.5f`; the code reads the supplied live slots instead of embedding these
values. After the established field stores it calls actual
`ensure_gui_text_draw_sections_00ab8530`. That code creates the shadow from
the same widget runtime's model pool and registers the same model reference,
geometry, section and material. The constructor may run before the primary
node is bound, as at `00AB9D4F`; that caller subsequently binds the node and
calls `00AB8530` again at `00AB9D93`. The other constructor caller is the
null-copy-source arm of `00AA1380` at `00AA13DD`; copy construction remains
`00ABB2C0` and is outside this packet.

The owned data is:

| Native fields | Canonical C++ storage / ownership |
| --- | --- |
| `+EC/+F4/+100..+174`, shader/font names `+1C0/+1C8`, font scale/default shadow | The one `GuiTextWidget`; base `size/color` remain its pre-existing reader projections of the same layout. |
| `+178/+17C/+180/+184/+18C/+190` | Address-named fields; constructor values `0,2,null,null,0,0`. Pointer fields are borrowed; destructor does not release them. |
| `+188` | The one `NativeNodeBinding*`; creator reference belongs to this Text. |
| `+198/+19C/+1A0` | The one glyph vector, distinct from the general GUI child list. |
| `+1A4/+1A8`, `+1AC/+1B0`, `+1B4`, `+1F0` | Owned optional UTF16 string; borrowed pointer words; bytes initialized `0` and `1`. |
| `+1DC/+1E0` | Uninitialized float pair, established by `00ABAA03/00ABAA15` after shader selection, consumed by the continuation worker. No constructor defaults invented. |
| `+1EC` | Actual raw shader identity owning the renderer's returned reference, released through the existing raw `+04` owner operation. Legacy `has_cached_shader` is only a projection; future producers must use this actual slot and maintain the flag. |

`+194`, `+1B8/+1BC`, `+1D4` and `+1DC..+1E8` receive no native constructor
store. There is no new representation for unwritten words that no current
consumer needs. The existing `GuiTextWidget.alpha_texture_scale` has a
semantic in-class default; it is explicitly **not** a native constructor
value and must not be consumed before font resolution. The same rule applies
to the uninitialized material pair before its producer runs.

Derived destruction captures and releases cached shader `+1EC`, clears the
live slot after the zero-reference callback, then captures/releases the
shadow and clears `+188` after its callback. Shadow resolution goes through
actual storage to `NativeModelReference`; wrapper types are never cast into
one another. The same registered model and service domains are validated.
The child-clear function then operates on the same glyph vector and actual
detach/deletion contracts. Typed allocations are released immediately in
native order: font name, shader name, optional UTF16, glyph vector, source,
displayed text. Native destructor string headers stay stale after pool return;
C++ containers become empty. That representation distinction is not hidden.

The embedding Text implementation must call
`destroy_derived_00ab8250_fragment` from `before_scene_release`, before the
base/widget companion is retired. The C++ destructor runs this cleanup if
still live; all supplied owners/services must therefore still exist. Its
repeat-after-explicit-cleanup guard is C++ lifetime bookkeeping. Reentrant
destruction, exceptions from deletion callbacks and native SEH unwind parity
are outside the supported normal path. Failed construction performs a C++
shadow cleanup before propagating the exception; no native unwind claim is made.

`00AB8EE0` calls `00AB8250` at `00AB8EE3`, tests the low flag bit, and only
then calls `00AB75A0` at `00AB8EF5` with ECX `F8BDF0` and the original object.
The callee consumes one stack DWORD (`RET4`) and reads allocator metadata at
object `+1F4`, beyond the `1F4h` instance. A C++ `GuiTextLifetime` cannot be
passed to it. No fake scalar delete or pool-return callback was added.

Factory registration remains disabled. Actual Text property/loaded/visibility,
nonempty content, shader selection and recursive Text deletion must be composed
before it can be enabled. The current executable cannot reach this companion.
Validation is strict MSVC Win32 `/W4 /WX /O2 /fp:strict` source compilation with
the workers' current owner/content headers; no new tests, runtime execution,
native differential fixture or game validation is claimed. Parent performs the
normal combined build after integration. Raw Ghidra analysis was read-only.
