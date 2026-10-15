# Scene integer-set destructor wrappers

Addresses: 0046B530, 0046C4C0, 0046CE90.

| Native address | Source API | Native span | Coverage |
|---|---|---|---|
| 0046B530 | destroy_native_scene_integer_set_0046b530 | 0046B530..0046B563 | complete52B/23ops |
| 0046C4C0 | destroy_native_scene_integer_set_0046c4c0 | 0046C4C0..0046C4F3 | complete52B/23ops |
| 0046CE90 | destroy_native_scene_integer_set_0046ce90 | 0046CE90..0046CEC3 | complete52B/23ops |

The three complete bodies differ only in same-target relative CALL operands.
Each receives actual tree in ECX, ignores incoming EDX, and has plain RET.
Native EAX is cleared to0; no semantic return type is inferred. Source uses an
ordinary void API with explicit stable frame metadata. Correct STL library names
are retained; the family name follows actual raw providers, not a recovered
class, mapped type or nominal ownership identity.

Each captures current tree.head at+4 then that head's left link before outgoing
argument stores. Five words are stored in native push order: last-node=head,
last-owner=tree, first-node=minimum, first-owner=tree, private-output address.
The complete actual 0046B170 range provider is then invoked and its result is
ignored. After it returns, the wrapper reloads CURRENT tree.head for actual
current-CRT free, then stores tree.head=0 followed by tree.count=0. The captured
old head is not substituted at free, and count is not reset before free. No
guard, rollback, key cleanup, string pool context or nil19h body is introduced.

`NativeSceneIntegerSetDestroyFrame` binds the range's actual20B outgoing cells
and a separate actual8B destructor result cell. That result is not implicitly
the range's own private output8B for partial single erasure. Metadata/bindings
are stable and disjoint from reached payload; deliberate payload/argument/output
aliases remain admitted through raw volatile loads and stores. The range's
actual complete contract controls reached-only child metadata and backing.
No local output initialization or copied-pair snapshot is inserted. Therefore
returning diagnostics and partial paths remain real instead of being replaced
by a full-clear shortcut.

The imported concrete range Source is worker draft `cbb797126`, reviewed at its
actual API and merged into this worktree before this wrapper draft. It uses the
actual accepted nil11h14h-node increment, complete single erase and clear
providers; no stub or invented callback is supplied. Its own validation is
tracked separately. Final Source readiness/build validation waits for the
combined concrete dependency checks. Current-CRT free is accepted
singleton_lifetime_free over std::free; native 00BF65AC is a JMP thunk. Native
CRT/FH3/private-frame/fault identity remains a qualified boundary.

The first two wrappers have no live incoming xrefs. The third has four encoded
EH tail JMP users at00C6204F/5D/CF/DD. They form ECX from the saved enclosing
object plus158h or164h and ignore the destructor result. Typed metadata retains
their existing CALL_RETURN/effective CALL_TERMINATOR state without modifying
these unowned callers. All six native CALL edges and four explicit tail-JMP
edges, containing-function boundaries and caller setup are recorded in JSON.

All156 live native bytes match the installed PE. Complete typed body ranges
are52B each, returning/non-thunk, with no gap or flow repair needed. Guarded
batches verify C:/Users/sqz269/bsp.gpr and program /battlestationspacific.exe,
x86 image base00400000. Owned annotation records prior values, preserves
existing names/comments, takes the write lock, saves and refreshes exports.
Build and existing checks are pending at the reviewable Source draft. No new
routine tests are added. Reached raw backing and releases must remain valid;
compilation/static evidence do not establish direct native differential,
original register/stack/heap/CRT/FH3/SEH binary ABI, application adoption or game
behavior.
