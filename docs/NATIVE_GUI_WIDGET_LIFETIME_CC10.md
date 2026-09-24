# Raw GUI widget base lifetime

Addresses: `00AA9730`, `00AA8320`, `00AA6F30`, `00AA77B0`, `00AA7F50`,
`00A9B740`, `00A9BCE0`, `00A9E070`, `00AB6A30`, `00AB73B0`, `00CB7421`.

This packet supplies the complete normal raw AA9730/AA8320 schedules and the
reviewed source C++ exception cleanup. It can discharge the base obligation of
an AA9390-created object, including the previously retained AC6600 prefix. It
does not finish AC6600, supply raw derived child destructors, return the widget
pool slot, or bind application shutdown. Numeric profiles are evidence identities,
never host code addresses. Existing logical `GuiWidgetOwner` projections are
unchanged and cannot be passed as these raw objects.

| Original body | Coverage | Source operation |
| --- | --- | --- |
| AA9730..AA99B8, 649 B, ECX widget, RET | Complete normal schedule; required virtual/list bindings | `destroy_native_gui_widget_base_00aa9730` |
| AA8320..AA8396, 119 B, ECX widget, RET | Complete normal schedule; required non-base virtual bindings | `release_native_gui_widget_scene_00aa8320` |
| AA6F30..AA6F8E, 95 B, ECX header, signed capacity stack, RET4 | Complete alias | Existing raw B22D10 reserve |
| AA77B0..AA77FF, 80 B, ECX header, signed count stack, RET4 | Complete alias | Existing raw B22CC0 resize |
| AA7F50..AA7F66, 23 B, ECX header, RET | Complete alias | Existing raw B27F50 resize-zero/free |
| A9B740..A9B787, 72 B, ECX list, RET | Complete alias | Existing actual 4C5940 list teardown |
| A9BCE0..A9BCE4, 5 B, ECX list | Complete tail alias | A9B740 |
| A9E070..A9E097, 40 B, descriptor stack, AL result, RET4 | Complete low-byte result | Scan actual F8BC88/F8BC8C cells |
| AB6A30..AB6A35, 6 B, EAX result, RET | Complete | Read actual F8BE28 cell |
| AB73B0..AB73CD, 30 B, ECX raw Text, RET | Complete | Release actual +188 node then clear slot |

The array alias evidence compares all bytes, canonicalizing only the known CALL
relocations/dependency aliases. The list alias is byte-identical after the two
CALL relative operands resolve to the same BF65AC. Neither alias copies storage
or introduces a new STL implementation. A9B720 allocates the same 0Ch node as
the existing 4C3200/A4C4A0 producers: next, previous, untouched payload. AA9390
places that sentinel in raw widget+68 and count at +6C, with allocator +64
untouched. Its +88/+8C/+90 header is the existing raw pointer-array layout.

AA9730 publishes D5C130 before its direct AA8320 call. AA8320 walks list nodes
in order, calls each child's current slot20, then reads the node's next link
after the callback. It captures the current widget table's slot0C **address**
before AB6A30, loads the slot's target after the getter, and dispatches the actual
class query. D5C130 slot0C is A9E070, whose two borrowed lineage cells are read
in order. The usual initialized descriptor domain returns false for Text; this
packet does not hardcode false or substitute fabricated type IDs. A true result
requires a real raw Text extent and releases its +188 node. Main +4C releases
through the canonical actual node lifetime and is cleared after the callback.

The child deletion loop uses current count, calls current first child's slot04
with flags1, and reloads the first link. Native child deletion must detach itself
from this same parent list. A null payload or missing self-detach does not cause
an invented erase/decrement; it can loop forever natively. Deleting wrappers
bypass +04 reference counts. This packet supplies no replacement widget owner,
second retain/count, or generic raw-to-logical child adapter.

Parent is captured from +70. If a node was installed after the scene pass,
B6E680(null), B6D890(null), and conditional parent-node B6D940 execute in order
with current node fields. A9BD50 receives parent+64 and the address of a captured
widget pointer; +70 is cleared only after it returns. A9BD50 is an explicitly
required library binding: remove every matching node+8, preserving checked
iterator behavior, current count updates, CRT frees, allocator and sentinel.
No compatible raw widget library binding was found; the logical list helper is
not used. The raw node path uses `NativeNodeDestructionRuntime.attachments` to
resolve the existing actual pointer key and the existing backed transform and
virtual18 lifetime. Missing identities fail explicitly, without fake cleanup.

