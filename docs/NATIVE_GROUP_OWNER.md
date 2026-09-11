# Native cGroup owner and lifetime

This packet reconstructs the lifetime and selected installed dispatch of native
`cGroup`, vtable `00D634F8`, on the same actual 18Ch allocation used by the raw
constructor. `NativeGroupOwner` retains one `NativeNodeStorage`, one
`NativeNodeBinding`, one scene association and one attachment association. The
pool's trailing DWORD at 188h is never a group field. The native string `cGroup`
is observed at `00D634F0`; descriptive function names remain hypotheses.

The reference companion borrows the real +04 count and uses the existing
`GeneratedModelLifetimeRuntime`. Raw attachment pointer/count/capacity remain
at +178/+17C/+180. `NativeAttachmentArrayView` makes the existing unregister
operation use that same descriptor. Its legacy vector remains empty for this
owner; other diagnostic bindings retain their existing vector behavior.

## Recovered behavior and original ABI

| Address | Behavior | Original ABI |
|---|---|---|
| 00B8F680 | Install group table; resize attached count to zero without clearing any member's +A0; free the current array; call complete node destructor B6F440 | ECX group, RET |
| 00B8F8C0 | Direct destructor, return actual slot to canonical pool010902F4 iff flags bit0, return original address | ECX group, stack flags, EAX original slot, RET4 |
| 00B8EEC0 | Pop attached nodes from the end, decrement count before unconditionally clearing each +A0, then tail B6F310 | ECX group, no stack args, final JMP to shared logical release |
| 00B8F650 | Compare current three descriptor IDs0109032C/330/334 in order, no lazy initialization | stack token, AL boolean, RET4; incoming ECX unused |
| 00B8E620 | Read current own ID0109032C | EAX DWORD, RET |
| 00B8F590 | Set shared guard010902E1 before initialization; write target+0C name; initialize shared node descriptor; copy node/root IDs individually; consume shared counter ID | ECX target descriptor, RET |
| 00B8E6B0 | Clear group +138 bits10/20, with no notification callback | ECX group, RET |
| 00B6DBC0 | Clear +138 bits04/08/10/20, then tail current enclosing +A0 virtual3C if present | ECX node, RET or tail JMP; no initiating-node parameter |
| 0059E5E0 | Clamp requested capacity to at least1; allocate larger pointer array, copy live count, free old backing, then publish pointer/capacity | ECX pointer/count/capacity descriptor, stack signed capacity, RET4 |
| 0059FCE0 | Reserve if needed, initialize added pointer slots null, decrement excess count, finally publish count | ECX descriptor, stack signed count, RET4 |
| 00B8F4C0 | If node+A0 matches actual owner, remove first equal pointer by swap-last and clear+A0 even when no pointer matched | ECX owner, stack node, RET4 |

The generic array helpers receive the containing typed tail in this new C++
interface; native ECX points four bytes farther on, at descriptor+178. Capacity
growth in the actual append callback follows B8F460's double/minimum1 rule. It
uses real `singleton_lifetime_allocate` new-handler allocation and matching CRT
free. Malformed/overflowing descriptor extents are explicit adapter errors.

Virtual18 empties backlinks on **every** invocation, before shared B6F310's
released-byte gate. It does not release attached nodes merely because they
appear in this borrowed array. Child hierarchy release still belongs to the
shared B6F310 traversal. Direct group destruction deliberately does not perform
virtual18's backlink clears. Bytes174,176/177, scalar184 and the freed array's
pointer/capacity words are not reset by direct group destruction.

Group virtual1C routes to the separately reconstructed B8F4F0, preserving
assignment-before-registration and no child recursion. Current virtual50/54
route to B6ED80/B6EE10 through the same scene runtime. Current virtual40 is
B8E6B0; it must not use the node/camera B6DBE0 callback. The base destructor
publishes node-phase dispatch before its reentrant cleanup. The wrappers check
the supplied current native group/node tables; unsupported profiles have no
fallback. Other group slots, including cloning/render/bounds-generation entries,
are not implemented by this owner packet.

