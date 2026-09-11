# Text content prefix and glyph-child clearing

Addresses: `00ABA8D0`, `00AB80C0`, `00AB8EE0`, `00AB75A0`.

`gui_text_content.cpp` reconstructs the content comparison/assignment prefix and
changed-empty branch over the existing `GuiTextWidget`, retained GUI owner and
actual mesh/section storage. It also reconstructs the glyph-child clear loop,
using the actual `00AA83A0` detach implementation. It does **not** provide a full
Text owner or complete nonempty text rendering.

| Routine | Native ABI and inclusive body | Coverage |
| --- | --- | --- |
| `00ABA8D0` | ECX Text; one UTF16 wrapper pointer; `RET 4` at `00ABAEC4`, length 3; body through `00ABAEC6` | Partial projection: entry through the jump at `00ABA9BA..00ABA9BE`, plus temporary cleanup `00ABAE8A..00ABAEC6`. Nonempty `00ABA9BF..00ABAE89` remains unimplemented. |
| `00AB80C0` | ECX Text; no stack arguments; `RET` at `00AB81B6`, length 1 | Complete valid-collection control flow; actual Text deleting dispatch remains a required binding. Native corrupt-vector termination is replaced by explicit C++ failure. |
| `00AB8EE0` | ECX Text; stack deletion flags; returns original Text pointer; `RET 4` at `00AB8EFD`, length 3; end `00AB8EFF` | Complete body analyzed, not reconstructed. Calls Text destructor, then pool return iff bit 0 is set. |
| `00AB75A0` | ECX pool, stack object; `RET 4` at `00AB7609`, length 3; end `00AB760B` | Complete body analyzed, not reconstructed. Critical-section protected return to the pool block/free-slot bookkeeping; no native pool is invented here. |

All names are descriptive hypotheses. The C++ interfaces use new ownership and
string representations, not native ABI replacements. The complete listings of
the first two routines were read. No bytes in their bodies remain unread; the
nonempty tail's helper closure is deliberately not claimed complete. The
project/program were checked through the repository Ghidra wrappers for each
batch. Ghidra remained read-only.

## State and ownership

`GuiTextContentBinding` borrows the same `GuiWidgetOwner`, `GuiTextWidget`, live
`NativeNodeBinding*& shadow_188`, and glyph-pointer vector projecting
`Text+198/+19C`. The vector is distinct from the canonical GUI child ownership
list. It must be the real current collection, not a snapshot.

The optional-child producer is `00AB98F0`: `00AB9D38` allocates a Text pool slot,
`00AB9D4F` runs `00AB9650`, and `00AB9D70` appends that instance to the parent
collection at `+194` (data at `+198`). The constructor's Text vtable is
`00D5C6C8`; its `+04` word at `00D5C6CC` is `00AB8EE0`. That scalar deleting
destructor calls the existing analyzed `00AB8250` at `00AB8EE3`, then calls
`00AB75A0` at `00AB8EF5` with ECX `00F8BDF0` and the original instance when
flags bit 0 is set. The other direct pool-return caller is `00AB76F6` in
`00AB76F0`, also loading ECX `00F8BDF0` and passing the incoming instance.

The retained GUI tree owns `unique_ptr` wrappers, while native GUI lists borrow
pointers. Actual detach therefore returns any transferred wrapper ownership.
The required `accept_detached_child` transport retains that **same instance**
before the native post-detach slot reload. This is C++ lifetime bookkeeping,
not an extra native call or a new widget tree; it must not change native-visible
state. The native callbacks can replace the slot with another child, so the
transported instance need not be the one deleted next.

`delete_text_child_virtual4(child, 1)` must execute real Text destruction and
pool return, consume that instance's transported ownership when applicable,
and clear `before_destroy` before destroying its C++ wrapper. Dropping the
wrapper or calling `GuiWidgetOwnerRuntime::retire_tree` is not equivalent:
page retirement performs a separate virtual20/node-release pass first. There
is no default delete implementation or fallback in this packet. The full Text
destructor/lifetime owner remains a dependency.

The raw main/shadow mesh and section are resolved through the canonical
`NativeModelReference`, `NativeMeshReference`, `NativeMeshSectionReference`
and their current owner registry. The code verifies matching live storage and
service domains. The shadow model is resolved by its actual node-storage key,
not by casting a node-binding wrapper to a model wrapper. Mesh section counts
use the existing `NativeMeshSectionStorage::range_words_0c` declaration.

## Recovered order

1. Call `00AB8530` before copying or comparing the source, including equal-empty
   input. Its actual body creates missing shadow/section resources and performs
   its applicable shadow parenting; an assertion that resources already exist
   is not an equivalent implementation.
2. Copy the UTF16 wrapper, read the live font's `+48` uppercase byte, and repeat
   the locale uppercase pass when enabled. This pass also runs after ellipsis
   already uppercased the text: mappings are not assumed idempotent.
3. Compare wrapper empty/nonempty states first. For two nonempty strings call
   the native case-insensitive comparison; there is no separate length-equality
   check. `00C03A39` uses ASCII A-Z folding while the live `0109DE1C` flag is
   zero, otherwise `00C0392A(a,b,null)` uses the current CRT locale. The latter
   remains a required actual comparator. Equal input returns without changing
   stored text, width or section counts after step 1.
4. Changed input first zeros measured width `+114`, then clears glyph children.
   Capture main model geometry0 and its section0 before assigning the transformed
   copy to the stored text `+EC/F0`.
5. For changed-empty input, write main section `+18=0`, then `+10=0`; reload the
   current shadow slot; resolve its geometry0/section0; write shadow `+18=0`,
   then `+10=0`. Stream identities, primitive kind, materials, line/height/origin
   metrics and other section words remain as they were.

