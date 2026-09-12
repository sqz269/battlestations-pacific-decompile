# Canonical Listbox frame and navigation

Addresses: 00A9D030,00A9C400,00A9DA60,00696470,00AA0F50.

The existing `GuiListboxRuntime` now implements the actual Listbox frame and
both navigation slots over its same FC selection, borrowed14C paging list,
listener114, fields and attached row owners. No widget, node or list tree is
cloned. Descriptive names are hypotheses. These are new C++ interfaces, not
raw180h/checked-STL/native calling-convention replacements.

Evidence came from `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, through target-verifying read-only wrappers.
No Ghidra definition, name, comment, flow override or save was changed.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| A9D030..A9D358 current40 | ECX Listbox; float dt stack; RET4 | Complete normal valid-owner caller sequence with required downstream providers |
| A9C400..A9C537 current88 | ECX Listbox; raw low-byte forward stack; RET4 | Complete valid FC iterator domain |
| A9DA60..A9DE46 current84 | ECX Listbox; raw low-byte forward stack; RET4 | Complete valid FC/14C domain with produced EC/158/164 on reached reads |
| 696470..6964AB default input | ECX previous output, EDX next output, activation output stack; RET4 | Complete three-call sequence; actual4D92B0 provider required |
| AA0F50..AA0F6E selector | ECX manager; signed selector stack; EAX borrowed widget; RET4 | Existing complete selector revalidated |

The A9DF74 constructor writes D5BBF8. Live bytes establish current40 at
D5BC38=A9D030, current84 at D5BC7C=A9DA60, current88 at D5BC80=A9C400 and
current8C at D5BC84=A9BA40. A9C400 is not current84. A9DA75 loads current88
when the raw149 byte is zero; A9DFFC initializes149 to1.

A9C400 moves one iterator node. It restores the old position without80 when
that one target is hidden; it does not skip an arbitrary hidden run. Forward
movement first compares the selected row pointer against the final row pointer,
so repeated row identities can stop navigation before the final iterator.
Backward movement at the first iterator returns; backward from end can select
the last node. Invalid increments/decrements fail at their native boundary.

A9DA60 wraps and skips hidden nodes unless121 is nonzero. From an end selection,
the forward arm first assigns begin and then increments. At the final forward
boundary it reads158 and154 before testing paging148, compares their unsigned
values, and updates rawEC when appropriate. The remaining count uses wrapping
SUB, while page-start164 comparisons are signed. Backward at begin reads and
decrements nonzero rawEC even when paging is off. Unwritten EC/158/164 are
explicit optional-state failures rather than fabricated constructor values.

Paging updates164 then calls the existing real D870 rebuild, which can invoke
row/listener/sound callbacks. The forward path reselects the rebuilt final row;
the backward path assigns begin, increments, then performs its common decrement.
Final-page navigation restores the last iterator before listener0C(false,true,
Listbox); first-page navigation uses0C(true,false,Listbox). The callback may
change selection, paging or hidden flags; subsequent checks read those live
fields. Both arms then invoke current80(false). Current row nodes, actual
type profiles and borrowed widget owners must remain valid through native
iterator advances. Existing D870 restrictions on replacement14C apply.

A9D030 first checks actual hidden77, active85, current38 and the returned
manager's raw70 byte. Native AA5D70 leaves70 unwritten; the parent's canonical
AA4F80 frame producer publishes it atAA4FA3 and resets0 atAA503F after normal
completion. `GuiResourceState::blocked_70` therefore remains optional until
produced. The frame does not use a cached flag from a different manager.

The parent frame runtime retains the active owner across this derived method
and its concrete AA87B0 base call. The base receives the original dt through
FLD/FSTP float32, exactly once. Paging delay168 uses MOVSS/COMISS against live
D7A218, ordered JBE, then x87 subtraction of the original dt and float32 store.
It is not clamped at zero. NaNs take the native no-subtraction branch and the
native comparison instruction is retained rather than replaced with a scalar
C++ comparison. Complete signaling-NaN/trap/native ABI equivalence is untested.

Selection is cleared when8C reports no selectable rows; otherwise an end
selection invokes C310. Then the current144 callback is used when nonnull,
otherwise the currentF8BC08 binding. A9E02A initializes144 null and A9AC74 is
its explicit setter. At4DD6E5..EA the game registers696470 as the default.
That fastcall obtains4D92B0 actions46,47,4A in order and writes previous,next,
activation output bytes. The C++ callback must return all three raw bytes.
Adapters bind actual external state; mutable callback-owned substitute game
state is outside this transport's contract.

Activation obtains backend class0/index0, reloads F8BBF4, obtains class2/index0,
then queries captured keyboard2C before gamepad2C with native short-circuiting.
Keyboard D5B904+2C is the seven-byte A95F00 tailcall to current28=A95ED0,
which scans current20 codes0..255. D5B7F0/D5BB48+2C is A93F60, scanning0..59.
Their actual provider is required; existing raw-storage input implementations
cannot receive a typed `InputDevice` cast. The separate input worker owns those
addresses. A95F00 currently has no Ghidra function; inclusive endA95F06,
bytes `8b 01 8b 50 28 ff e2`. No definition was manufactured here.

A reached activation queries current selection before actual listener04(row,
Listbox). It then reloads the existing F8BC0C sound binding with CL0/DL1 even
if there was no listener or selected row. Without activation, the two raw
direction bytes are compared before nonzero tests: different nonzero bytes
choose forward. Current84 is followed by another80(false), retaining native
double-refresh behavior. Automatic row control then rechecks11E/85/77 and
applies state1 to the live selected iterator,3 to other hidden rows,0 otherwise.

Finally the selected row pointer is captured before120 is read. With120 nonzero,
the actual manager and live118 select a highlight, resolved depth is read,
and the same C540 owner implementation runs with current148 and captured row.
AA5E20 produces manager74/78 from actual hl_FrameBox/hlCircle_FrameBox children.
AA0F50 returns only74 for1 and78 for2; selector0 returnsnull. A9D323 pushes0
and A9D333 dereferences its result before34 even when no row is selected.
That native invalid-state branch fails explicitly. A9DF96 initializes120=1,
and normal A9E400 properties supply NewHighlightIndex(default1); no selector0
fallback resource or success path is invented. This resolves the apparent
pseudocode contradiction without claiming120 cannot be changed elsewhere.

The report lists every85 CALL site with its containing function, native target
or explicit indirect slot, and ABI cleanup. The verifier reports78 numeric rows
and zero failures:64 are direct calls,14 are numerically resolved indirect calls
supported by table/producer evidence. Seven more rows are symbolic indirect
boundaries. The mechanical verifier does not establish an indirect callee body.
The sole A9DA60 listing gap is unreachable alignment A9DDBD..A9DDBF,
`8d 49 00` (LEA ECX,[ECX]) after JMP A9DDC0. Correct library names are retained.

Validation: strict standalone MSVC Win32 `/W4 /WX /fp:strict` compilation passed.
The emitted object retains the delay MOVSS/COMISS/JBE then FLD/FSUB/FSTP sequence.
The initial required build failed downloading Lua from lua.org. A retry reused
the parent's existing dependency sources through local CMake cache overrides.
`scripts/build.ps1` then passed; after `ghidra_export.py verify-seeds`, a second
run passed both reconstructed_math and native_math_differential. Those existing
math tests are not Listbox native differential tests. The report pins source,
library and log hashes. The parent's integrated frame/listener fixture remains
separate; native Listbox differential, rendering and gameplay are unvalidated.
No permanent test was added.