## Ghidra and instruction boundaries

Every live query/export used `bsp.py ghidra`, whose client verifies project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. No Ghidra mutation
was made. Defined-function exports remain ignored under `exports/bsp`.
Undefined entries were inspected with verified live bytes and disk disassembly.

| Entry | Last instruction | Length | End exclusive | Analysis defect |
|---|---|---:|---|---|
| 00B8F680 | 00B8F6E5 RET | 1 | 00B8F6E6 | `_free` at00B8F6BE falsely ends body; fallthrough00B8F6C3 reaches B6F440 |
| 00B8F8C0 | 00B8F8DD RET4 | 3 | 00B8F8E0 | None observed |
| 00B8EEC0 | 00B8EF00 JMP00B6F310 | 5 | 00B8EF05 | Undefined function entry |
| 00B8F650 | 00B8F675 RET4 | 3 | 00B8F678 | Undefined function entry; alternate RET4 at00B8F670 |
| 00B8E6B0 | 00B8E6B7 RET | 1 | 00B8E6B8 | Undefined function entry |
| 00B8E620 | 00B8E625 RET | 1 | 00B8E626 | Undefined function entry |
| 00B8F590 | 00B8F5D6 RET | 1 | 00B8F5D7 | Undefined function entry |
| 00B6DBC0 | 00B6DBD8 RET | 1 | 00B6DBD9 | Undefined function entry; alternate tail JMP at00B6DBD6 |
| 0059E5E0 | 0059E63C RET4 | 3 | 0059E63F | `_free` at0059E62C hides fallthrough0059E631 and pointer/capacity publication |
| 0059FCE0 | 0059FD2D RET4 | 3 | 0059FD30 | None observed |
| 00B8F4C0 | 00B8F4E8 RET4 | 3 | 00B8F4EB | None observed |

B8F680 EH metadata is `00DFC1C0`, one unwind state at `00DFC1B8` targeting
`00CC2AF0`: load original owner then jump B6F440. Normal flow marks state-1
before calling the base. The implementation does not repeat base destruction
if that base throws. The native array shrinking/free operation does not invoke
borrowed-node destructors. Native EH execution itself is not fixture-tested.

## Validation and integration

`scripts/build.ps1` passed MSVC Win32 Release and both existing CTests after
`ghidra_export.py verify-seeds` confirmed native seed/disk parity. No permanent
test cases were added. The ignored `local/native_group_owner_probe.cpp` passed:

- Original B8F650 instructions matched the C++ predicate for five tokens. Only
  the descriptor begin/end addresses were relocated to explicit fixture storage.
- Original B8EEC0 and B8F680 prefixes demonstrated the distinct backlink
  cleanup, including mismatched +A0. B8EEC0's tail and B8F680's base destructor
  were observation adapters; B8F680's resize/free calls used host adapters.
- Host execution used the real reconstructed node base and group pool for
  direct flags0 destruction plus explicit pool return, and reference-triggered
  flags1 deletion. Raw-array growth/swap, notification masks, type bootstrap,
  repeated logical release and final companion retirement were checked.

Logs: `local/native-group-owner-build.log`, `local/native-group-owner-probe.log`.
Original-byte manifest: `local/native-group-owner-inputs.json`. These local
fixtures do not establish original full group/base/pool differential parity,
native EH parity, binary replacement compatibility or game validation.

The build used raw constructor commit e3cd51f (local cherry-pick b69302d) and
the final pool source/header from708de517, temporarily overlaid for validation.
Required services are the canonical pool010902F4, node destruction/string/scene
and attachment runtimes, shared type counter and node descriptors, actual
group/node table views and constants, and terminal companion retirement. The
parenting integrator must route B8F460 append through
`append_generated_model_attachment`; raw identity can resolve through the
existing attachment runtime to `native_array.context`, the canonical owner.
The final GUI dispatcher must supply current virtual1C/3C routing. No fake
allocator, copied backlink list, fallback type IDs or no-op lifetime is supplied.