Timed entries use signed current count, re-read the raw header after callbacks,
and call current entry slot0 with flags1. The original captured pointer cell is
cleared after deletion, even if the header changes. Existing source timed-entry
owners use logical widget pointers; they are not adopted here. Their scalar
deletion remains a required raw binding. Final resize-zero can allocate one
slot when capacity is negative; data/capacity remain stale after free. AA9730's
inline negative-count branch tests the sign of the wrapping **byte offset**,
unlike AA77B0's element-index loop; that distinct branch is retained explicitly.
Then list
storage is destroyed, +68 becomes zero, and D5C104 -> BD30F0 -> CEB130 publishes
the base profiles. +04, opaque bytes, derived fields, and allocator slot metadata
are preserved. No allocation return for the widget occurs.

## Constructor and exceptional lifetime

AA9730's FH3 handler is raw CB7421..CB742A: MOV EAX,DEDCA0 then JMP BF6B43.
It has no Ghidra function at packet preparation. Metadata DEDCA0 has magic
19930522, max state3, unwind map DEDC88 and no try map. The map is:

| State | Next | Original cleanup |
| --- | --- | --- |
| 2 | 1 | CB7413 -> AA7F50(widget+88), array backing only |
| 1 | 0 | CB7408 -> A9BCE0(widget+64), list storage only |
| 0 | -1 | CB7400 -> AA6E10(widget), D5C104/BD30F0 |

State2 is published before AA8320. State1 is published at AA9895 before final
resize-zero; state-1 at AA9993 before terminal base destruction. Source C++
exceptions follow exactly those remaining cleanup stages; another cleanup
exception terminates. This is not native FH3/SEH identity. Abandoned child/entry
payloads are not magically destroyed by container unwind, and a retained raw
slot is not safe to retry after cleanup. Contexts, borrowed tables/type cells,
actual node companions and allocation domains must outlive every callback.

AC6600 state0 requires AA9730 after successful AA9390. State1 first requires
41DD20(page+100), then AA9730. The existing prefix frame retains those resources;
the caller can now explicitly perform these recovered stages with this domain.
The frame has no new automatic destructor, and this packet does not alter it.
Pool return must follow all derived/base obligations using the existing actual
allocation owner. Constructor failure before AA9390 returns stays with AA9390.

## Evidence and verification

Live project/program checks precede each Ghidra batch. Full source/assembly,
native bounds, call rows, table bytes and EH bytes are in the JSON report. All
eleven owned native spans match the installed PE. Existing Ghidra AA7F50 ends
at AA7F61 and A9B740 at A9B77B; original-byte inspection recovers their RET tails
through AA7F66 and A9B787 and A9B768..772 interior gap. No worker mutations.

The single focused comparison constructs two raw AC6600 prefixes using actual
page/string pools, destroys each name as state1 requires, and compares complete
AA9730+AA8320 original execution with source cleanup. Children/timed entries are
empty and main node is null. Original AB6A30/A9E070 run with explicitly supplied
distinct fixture type cells; class initialization is not tested. Allocator/free
and BD30F0 are source bridges. Every unsupported native branch traps if reached;
no child or node operation silently succeeds. All 128h slot bytes are compared
after normalizing independent stale name pointers and pool indices. Reference
count, pool indices and untouched +E4/+E8 are checked separately. This does not
test populated owners, nonnull nodes, native exceptions, ABI substitution or
gameplay. Build, existing checks, strict compile, call verifier and probe results
are recorded in the report; local evidence is ignored and supplied for archive.

## Primary integration and listing repairs

Published fbb20ad69 in main d936682ba with the strict Win32 build and all three existing CTests passing. The primary defined exact handler CB7421..CB742A under the write lock. Call-site override repairs decoded the missing cleanup tails; their stored function bodies still needed a separate exact recreation. That recreation preserved names/comments and now covers AA7F50..AA7F66 and A9B740..A9B787, with 10 and 27 instructions respectively and no remaining gaps. The project was saved and all affected exports refreshed. The original partial-repair records remain as history, alongside the final definition records. See reports/cc10_gui_widget_function_definitions.json and reports/cc10_raw_lifetimes_integration.json. These metadata repairs do not extend runtime, populated-child or exception evidence.
