# Raw widget visibility

Addresses: `00AA8450`, `00AA8530`, `00A9E0D0`, `00A9E100`.

This packet implements four raw bodies without a logical `GuiWidgetOwner`,
registry tree, new container, or library port. Names are descriptive hypotheses.

| Entry | Inclusive end / bytes | Original ABI | Coverage |
| --- | --- | --- | --- |
| `AA8450` | `AA852D` / 222 | ECX widget, five byte-valued stack arguments, `RET14` at `AA852B` (3 bytes) | Complete normal valid-list body |
| `AA8530` | `AA85AE` / 127 | ECX widget, byte-valued requested, `RET4` at `AA85AC` (3 bytes); another RET4 at `AA8595` | Complete normal body |
| `A9E0D0` | `A9E0F0` / 33 | ECX widget, EAX0/1, RET0 | Complete leaf; source supplies live-zero address in EDX |
| `A9E100` | `A9E102` / 3 | ECX widget, ignored stacked value, RET4 | Complete genuine no-op leaf |

Live Ghidra `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, contains all
four function bodies. Their 385 bytes and the existing 60-byte B6DA70 provider
match the installed PE. No Ghidra mutation was performed.

## Bytes and dispatch

`NativeGuiWidgetVisibilityBindings` resolves the current raw profile to a table
without side effects. Entries are numeric native targets; they are not invoked
as source code addresses. Targets `A9E0D0` and `A9E100` select the concrete raw
leaves. All other +38/+3C targets require genuine supplied implementations.
There is no unknown-profile or unknown-target base fallback. Every native call
site reloads the current profile and slot.

Base profile D5C130 maps +38 to A9E0D0 and +3C to A9E100. The getter reads current
widget+4C, then node+AC and the borrowed live float D7A218. MOVSS/COMISS/JBE return
false for null nodes, nonpositive factors and unordered comparisons. It never
reads widget+E4. The explicit naked source retains leaf comparison flags and
masked SSE exception behavior. A9E100 is exactly RET4, not a fallback callback.

AA8530 starts the ancestor accumulator at byte1. Each ancestor +38 callback is
followed by a CURRENT ancestor+70 reload, then `accumulator &= returned_AL`.
Return byte02 clears the accumulator; converting it to bool would be wrong.
The walk stops at a null ancestor or zero accumulator. The widget+75 byte is
then captured and AA8450 receives `(accumulator, accumulator, requested,
recurse, 1)`. After propagation, widget+4C is reread. If nonnull, CURRENT +75
is read again and an FLD1/FLDZ/FSTP factor selected from requested is sent to the
existing actual-node `set_native_node_visibility_factor_00b6da70`.

AA8450 returns immediately for null widget+4C, including its GUI descendants.
Before/after values are normalized to 0/1 exactly where native TEST/MOV does so.
Requested, recurse, and apply_requested retain their full low byte. Child apply
is `(recurse != 0 ? 0xff : 0) & apply_requested`; it is not a normalized bool.
Recurse0 still walks GUI children, but makes them use their current visibility.

The native partially written stack DWORDs have unspecified upper bytes: the
before scratch and after argument receive BYTE stores, and NEG/SBB changes DL
only before the DWORD AND. The new source interface promises byte semantics,
not equality of upper stacked DWORDs or outer register/stack ABI.

## Actual list and callback order

The GUI children are a circular linked list at widget+64: +68 is its sentinel,
+6C its count, node+0 next, node+4 previous, and node+8 the actual child pointer.
This traversal does not read count. The same layout is already used by native
widget construction/lifetime code. No std::list or vector copy is introduced.

The +4C gate is read once before virtual calls. After the conditional +38 reads,
a before/after difference dispatches the CURRENT +3C. Only then does traversal
capture the current sentinel's first node. Each iteration compares to current
sentinel, reads current child+8, recurses, validates against the current sentinel,
and reloads node+0 AFTER child callbacks. No next pointer is cached across them.
Callbacks may change profiles, parent links, current heads/next links, model
pointers and +75 within the retained-live-storage domain.

Native invalid-list call sites target BF6713. Source retains the two current-head
checks through the existing CRT invalid-parameter entry; native list-base-null
debug branches are outside the valid-widget domain. Native CRT failure behavior
is not claimed and no CRT/container implementation is supplied.

## Validation and boundaries

The strict MSVC Win32 build and both enabled CTests pass. Eight direct native
call rows verify; four virtual sites were checked in the listing and base slots.
The report records the actual scopes, hashes, source interfaces and limits.

One ignored focused probe (`local/output/cc10_visibility_probe.cpp`) relocates
the four complete original bodies plus original B6DA70. Calls, the COMISS global
address, and fixture virtual tables are rebound; invalid-list calls abort if
unexpectedly reached. Corresponding original callable tables and source numeric
target views drive the same actual derived byte callbacks. Table resolution has
no side effects. This is not an installed application dispatch binding.

Seventeen getter pairs compare return values, arithmetic EFLAGS, MXCSR and x87
status for signed zeros, positive/negative values, infinities, quiet/signaling
NaNs, a changed live zero, and null-node/no-constant-access. Nine flow pairs
compare all 4,516 actual widget/node/list bytes and the exact callback trace.
They cover early pruning, recurse0 traversal, noncanonical bytes, parent relink,
profile change between getters, current notification target, sentinel/next
changes during child notification, post-callback node/+75 rereads, and both
final node-factor branches. Compile-time NDEBUG rejection keeps assertions live.

The complete normal source requires aligned live objects, valid retained list
nodes through callback return, finite hierarchies, a live zero binding, and
genuine returning dispatch. No native invalid-parameter behavior, exception or
unmasked FP trap delivery, full stacked upper-byte/machine-state parity, outer
binary ABI, installed application binding, or game parity is claimed.
