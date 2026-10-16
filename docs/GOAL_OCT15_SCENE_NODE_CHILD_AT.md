# Actual scene-node child-at leaf

Address: `00467F30`.

This packet reconstructs the complete contiguous41B/17-instruction leaf
`[00467F30,00467F59)` against actual storage. Every byte matches live Ghidra
and the installed PE. The existing `scene_node_child_at` in
`engine_seg4_6_9_cc13.cpp` takes captured head/count and a `std::function` next
accessor; that projection is retained for its consumers and is not used here.

| Entry | Coverage | Native interface |
|---|---|---|
| 00467F30 | Complete41B/17ops,no callees | ECX actual list;stack signed index;RET4;raw EAX node bits/0 |

The new ordinary C++ API binds `actual_list` and `actual_index4`. Binding
metadata stays stable and disjoint from reached cells/payload. The index word
is read once before any receiver access. Payload aliases remain admitted where
the reached actual accesses are valid. A negative index can return without any
list storage;an invalid current count can return without any head/node access;
index0 can return arbitrary nonnull head bits without dereferencing that node.
There is no caller preinitialization,whole-container validation,new alignment
guard,owner,lifetime or implicit native/compiler/helper stack alias claim.

The complete schedule is:

1. Capture the current index DWORD. Its sign bit set returns0 and skips count.
2. Capture current count at list+8. Native signed count<=nonnegative index
   returns0,including every negative count. The Source sign-bit/unsigned tests
   preserve this full signed branch without signed-overflow assumptions.
3. Read current head at list+0. Null returns0. If retained index is zero,return
   those captured head bits immediately.
4. Otherwise read current node+44h link with DWORD address wrap,decrement the
   retained index and return0 immediately on a null link. Repeat until zero,
   returning the captured selected node bits. The count is never reread.

This body has no call,callback,SDK,allocator,pool,matrix,EH action or write.
Both return tails at the native end consume4 stacked bytes. It returns borrowed
raw bits;no entity/list class or ownership is recovered from the descriptive
name. Source pointer/integer conversion is deliberately MSVC Win32 only.

The sole current incoming transfer is `00925294 CALL 00467F30`,in
`009251F0`. That full native caller's setup was read:it pushes retained remainder
first,then current index;ChildAt RET4 consumes the index alone. The remaining
word becomes the recursive name argument after the same index word is reused
for the returned child. This is caller evidence only. The parent201B/90op
qualified-name lookup remains NOT ready for general Original-source closure:
its Original CRT search/prefix dependencies remain incomplete. No parent Source
implementation or completion credit is part of this leaf packet.

Whole listing/bytes,typed body/sole true incoming metadata,and existing legacy
Source are reviewed. Descriptive hypothesis and prior comments are preserved
through locked annotation/save/export/readback,with exact receipts in the report.
Source/header hashes identify the draft. Primary full41B/17op assembly and
entire Source/header review passed with hashes asserted unchanged. The other
harness write lock was first observed held;no competing write was attempted.
After it released,the normal locked annotation saved the project,retained the
old name/comment and refreshed/read back the export. The actual combined normal
Win32 build/emitted-object gate remains pending. The
worker has no CMake queue or duplicate build. No new tests are added. Static/
build proof will not establish Original ABI/private frame/fault/game identity.
