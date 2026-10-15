# Actual scene integer-set node allocation

This packet reconstructs the whole57-byte,18-instruction `004681B0` body over
current raw argument cells and the existing concrete current-CRT allocator.
There was no indexed Source for this leaf; its existing descriptive STL name
and empty plate comment are preserved before adding evidence.

| Routine | Native range | Coverage | Original ABI | C++ symbol |
| --- | --- | --- | --- | --- |
| `004681B0` | `[004681B0,004681E9)` | complete | No register input; five stacked words left,parent,right,key-pointer,color; RET14; EAX allocation | `allocate_native_scene_integer_set_node_004681b0` |

The explicit `actual_parameters20` view binds the five current DWORD cells at
0/4/8/C/10. Capturing that view's address does not read any input cell. Native
`00BF681B(14h)` runs first. Only after a nonnull allocation returns does Source
follow this exact schedule:

1. Read left and parent; write node.left0.
2. Read right; write node.parent4.
3. Read key-pointer; write node.right8.
4. Dereference the captured key-pointer; read the current color LOW byte.
5. Write keyC, color10 and nil11=0, leaving padding12/13 untouched.

The current argument cells are not snapshotted before allocation or copied into
a whole-node aggregate. Reads interleave with stores, preserving valid payload
aliases. Key-pointer capture precedes right-link publication, its dereference
follows that publication, and the color LOW byte is captured before keyC is
written. Upper color-word bytes are never read. The full null-result branch
skips all input reads and stores and returns the original allocation value.

The existing `singleton_lifetime_allocate` service implements the genuine
observed current malloc, `_callnewh(size)` retry and `std::bad_alloc` schedule.
It is nonnull-or-throw with both native and host size14h. Preserving the native
conditional branch does not add a nullable allocation stub or recovery policy.
This leaf has no EH registration, placement call, catch, free, local cleanup or
rollback. Current CRT handler identity and private exception representation
remain qualified host boundaries.

The one live caller is the insertion/link body `0046A150`, at `0046A1D4`.
Its pushes provide color0, key-pointer, current head as right, parent, and current
head as left, yielding the native stacked left/parent/right/key-pointer/color
order. The callee consumes all five words via `RET14`. The caller's own length
check and throw scope belong to that enclosing routine and are not invented in
this allocation leaf. Its consumer chain includes unique insertion `0046AC50`
and the scene database's integer-key sets at +158h/+164h; no nominal enum or
class-map meaning is assigned to keyC.

| Containing function | Site | Native callee | Established contract |
| --- | --- | --- | --- |
| `004681B0` | `004681B2` | `00BF681B` | Stack size14h, EAX allocation; caller ADD ESP,4 at `004681B7`; real nonnull-or-throw allocator. |
| `0046A150` | `0046A1D4` | `004681B0` | Five stacked current argument copies; EAX node; callee RET14. |

The complete candidate assembly, real allocator body/API and caller setup were
inspected. Live query batches verify configured `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`; complete original bytes are compared to installed
PE. Prior annotations are preserved under the Ghidra write lock before save
and refreshed exports. After the foreign registry lease released, the worker
removed only its own queue and appended the unique CMake source registration
without an exclusive registry lease. Strict MSVC Win32 build is pending.
No new routine tests, candidate-specific original-byte differential, private
physical-stack/CRT/FH3/SEH ABI, startup execution or gameplay proof are claimed.
