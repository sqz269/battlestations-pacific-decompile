# Raw Group construction and scalar deletion

Addresses: `00AC6FA0`, `00AA12F0`, `00AC73E0`; compiler boundaries
`00CB6DE0`, `00CB6DE8`, `00CB6DF0`.

This packet composes the existing actual Group pool, raw widget constructors
and raw widget lifetime provider. It returns actual `F8BFA0` pool slots with
`ECh` payload, `F0h` stride and the existing hidden slab ID at `+EC`. It does
not use `GuiWidgetOwner`, a logical factory, a second allocator or a new
reference count. Names are descriptive hypotheses, not recovered symbols.

| Routine | Exact bytes | Coverage | Original ABI |
| --- | --- | --- | --- |
| `AC6FA0` | `[AC6FA0,AC6FB9)`, 25 | Complete normal adapter; inherits supported base-copy domain | ECX destination, stack source, EAX same destination, RET4 |
| `AA12F0` | `[AA12F0,AA1374)`, 132 | Complete normal wrapper and host C++ unwind projection | ECX optional source, EAX actual result or null, RET |
| `AC73E0` | `[AC73E0,AC7406)`, 38 | Complete normal scalar-destructor schedule with required raw lifetime bindings | ECX owner, stack flags DWORD, EAX saved owner, RET4 |
| `CB6DE0` | `[CB6DE0,CB6DE8)`, 8 | Boundary evidence; first host catch maps its slot-return effect | Compiler frame EBP, tail AC73D0 |
| `CB6DE8` | `[CB6DE8,CB6DF0)`, 8 | Boundary evidence; second host catch maps its slot-return effect | Compiler frame EBP, tail AC73D0 |
| `CB6DF0` | `[CB6DF0,CB6DFA)`, 10 | Unimplemented native FH3 handler | EAX FuncInfo DED36C, tail BF6B43 |

`AC6FA0` loads its source stack word at `AC6FA0`, before saving ESI; pushes
that captured source at `AC6FA5`; and saves the actual destination in ESI at
`AC6FA6`. It calls raw `AA9520` at `AC6FA8`, stamps `D5CB80` only after
successful return (`AC6FAD`), and returns the saved destination. Its own body
does not read source payload fields, copy a count or initialize derived data.
It forwards the caller's `NativeGuiWidgetCopyContext` and
`NativeGuiWidgetCopyAcquired` directly to the existing base-copy provider.
That provider supplies actual Model virtual10 cloning for supported types
`26h`/`3Eh` and parent zero. Other clone domains remain its explicit boundary.

`AA12F0` captures ECX in ESI at `AA1307`, before the allocator call at
`AA130E`. Native ECX request `ECh` is ignored by selector `AC76D0`, which
selects the same actual Group pool. The source uses the explicitly initialized
canonical Group process; startup and application publication are caller duties.
It stores the allocation in the frame local at `AA1315` (`ESP+4`, handler
`EBP-10`) and branches on the saved source. Nonnull source selects copy with
state zero; null source selects raw `AC6F50` with state one. Each path checks
for a null allocation before entering the constructor. Successful return uses
the constructor's EAX. There is no source-type test, output registration,
retain/release, callback, or extra payload initialization in this wrapper.

The host catch begins only after allocation succeeds. Both native unwind map
entries at `DED35C` have next state `-1`: state zero uses `CB6DE0`, state one
uses `CB6DE8`. Each loads the original allocation from `[EBP-10]` and tails
to `AC73D0`. Constructor-local cleanup runs first; the outer wrapper returns
only the failed slot and rethrows. It does not invoke the successful-payload
destructor. A second exception during that cleanup terminates, matching the
existing host unwind policy; this is not a native FH3/SEH implementation.

Copy diagnostics remain in caller-owned storage after failure. In particular,
if a model clone completed before a later base-copy failure, the returned
widget slot must not be used to rediscover that clone. The caller retains the
base provider's acquired record and its cleanup obligations. Default and null
allocation paths leave copy diagnostics untouched. No implicit reset,
registration, companion admission or model rollback is added here.

`AC73E0` saves the actual owner, stamps `D5CB80` at `AC73E3`, and calls
the existing raw `AA9730` at `AC73E9`. Only after it returns does `AC73EE`
read the low byte of the caller's flags DWORD. The public source API therefore
borrows the caller-owned volatile flags slot instead of taking a copied flags
value. The slot may alias actual widget state or be changed by raw destruction
callbacks. If its current bit zero is set, the source calls `AC7260` on the
same `F8BFA0` process pool with the saved pointer. It then returns that pointer
without reading the payload. A base-destructor exception skips both the flags
read and the pool return. Neither original nor source tests/decrements `+04`.

The existing `NativeGuiWidgetLifetimeContext` remains required. It carries
actual node companions and exact current table/class-query/child/timed-entry
dispatch. Its raw `AA9730` implementation handles scene release, child
deletion, parent-list detachment, timed entries, containers and the final base
stamps. Raw `A9BD50` list removal is separately available for its required
binding. This packet adds no successful default implementation for a missing
dispatch and no logical owner projection.

There is no canonical raw Group render-reference companion yet. The existing
Text identity companion depends on a logical owner and cannot serve this raw
slot. Consumers that retain a Group through `NativeRenderActualOwnerRegistry`
must provide one actual-`+04` companion with real current-profile terminal
dispatch and explicit retirement tied to this scalar destructor. Explicit
flags-one deletion may occur with a nonzero count. The factory itself has no
native admission call, so it does not manufacture one or include host registry
failure in constructor rollback. Its normal output carries the native creator
reference; the caller owns later publication and deletion.

The report pins 221 code bytes and 60 vtable/EH data bytes against both live
Ghidra and the installed PE, records all nine direct/tail call sites, and
records the undefined handler's inclusive end and final instruction length.
The worker performs no Ghidra mutation. Strict MSVC Win32 build and all three
existing CTests passed. The compiled object confirms the flags-address/byte
loads at `+16h/+1Ch` after the raw base-destructor call at `+11h`, and no
payload access after pool return at `+31h`. It also confirms the post-copy
Group stamp and the saved-slot return before rethrow. These are offsets in
the new source ABI, not in the original executable. No new runtime probe or
test suite was added for these thin adapters. Native binary ABI, FH3/SEH, application startup/publication,
arbitrary clone types, and gameplay remain unproved.
