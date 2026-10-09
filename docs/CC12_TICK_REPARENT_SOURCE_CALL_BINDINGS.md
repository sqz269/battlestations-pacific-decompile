# Tick reparent Source call bindings

The admitted getter and unlink APIs can compose with the proposed reparent
Source caller if the caller lends the original F/M publication cells and
creates one new actual child node argument word from its already captured
node value. This closes the argument-binding question only. Root owns the
parent implementation, its accepted Native243 contract, cleanup decisions,
compilation and admission. This packet adds no executable Source entry.

## Actual admitted interfaces

| Call | Declaration and placement | Required borrowed storage |
| --- | --- | --- |
| `get_native_pending_registry_00875280(F, M)` | `void* (void* volatile&, void* volatile&)`; current Win32 cdecl | Stable addresses of the actual registry and manager publication cells, in that order |
| `unlink_native_tick_subnode_00875960(receiver, unused_edx, N, F, M)` | `void __fastcall (void*, std::uint32_t, void* volatile&, void* volatile&, void* volatile&)` | ECX receiver, explicit EDX placement word, then addresses of the actual child node-input, registry-publication and manager-publication cells |

The declarations are the current headers. The unlink definition is naked;
it returns through `RET 0Ch` to consume its three reference arguments.
There is no common semantic EAX result. These extra Source arguments and
that return adjustment already differ from Native `RET 4`; they confer no
whole-call Original ABI compatibility.

The EDX formal must be present so the first borrowed reference occupies the
stack location consumed by this implementation. `0u` is a valid explicit
Source placement value. This does not recover a Native EDX argument or
promise propagation of incoming Native EDX. The child does not clear EDX
before its inner raw-leaf call; the intervening getter/imports can replace
volatile EDX. Its actual physical schedule remains authoritative.

## Parent-to-child node value and cell identity

Root's accepted parent contract captures Node once after the outer lock
entry/depth step. Its Native `PUSH ESI` supplies a value copy to the child.
That contract is an input to this packet; no Native parent audit is repeated.

For the new Source composition, initialize one real volatile pointer word
from that captured value and lend its address to the admitted child. A
binding-only sketch, placed at the approved child-call point, is:

```cpp
void* volatile child_node_argument_word = captured_node;
bsp::unlink_native_tick_subnode_00875960(
    actual_unlink_receiver, 0u, child_node_argument_word,
    actual_registry_publication_00f878cc,
    actual_manager_publication_01090aa0);
```

`captured_node` is the parent's already captured value. Evaluating the
original incoming Nodecell again here would select a later value and break
the accepted parent contract. Passing the original incoming Nodecell by
reference would also let the child perform a later read of that different
cell. Neither is the required value-copy composition.

The new word is the child's actual input cell for this invocation. Keep
it alive at a stable address throughout the child call, including all its
getter, lock and leaf activity. Do not replace the child's late dereference
with an early register value or a temporary that expires before the call
completes. A volatile word preserves the admitted Source cell-access form;
it does not by itself establish atomicity or a concurrency policy.

This newly owned Source argument word is distinct from borrowing an
existing incoming Nodecell. It implements the value-copy input within the
new API; it does not reproduce Native stack-slot addresses, mutable frame
aliases, unwind records or spill identity. The child's general requirement
for an actual live input cell still holds. No new private F/M publication
cells are permitted by this composition.

## Current publication values and read timing

Pass the same original F and M references to the parent's getter and to
the unlink child. A local copy of the current F/M pointer values creates
different publication cells, loses later observations and diverts writes.
The getter result is an owner pointer; it is not the F publication cell.

The current getter reads F once on its fast path and returns that capture
when nonnull. Its slow path passes the actual M cell to the manager getter,
captures that manager's raw `+10h` section, optionally enters/increments,
then rechecks current F. After allocation/construction it publishes to F,
performs the second manager lookup through the same M cell, then reloads
current F for registration. After decrement/Leave it reloads F again for
the slow-path return. A single publication snapshot cannot preserve these
distinct observations or the existing failure-cleanup contract.

The unlink child calls that getter with the borrowed F/M cells, then
captures returned owner+4 once as its section. It optionally enters that
section and increments its raw depth before dereferencing N. Thus the new
node argument word remains the child input across all those operations.
After the raw unlink leaf, the child clears current EDI+4 without rereading
N or substituting EAX. Its decrement and Leave use the captured section;
no local unwind cleanup was added by the admitted child.

Let S be entry ESP of the unlink child. Its borrowed argument addresses
occupy S+4=N, S+8=F and S+0Ch=M. Three register saves leave ESP=S-0Ch.
The first `PUSH [ESP+18h]` selects M at S+0Ch; the second selects F at S+8.
The getter therefore receives actual F/M addresses in cdecl order. The
child discards those two temporary arguments with ADD ESP,8. At the later
node step, `[ESP+10h]` selects N at S+4 and the next instruction reads its
current pointer. Normal `RET 0Ch` leaves ESP=S+10h. These Source placements
are not asserted to coincide with any Native frame.

## Runtime storage and lifetime qualification

Current `GameNativeStringProcess` retains the canonical F and M cells.
`game_native_string_process()` keeps the process object behind a static
pointer with no process-object exit destructor. `GameSingletonHost` borrows
the same cells; its F accessor returns its reference and its constructor
installs that same cell's address in the actual pending-registry deletion
binding. Those accessors introduce no publication snapshots.

Production composition requires that canonical cell pair and the matching
host deletion domain. The cell lifetimes extend through process teardown,
but that does not extend the lifetime of the Host or its other contexts.
Keep the matching deletion bindings alive through manager drain. This
packet creates no Host, cell, owner, registration, consumer or startup route.

Cell lifetime also does not establish pointee or raw control backing. The
receiver, captured node, selected neighbors and sections must satisfy the
existing child contracts for each access. Arbitrary raw aliases, faults,
nonlocal exits and the parent's own cleanup remain separately qualified.
No immutable saved-register/control-word guarantee follows from the new
local node word. Existing getter C++ cleanup and the child's lack of local
cleanup are unchanged.

## Current compiled evidence

At the Source109 snapshot on main `806294aa1`, all 109 canonical input pins
and four artifacts matched the normal build ending
`2026-10-09T19:05:05.720846Z`: three existing checks, 30 captured/replayed
objects and 34 unique positive public Core definitions. The two selected
complete object members were copied to ignored local captures before any
later Root build could replace the artifact paths.

The accepted getter object is 3,703 bytes; its public root is 250 bytes /
85 instructions. The accepted unlink object is 1,107 bytes; its public root
is 82 bytes / 29 instructions. This packet replayed all 332 selected root
bytes, all 114 instructions and all fourteen root relocation records,
including their physical indexed symbol records and current positive Core
definitions. The unlink's external getter edge resolves to that admitted
current getter definition. The getter's containing section also has two
previously admitted catch tails; those remain covered by whole-object
identity and are not counted in this selected public-root reread.

The companion report records exact pins, offsets and binding facts. It
uses the preserved snapshot after capture; later build paths may change.
No Native body, handler, caller, PE or Ghidra query, whole-library decode,
new test, fixture, build, probe, CMake, ledger or executable mutation is part
of this packet. Parent compilation and complete integration review remain
Root's responsibility.
