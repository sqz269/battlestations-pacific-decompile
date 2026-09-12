# GUI pointer hit traversal

`GuiPointerHitRuntime` implements the actual `GuiPointerHitCalls` dependency of
AA3910, using the same resource owner, registry, widget runtime, input source,
and F8BC70/74/78 publications as the pointer and widget frame callers.

| Routine | Body, inclusive | Original ABI | Coverage |
| --- | --- | --- | --- |
| AA2F10 | AA2F10..AA3054, 102 instructions | ECX manager, RET | Complete normal caller on the canonical owner domain |
| AA8BD0 | AA8BD0..AA8E32, 206 instructions | ECX widget, borrowed float[3] stack argument, RET4 | Complete normal recursive caller on the canonical owner domain |

Both live bodies have zero flow gaps. Analysis was read-only in the verified BSP
project/program. The saved pseudocode misidentifies several AA8BD0 stack locals;
the implementation follows the full assembly, including current64's RET10.

AA2F10 publishes X then Y from the manager's produced position, captures CE4970,
clears the hover pointer, and stores the captured initial depth in F8BC7C. It
walks the registry in its current order. The vector begin is captured once;
end and the current page payload are read again at native iterator/callback
boundaries. Visibility precedes the current exclusive-page filter. Each accepted
page receives its own zero origin. There is no copied page list.

After the walk, a null hover or MouseBlock ends the pass. Otherwise the current
backend's first mouse is looked up and its actual current28 activity method
runs. This is A9AAB0's activity scan over query20 codes 0..16, not just a left
button predicate. A true result reloads F8BC70 before dispatching current68 with
a null stack argument. The native missing-mouse/null-current-target dereferences
are explicit errors in the host interface, without successful fallbacks.

AA8BD0 captures pivot-times-size X/Y, absolute position X/Y/Z, then child origin
(absolute minus pivot products, and Z minus the live double D7A258). Its visible
children are traversed first in the existing owning layout/transform vectors.
The child payload is read again before recursion. Only afterward are MouseHit,
hidden77, size, and current64 read for the widget's own rectangle. The existing
frame dispatcher invokes Text's AB6D70 alignment or the established base A9E120
RET10. Post-callback fields30/34 and clipping fields38..44 are read live.

The arithmetic retains native x87 binary32 stores, the FST rather than FSTP
rectangle setup, mixed UCOMISS/LAHF/TEST zero gates, operand order, and x87
containment/depth comparisons. Pointer X, pointer Y, and prior depth are fetched
at their native points rather than copied at recursive entry. Failed clipping
can avoid later field/global reads. A successful candidate publishes its actual
layout identity first and its captured child-origin Z second.

The winner is the **smallest resolved Z**, with a strict comparison preserving
the first equal-depth hit. This corrects GUI_LAYER_MANAGER.md's older claim
that horizontal half extent wins. Native widget18/1C are Pivot and widget20/24
are Size, as established by the property producer. At AA8BE6/AA8BF0 the pivot
products go to frame+1C/+20. At AA8C36 the child-origin Z goes to frame+2C.
Current64 at AA8CDA consumes all four pointers with RET10, restoring ESP. Thus
AA8DD3 and AA8DE3 read the unchanged child-origin Z at frame+2C. The scratch
slot frame+1C is repeatedly overwritten after current64 and is not the score.

The seven previously established Screen, Group, Text, ClipBox, Section,
FrameBox and Icon tables have current68 AA7190. The report records their exact
address words. AA7190 is provided by the shared frame runtime: current listener
00, current backend lookup, then captured-mouse rising-button tests and fresh
listener04/08; its other branch bubbles to the current parent. No replacement
listener owner, input device, bounds implementation or event sink is introduced.

Listbox is distinct: D5BC60 contains A9CA60, whose body A9CA60..A9CC46 handles
its own scrolling/selection/activation sequence. It is not AA7190. The current
shared dispatcher explicitly rejects that unreconstructed profile, including
when an ordinary row's parent bubbling reaches the Listbox. This is a remaining
normal interaction boundary, not completed Listbox pointer interaction.

The host domain retains live widget owners and child membership/order through
callbacks, as does the existing frame runtime. The current C++ child projection
has vectors, not native list-node identities; arbitrary child-list mutation
during callbacks is not claimed. Registry storage invalidation is detected at
the corresponding iterator boundaries. Native checked-STL failure handlers,
exception continuation, allocation failure, binary ABI, hardware polling and
game/render validation are outside this API. Unsupported derived profiles and
missing providers fail explicitly. Validation results are recorded in the report.

Release Win32 /W4 /WX /fp:strict build and the fresh cache's reconstructed_math
CTest passed (1/1). The ignored one-TU fixture linked the built production
libraries and used actual Group constructors/owners: depth8 won over a smaller
pivot product, equal depth11 retained the first child, and live field38 rejected
that first candidate. It also checked empty-registry AA2F10 XY publication and
hover/depth reset. No unresolved fixture provider was reached. This does not
exercise populated page traversal, Text alignment, activity/current68, native
child attachment production, enabled AA3910, rendering or game behavior.
Object inspection confirms all manager XY/depth and winning depth publications
use MOVSS, with hover published before winning depth and no extra x87 conversion.
