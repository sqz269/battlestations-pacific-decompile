# Canonical Text runtime composition

`gui_text_runtime_factory` constructs one `GuiTextLifetime` inside one concrete
`GuiWidgetTypeImplementation` retained by the existing `GuiWidgetOwnerRuntime`.
The layout, logical child lists, transform, primary model and shadow slot are
the existing canonical objects. Its pool map holds only raw allocation handles;
it is not a second widget tree, Text state, or ownership hierarchy. Names remain
descriptive hypotheses. Ghidra was read-only for this packet.

Configure the existing environment's `make_type` Text branch to call
`GuiTextRuntimeFactory::make_type(owner)`. The host constructs the owner runtime
first, builds its actual shared services, creates the Text factory, and publishes
that factory to the dispatch closure before creating Text. The factory validates
the common widget, buffer, model, material, actual-resource, font, mapping,
parameter and relevant constant identities. Initialize and destroy the supplied
`NativeGuiTextPool` explicitly; there is no new startup/atexit surrogate.

| Native profile slot | Target | Concrete composition |
| --- | --- | --- |
| 18 | ABB630 | Existing after-base reader; retains properties/submission/content frames. |
| 20 | AA8320 | Existing owner release plus Text lineage/shadow hook. |
| 34 | AA8530 | Existing inherited owner visibility propagation. |
| 38 / 3C | A9E0D0 / A9E100 | Current primary-node visibility and proven RET4 notification. |
| 4C | AB6AD0 | New alpha-only base and shadow material writes. |
| 50 | AB6B50 | Existing actual Text color implementation. |
| 58 | ABBF30 | Existing actual size/recompose/content rebuild. |
| 60 | AB87D0 | Existing actual-name lookup and both-arrow current34 forwarding. |
| 64 | AB6D70 | New complete x87 bounds/height/byte1F0 composition. |
| 70 | AB7A40 | Existing actual main/shadow clip registration, then real child current70. |
| 74 / 78 | AB7700 / AB6AA0 | Existing mesh constructor and loaded thunk/current60 path. |
| 80 | AB7200 | Existing actual state-color implementation and its supported domain. |

The generic owner interface currently exposes the required factory hooks plus
current70. Additional typed Text entries expose 4C/50/58/64/80 and the source,
UTF16, ellipsis and rebuild calls. They are usable concrete C++ methods, not a
claim that every native slot in D5C6C8 is installed in a binary-compatible table.

## Construction and pool lifetime

`AA1380` calls `AB79E0`, tests its incoming source pointer, and selects default
`AB9650` only for null source. Nonnull copy construction `ABB2C0` is outside this
factory. Native `AA1380` ends at `AA1403 RET`; source is in ECX, EAX returns the
Text/raw-null result. Its earlier default/copy returns at `AA13F1`/`AA13CE` are
both single-byte RETs. `AB9650` is ECX Text, no stack arguments, returns the same
Text in EAX; its last instruction is `AB98EE RET`, length 1.

Ordinary `construct_child_00aa6560` creates the base owner and this Text
implementation while primary+4C is null. The lifetime initializes the derived
defaults, publishes its association before constructor callbacks, and runs the
existing actual `AB8530`. The ordinary owner route then creates/binds the named
primary model. Standalone `AA6640` runs current74 after binding; the loader runs
74 after its separate parenting step. Pool allocation never implicitly runs74
or78. A raw 1F4-byte slot is never cast to `GuiLayoutWidget`, `GuiWidgetOwner`, or
`GuiTextLifetime`.

`construct_unbound_glyph_child()` is specifically the `AB9D38/AB9D4F`
prerequisite. It returns the sole `unique_ptr<GuiLayoutWidget>` with its same
runtime's fully constructed Text companion and null primary node. It does not
append the parent's glyph vector, clone/bind a model, run74, or complete the
AB98F0 child tail. That caller must append the exact allocation at the native
phase, clone the current template model with its actual source-name/pool domain,
bind it, and invoke `AB8530` again before continuing the child content/tail.

The factory associates a raw slot before constructing `GuiTextLifetime` and
returns it through `AB76F0` if typed construction fails. Native allocation occurs
before base construction; this new C++ factory is called after the existing
base owner's field initialization. It therefore claims the successful supported
allocation transport, not native allocation-callback ordering, raw object layout,
original string/vector allocations, null-allocation behavior, or SEH equivalence.
No raw original executable pointer is called.

Completed scalar deletion with flags1 returns the slot after typed teardown.
Flags0 destroys the typed owner/implementation but retains the allocation record
and raw slot. While the completed wrapper still exists, call
`release_completed_storage(layout)` before explicitly disposing that wrapper.
The factory terminates on destruction with any remaining raw allocation; it
cannot silently lose flags0 storage. The existing `GuiTextChildDeletion` supplies
the actual scalar phases and allocation transport. Its host-only
`before_scalar_deletion4` preflight invokes this implementation's pending guard
before any native scalar phase/flag stores or derived effects; this preflight
is not an extra native virtual call. Ordinary page retirement
returns its allocation after the derived/owner sequence, within the explicit
empty native timed-entry header domain; nonzero +88/+8C/+90 is rejected before
derived teardown. This does not implement the generic AA9730 timed-entry tail.

## Pending work stays pending

