# Text construction, loading and active-state dispatch

`gui_text_type_dispatch.cpp` implements the ready Text hooks over the same
`GuiTextLifetime`, `GuiWidgetOwner` and actual resource domains. It adds no Text
state, scene hierarchy, implementation factory or fallback virtual method.

The current table at `00D5C6C8` was read from the existing target. Its relevant
entries are:

| Offset | Table word | Target | Concrete route |
| --- | --- | --- | --- |
| 74 | `00D5C73C` | `00AB7700` | New actual mesh construction/association and zero-position hook. |
| 78 | `00D5C740` | `00AB6AA0` | New raw tail thunk to existing `00AA7170`. |
| 60 | `00D5C728` | `00AB87D0` | New native-name lookup and both-arrow visibility forwarding. |
| 38 | `00D5C700` | `00A9E0D0` | Existing `GuiWidgetOwner::base_is_visible38_00a9e0d0`. |
| 3C | `00D5C704` | `00A9E100` | Existing `base_visibility_changed3c_00a9e100`; actual body is RET4. |
| 34 | `00D5C6FC` | `00AA8530` | Existing current-visible setter/propagation. |
| 20 | `00D5C6E8` | `00AA8320` | Existing owner release with the shared lifetime's secondary-shadow hook. |

The last four targets are consumed existing implementations, not newly claimed
reconstructions. Inherited visibility reads current node+AC and returns false
for a null node or a value not greater than zero, including an unordered
comparison. It does not read the Text shadow-enable byte or the base active
byte. The notification slot is a proven empty native leaf, not a missing
implementation disguised as success.

| New entry | Native ABI / inclusive body | Coverage |
| --- | --- | --- |
| `00AB7700` | ECX Text; no stack arguments; RET at `00AB77B1`, length1 | Complete supported successful allocation path. |
| `00AB6AA0` | ECX Text; no stack arguments; JMP `00AA7170` at `00AB6AA0`, length5; end `00AB6AA4` | Complete raw thunk, `no_ghidra_function`. |
| `00AB87D0` | ECX Text; one DWORD flag; RET4 at `00AB88FE`, length3; end `00AB8900` | Complete valid Boolean0/1 path matching the type interface. |
| `00AA7E00` | ECX parent; native name header and unused DWORD; RET8 at `00AA7EAE`, length3; end `00AA7EB0` | Complete valid-list actual-name overload; native intrusive-list ABI and corrupt-list termination excluded. |
| `00B6D800` | ECX actual node; returns same native string header+54; RET at `00B6D803`, length1 | Complete storage leaf. |

Every new routine's full body was read; no bytes in these ranges remain unread.
Raw bounds use inclusive ends and exact final instruction lengths. The project
and program were verified through repository wrappers for each batch:
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Ghidra remained read-only.
Names are descriptive hypotheses; these are new C++ interfaces, not binary ABI
replacements. Allocation/SEH and corrupt native storage failure behavior are
not reconstructed as successful C++ behavior.

## Recovered order and ownership

`00AB7700` captures Text+4C in EBX at `00AB771D`, before allocation. Unlike the
Icon/FrameBox caller of the shared helper, Text does **not** test its base+74
bounds flag or keep an existing mesh. It allocates a BC payload from the
existing C0-slot mesh pool, runs the real constructor, reads D7A260 once, calls
`00B75170(0, mesh, sentinel, sentinel)` on that captured model, and releases the
creator reference. The concrete `GuiNativeGeometryOwners` helper executes
these existing actual allocation/constructor/setter/release bodies and adds no
semantic mesh wrapper. The native constant is -1.0; the model setter re-reads
its live sentinel after terminal resource callbacks.

Only then does `00AA7DC0({0,0,0})` write position and perform its real transform
and bounds updates. This call uses the widget's **current** node; the preceding
mesh association continues using the pre-allocation captured model. Captured
objects must survive native callback intervals. No extra retain is inserted.
ESI is the allocated mesh from `00AB7741`; EDI is entry Text throughout.

`00AB6AA0` is exactly five bytes, `E9 CB 06 FF FF`. It jumps to `00AA7170`, whose
actual body calls current virtual60(false) at `00AA717A`, then tail-jumps to
`00AA70E0` at `00AA717F`. The C++ function invokes that existing owner body;
the eventual actual Text implementation must bind current60 to the new concrete
arrow method. It must not substitute base `00AA6A30`, which writes the active
byte and is a different behavior.

