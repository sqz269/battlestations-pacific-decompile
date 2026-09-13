# Native Text retained identity prefix

Addresses: `00AA9390`, `00AA9520`, `00AB9650`, `00ABB2C0`, `00AB8250`,
`00AB8EE0`, `00AA9730`. Names are reconstruction hypotheses.

This packet implements the proven eight-byte native identity prefix in the
existing Text pool allocation. The canonical `GuiWidgetOwner`/`GuiTextLifetime`
continues to own the represented body. The prefix is not a complete `1F4h` Text,
cannot receive arbitrary native Text calls, and does not construct native
strings, child lists, vector headers, SEH state or the remaining payload.
The pool's `+1F4` allocation metadata is never touched by these fragments.

| Original routine / ABI | Proven identity behavior | Coverage |
| --- | --- | --- |
| AA9390..AA9513, ECX destination, type DWORD stack, EAX destination, RET4 | AA93AA writes CEB130; AA93B5 writes fresh count1 at +04; AA93CB writes D5C130 | Partial: these three stores only, before remaining base effects |
| AA9520..AA9722, ECX destination, source DWORD stack, EAX destination, RET4 | AA953B/AA9546/AA9551 repeat the same stores; source count is not copied | Partial: identity prefix, before all copied fields/child allocation/model clone |
| AB9650..AB98EE, ECX Text, EAX Text, RET | AB9671 calls AA9390(type3); AB9678 publishes D5C6C8 | Partial: derived profile publication only |
| ABB2C0..ABB623, ECX destination, source DWORD stack, EAX destination, RET4 | ABB2E5 calls AA9520; ABB2FD publishes D5C6C8 before first string copy ABB303 | Partial: derived profile publication only |
| AB8250..AB83CF, ECX Text, RET | AB826F writes D5C6C8 before shader, shadow and glyph destruction | Partial: entry profile store; existing canonical lifetime supplies body |
| AA9730..AA99B8, ECX widget, RET | AA9752 writes D5C130 before AA9760 scene release; AA999B writes D5C104; AA99A1 calls BD30F0, which writes CEB130 | Partial: entry/end profile stores and existing BD30F0 only |
| AB8EE0..AB8EFF, ECX Text, flags DWORD stack, EAX original Text, RET4 | AB8EE3 destroys Text; only flags bit0 enables AB8EF5 pool return | Existing canonical scalar body/transport consumed, no second implementation or pool |

The source default/copy producer pseudocode was checked against assembly because
the copy body contains register-derived inputs and x87 copies. Text publication
and scalar/base destruction stores were checked in the listings, including the
real post-`_free` tails. Live wrapper queries verified the configured `bsp`
project and `/battlestationspacific.exe`; workers made no Ghidra mutations.
AA9390 and AA9520 have other widget callers; the covered prefix is unconditional
and independent of their type/source arguments. The Text callers are AB9671 and
ABB2E5. AB9650 is reached at AA13DD/AB9D4F and ABB2C0 at AA13BA. AB8EE0's sole
direct reference is the Text table's scalar slot; indirect callers remain
current-profile dispatches. AB8C30 was checked and is font selection, with no
producer/terminal identity role; it is not reconstructed here.

The first two words verified in the current Ghidra image are:

| Profile | current0 | current4 |
| --- | --- | --- |
| D5C6C8 Text | BD30E0 | AB8EE0 |
| D5C130 widget | BD30E0 | AA9A90 |
| D5C104 intermediate base | BD30E0 | AA6E30 |
| CEB130 reference base | BD30E0 | 4F9F40 |

`NativeGuiTextIdentityReference` borrows the actual prefix atomic; construction
neither increments it nor fabricates another count. Its zero-reference entry
uses existing BD30E0, whose no-argument body reloads current profile and calls
current4(flags1) without decrementing. The supported Text dispatch concretely
calls `GuiTextChildDeletion::delete_text_child_virtual4` on the same owner's
layout. The callback can destroy the companion; no access follows it. Other
profiles, reentrant/unfinished deletion and absent flags1 allocation transport
remain explicit unsupported terminal states, never successful no-ops.

The existing B18A40 implementation is reused. AB8BB3/AB8BB5 push flags1 and the
same Text EDI; AB8BB8 calls it with material EBX. EDI originated from caller ECX
at AB892D, remains Text through AB8BB5, then is overwritten at AB8BC5. B18A40
decrements old retained `+0C` at B18A58, dispatches its current0 at B18A68 only
on zero, clears old slot at B18A6A, publishes new pointer/byte at B18A7C/B18A7F,
and increments incoming actual `+04` at B18A91. Equal identity still releases
and reacquires. The new helper verifies the same actual-owner lookup before
that operation and adds no protective retain or post-callback access.

An explicit scalar delete is distinct from dropping a counted reference.
AB80C0's AB8151 and AA9730's AA9797 directly call child current4(flags1), with
no count decrement or count==1 check. AB8EE0 also never reads +04. Consequently
a live Text with creator count1 plus its cursor material retain has count2;
direct teardown is allowed to start at2. Text owns its primary Model subtree,
including cursor Model -> mesh -> section -> material -> retained Text. Base
entry publishes D5C130 before releasing that subtree, allowing its material to
drop Text back to1, followed by scalar pool return despite residual creator1.
Converting direct deletion to decrement-only would leave this cycle alive;
requiring count1 before teardown would reject the normal count2 case. This
count trajectory is a source-derived ownership example, not gameplay proof.
Arbitrary external references surviving explicit deletion are not made safe
by this reconstruction. A ref-zero callback during unsupported base teardown
is rejected rather than dispatched incorrectly as Text.

Factory/lifetime integration must initialize this prefix before remaining base
effects, use its atomic for the canonical owner, publish the derived profile
before derived construction, and register exactly one reference in the existing
actual-owner domain before cursor retention. It must call the profile helpers
at the specified derived/base phases and preserve prefix, lookup and companion
through callbacks. Only after completed body teardown may existing factory
transport unregister and return the same slot through AB75A0. Flags0 retains
storage, with ended body and final CEB130; it is not a live retainable Text.
This worker does not edit those shared integration files.

Validation and the exact call-site rows are recorded in
`reports/native_gui_text_identity.json`. No game execution, whole native Text
ABI, native constructor unwind, arbitrary indirect profile or render parity
claim follows from these fragments.