The clear loop reloads the pointer vector and bound after each detach, captures
the resulting slot address before deleting, clears that captured slot only
after virtual04 returns, and rechecks the live size next iteration. Callbacks
may replace entries or append children, but virtual04 must not invalidate the
captured slot address (the same native EDI-slot precondition). If detach removes
the indexed slot entirely, the C++ function throws instead of native CRT
termination. After the loop the native erase's move length is zero for a valid
serialized vector, so `vector::clear()` preserves the allocation and models
the end-to-begin update without deleting pointed-to children.

For changed-nonempty input the return value is
`GuiTextContentBranch::needs_nonempty_geometry`, with the separate transformed
temporary and captured actual main mesh/section. It represents suspension at
`00ABA9BF`, **not successful native completion**. Those borrowed objects must
remain alive until the actual continuation resumes. That tail still allocates
buffers, establishes material/shader parameters, chooses/builds layout, copies
streams/counts into the shadow, applies color and updates shadow placement and
parent/root registration. The existing fixed-font `FontGeometryOwner` fragment
cannot substitute for that live Text-owner continuation. No caller-level final
virtual50 color call is done here.

The input domain is null-free UTF16 with lengths fitting native signed32 and
the existing live font/locale resources. Native string allocation/SEH and
corrupt storage behavior are not reconstructed. Invalid ownership fails
explicitly; it does not synthesize models, child owners or locale defaults.

## Call evidence

| Containing routine | Call site | Native target | Contract used |
| --- | --- | --- | --- |
| `00ABA8D0` | `00ABA8EC` | `00AB8530` | Direct `ensure_gui_text_draw_sections_00ab8530` over borrowed `GuiTextBufferServices`, supplied by the peer buffer/section packet. |
| `00ABA8D0` | `00ABA8FA` | `004C8DD0` | UTF16 copy constructor; valid-domain behavior represented by a separate `u16string`. |
| `00ABA8D0` | `00ABA91C` | `00A9EC30` | In-place uppercase; actual ECX locale manager and one stack wrapper, `RET 4`, not cdecl. Reuses `locale_uppercase_00a9eba0` per code unit. |
| `00ABA8D0` | `00ABA945` | `00C03A39` | Two terminated strings; `ADD ESP,8` at `00ABA94A`. |
| `00C03A39` | `00C03ABF` | `00C0392A` | Current-locale arm: three arguments including null locale; `ADD ESP,0Ch` at `00C03AC4`. |
| `00ABA8D0` | `00ABA964` | `00AB80C0` | Concrete live glyph-child clear. |
| `00AB80C0` | `00AB8118` | `00AA83A0` | Concrete retained-owner detach, including null-child calls; parent packet owns its implementation. |
| `00AB80C0` | `00AB8151` | Text virtual04, resolves `00AB8EE0` | Required actual deleting dispatch, stack flag 1; pointer reloaded after detach. |
| `00ABA8D0` | `00ABA96D`, `00ABA9A7` | `00B74640` | Existing actual model geometry0 access. |
| `00ABA8D0` | `00ABA97B`, `00ABA9AF` | `00B732C0` | Existing actual raw mesh section0 access. |
| `00ABA8D0` | `00ABA98D` | `004C5E20` | Assign transformed copy after main-section capture. |

All seven direct content-assignment sites were inspected: `00ABAF66` and
`00ABAFAA` in `00ABAED0` submit localized or widened temporary wrappers;
`00ABB0D2` and `00ABB151` in `00ABB000` submit ellipsis results; `00ABB275`
in `00ABB1D0` resubmits copied current text after clearing its stored wrapper;
`00AB9E61` in `00AB98F0` submits a glyph-child wrapper; `00AB6AB8` in
`00AB6AB0` forwards its incoming wrapper. ECX is the current Text in each case.

All twelve clear call sites were inspected: four at `0053266D/684/69B/6B2`
belong to the live Ghidra body `00530A60..005329B9`, despite the older index's
`00532360` entry; they clear Text handles `+28/+2C/+30/+34`. `0054B608` in
`0054B530` clears each command text; `005D2BCB/BE2/BF9` in `005D26D0` clear
handles `+20/+24/+28`; `0062169B` belongs to `0061FBE0`; `00AB82BC` belongs
to the Text destructor `00AB8250`; `00ABA279` is the wrapped builder's entry;
and `00ABA964` is the content prefix. Each supplies ECX only, without a stack
argument. Exact call/function/native rows are also in the JSON report.

The complete listings were filtered for all ESI/EDI/EBP/EBX occurrences.
Content's ESI is entry ECX throughout, EBP is zero from `00ABA905` through
the covered prefix, EBX captures the main mesh at `00ABA972`, and EDI captures
the section at `00ABA98B`. Clear's ESI is entry ECX, EBP starts at zero and
increments at `00AB8159`, EBX is the byte offset computed at `00AB810E`, and
EDI becomes the post-detach slot at `00AB8144`. Later EBP/EBX reuse in the
unimplemented content tail is not projected backward into the prefix.

## Verification and limits

The source was compiled as MSVC Win32 C++17 with `/W4 /WX /EHsc /permissive-`,
using the integrator's actual detach header and peer buffer/section header. The stronger integrator
`verify_report_calls.py` checks each reported direct instruction and containing
function: 39 direct rows passed with no failures. The Text virtual04 resolution is additionally supported by the
vtable bytes and its complete scalar-deleting body. No new tests were added.
Combined build/registration is the integrator's responsibility.

The new functions are not wired into `bsp_game.exe` yet. Its existing
`GameTextHost` still executes the older separate layout/bridge path, so those
logs cannot demonstrate this prefix, child destruction or nonempty rendering.
No executable-path, game, rendering or ABI-compatibility claim is made.