Properties18 invokes only the derived `ABB630` continuation after the existing
base reader and its children. A pending result retains the entire properties
frame on the implementation and throws `GuiTextRuntimePending`. All subsequent
loaded78, content mutations, current70, scene release and typed destruction must
respect that pending state. The caller must retain the surrounding page/layout,
actual mapped resources and loader/glyph frame; an exception is not permission
to let a stack-owned page unwind and destroy those resources.

The same rule applies to UTF16/source/ellipsis/size/rebuild submission methods.
`pending_content()` exposes the same borrowed nested builder frame.
`resume_after_glyph_child()` is legal only after its exact native AB98F0 child
tail completed with the saved arguments. It can suspend on another child, and
unsupported alignment/undefined format remain explicit dependency boundaries.
Neither method supplies an invented child or unlocks mapped streams on failure.

Current70 calls `begin_gui_text_clip_refresh_00ab7a40`, retains the exact first
child frame, invokes that child's current `refresh_clip70`, and advances only
after ordinary return. A child exception leaves the parent's frame unchanged;
the operation cannot restart registration or skip that child. After the child's
actual nested current70 completes, `resume_clip_after_child70()` advances once.
The generic default current70 throws unsupported; it is not a base no-op. The
existing void page-loader protocol has no retained outer loader continuation,
so a property path that suspends requires a retaining caller or synchronous
completion of the real child dependency before that loader can complete.

## Alpha and bounds evidence

`AB6AD0` is ECX Text, one float stack argument, RET4 at `AB6B43` (length 3,
inclusive end `AB6B45`). It x87-spills its argument before calling base `AA6980`.
The base writes only color alpha at+5C, then main material diffuse+0C behind the
actual geometry/count gates. Text reloads live shadow+188, follows the same
actual model/mesh/section/material domain, multiplies current shadow alpha+174
by the original incoming alpha, spills to float32 and writes only diffuse+0C.
The C++ legacy transform/Text alpha projections mirror the same canonical field;
RGB fields and material ownership are not replaced by a full-color setter.

`AB6D70` is ECX Text, four float pointers on the stack, RET10h at `AB6ED8`
(length 3, inclusive end `AB6EDA`). The full 111-instruction listing and the
complete height helper were read. The code preserves the division-to-float32,
midpoint-to-float32 and half-width-to-float32 spills, then x87 extended arithmetic
between spills. It preserves pointer alias effects within valid live float
objects and original left/right words for the final byte1F0 restore.

The assembly's vertical conditions are deliberately unusual: `AB6E3C TEST EAX`
still tests **horizontal Align**, so horizontal 0 always selects the top path.
Only `AB6E5B CMP ECX,1` tests VerticalAlign for the center path. The bottom path
again tests horizontal Align2 at `AB6E9F`. Do not replace these with a vertical
enum switch. Byte+1F0==0 restores the original horizontal words after all vertical
writes. Earlier high-level horizontal-only helpers remain separate interfaces.

The entry always explicitly FLDs the native double half before reading either
endpoint, including paths that discard it. A host-only 80-bit spill preserves
that loaded extended value between C++ branches, avoiding an implicit SSE load
or narrowing it to a C++ double. The right branch loads the original right value
before the width division, and the bottom branch explicitly FLDs its current
float endpoint and FSTPs a double before the height helper. Numeric results are
claimed with the verified finite native constants and ordinary masked x87
operation; transient x87 stack layout and unmasked exception timing are outside
the new C++ ABI. The generated MSVC assembly was inspected for these operations.

`AB6BD0` is ECX Text, no stack arguments, ST0 result, final RET at `AB6C27`,
length1. Multiline divides current+178 by double720. Otherwise it divides the
CURRENT font+14 signed lowword by720, or integer zero for null font. Every arm
spills float32 then reloads ST0. The existing actual font association supplies
that sole live metric; font scale/descriptor copies and mutable vertical scale
E12FD4 are not substituted. Native constant bytes read were D7A280 half0.5,
CEC380 divisor960, CEF1B8 divisor720 and D5C5C8 padding
`00 00 00 C0 74 93 88 3F`; all are borrowed live references in this interface.

## Verification and integration

Read-only batches verified configured project `C:/Users/sqz269/bsp.gpr` and
program `/battlestationspacific.exe` through repository wrappers. Full relevant
table bytes D5C6C8..D5C74B, factory/default constructor listings, alpha/base-alpha,
bounds and normalized-height listings were inspected. No saved names/comments
were changed. New descriptions remain provisional hypotheses with call rows in
`reports/gui_text_runtime_factory.json`.

Strict MSVC Win32 compilation passed with `/std:c++17 /W4 /WX /EHsc
/permissive- /O2 /MD /fp:strict`. The only compile overlay was the integrator's
then-current `gui_widget_owner.hpp`, SHA256
`F6DA5E513D44D7A639155B619A3CE628E5EF9873C1EA6DCBDEC0F0226F87DD98`, supplying
the agreed owner and scalar-preflight hooks. `scripts/build.ps1` passed the
existing worker checkout build and its `reconstructed_math` test (1/1); the new
source is separately strict-compiled because CMake registration is integrator
owned. The integrator runs the combined build after registration. This packet
adds no tests or runtime stubs. No
native differential, gameplay, menu, or rendering validation is claimed.