`00AB87D0` creates an actual NativeString of length18, copies
`__Active_Left_Icon` including its terminator, searches, captures the result,
and destroys the name. It then independently creates length19 for
`__Active_Right_Icon`, searches and destroys that name, even if the left result
was null. Both lookups pass unused argument0. Only after both cleanups, and
only if both captured children are nonnull, it calls left current34 and then
right current34 with the same flag. The right implementation is obtained after
the left callback. It never changes the Text's base active field.

Register filtering of the entire routine proves ESI starts as entry Text
(`00AB87ED`), EBX captures left (`00AB8833`), ESI becomes right (`00AB88A4`),
and EDI is initially zero but becomes the original stack flag at `00AB88D7`.
That last reassignment occurs after both name cleanups and before either child
call. Captured children and their existing actual owners must survive callbacks.

## Actual-name lookup

The old layout lookup compares `GuiLayoutWidget::key`. The new overload keeps
that existing API intact and instead resolves each **current** native node
name. It walks `parent.transform.children`, the existing borrowed GUI list
projection, preserving order and duplicate payloads. The unique_ptr child
vector owns allocations separately; it cannot represent duplicate native list
entries and is not used as a second search index.

The current owner node slot controls the null-node skip. `00B6D800` returns its
same native name header at+54 without allocating or retaining. `00AA7E4C`
reads query length before node-name length; unequal lengths skip comparison,
equal zero lengths match, and equal nonzero lengths call the required current
CRT `00BF7FBF(node_data, query_data)`. No ASCII fallback, case-folded cache or
layout-key equality assumption is used. The ordinary CRT comparison contract
does not mutate the GUI list, whose C++ vector/node instances must stay stable
during traversal. Native list-node identity and corrupt-iterator termination
are outside this projection.

The complete lookup listing establishes EBP=query header at `00AA7E05`,
EBX=parent+64 at `00AA7E09`, ESI=current list node at `00AA7E0D` and after each
next link, and EDI=current sentinel at `00AA7E12`. The second stack argument
is never read. RET8 establishes its presence; neither caller constants nor
the misleading old prototype imply recursion. The current CRT callee body
was read: its live locale flag selects its ASCII/current-locale paths. The
required comparison binding must implement that actual contract.

The three Text hooks have no direct-call xrefs: their sole references are the
exact table words above. For the two generic lookup leaves, all live caller
listings were collected and their complete filtered call setup inspected in
430 distinct operand patterns: 1,485 lookup calls and 44 name-getter calls
across 196 live containing functions. One 10,734-instruction caller was paged
beyond the initial cap; no capped caller listing remains in this audit.
The report retains exact call-site/target/function rows for the checker.

Nine further lookup sites have no live containing function: `00519C20`,
`00519C64`, `0056DD0D`, `00578894`, `00578918`, `005D3C43`, `005D3CB0`,
`005D3D1D`, `005EC6C3`. Their contiguous raw call-setup windows were read;
no caller body bounds or attribution to an earlier candidate are claimed.
The getter xref at `00AA6803`, classified as a call by the index, is actually
an E9 JMP. Raw `00AA6800..00AA6807` loads widget+4C then tail-jumps to B6D800;
last instruction `00AA6803` has length5. It has no Ghidra function and is
recorded separately, not submitted as a fabricated direct CALL row.

## Factory boundary and verification

The new helpers close the current74/78/60 methods. Existing base functions
already supply current38/3C/34. A future `GuiWidgetTypeImplementation` still
needs concrete `properties_bound` composition for `00ABB630` against this
same lifetime: actual font lookup/cache release (`00AB8C30`), localized source
submission (`00ABAED0`), shadow preset/configuration and the complete content
stages/builders. The older `GuiTextHost` entry points still route geometry
through a separate semantic builder interface and cannot simply be attached
as the actual owner implementation.

Destruction and current20 must use the shared lifetime and canonical scalar
deletion/owner association, not page retirement as delete4. Beyond these
factory hooks, actual runtime dispatch still needs the Text-specific routes
for current4C (`00AB6AD0`), current58 (`00ABBF30` plus `00ABB1D0`), current64
(`00AB6D70` including its vertical helper), and current70 (`00AB7A40`, main
and shadow clipping followed by child70). Those are dependency identities,
not new complete-body coverage claims in this packet. Current50/80 already
have the shared actual style functions; remaining builder/optional-child
work must finish before a pending content update is treated as complete.

Strict MSVC Win32 C++17 compilation passed with `/W4 /WX /EHsc /permissive-`,
using the new header before integration and peer dependency headers. Exact
call-site and worker checks are recorded in the report. No tests were added.
No factory was registered and no executable path reaches this new module yet;
there is no game, rendering or ABI-equivalence claim. The integrator owns
source registration and combined build verification.
