# Pending registry scalar retirement readiness (CC12)

This read-only packet resolves **exactly slot zero at `00D0DEA0`** to native
**`00875850`**, then establishes the complete **55-byte, 15-instruction** scalar
retirement entry. Its ordinary destructor operations are inline; there is no
separate ordinary destructor CALL to expand. The only direct callees are the
already admitted actual-section release `0041CC80` and native free `00BF65AC`.
Their native bodies were not reopened. No table extent is inferred from one slot.

The slot and whole scalar body were freshly read from the verified saved Ghidra
project and compared with the configured installed PE. All **59 bytes** are
file-backed and equal. The saved listing omits `87587E ADD ESP,4`; the physical
bytes and complete independent decode include it. This packet makes no listing,
flow, prototype or Ghidra mutation and assigns no new reconstruction credit.

## Native receiver, flag and return contract

Native entry ECX is the actual raw8 registry receiver, captured in ESI before
any service call. If entry ESP is `S`, `[S]` is the caller return address and
the low byte at `[S+4]` contains the deleting flags. The body tests only **bit 0**;
the other flag bits have no effect here. The callee removes the entire four-byte
argument slot with `RET 4`. There is no receiver null check.

| Native address | Complete ordinary operation |
| --- | --- |
| `875850..875853` | Save caller ESI, capture ECX in ESI, compute ECX = captured receiver + 4. |
| `875856` | Store profile DWORD `D0DEA0` at captured receiver + 0. |
| `87585C` | Call `41CC80` with the **address of the actual owner + 4 slot** in ECX; no stack argument. |
| `875861` | After section release returns, test bit 0 of the current byte `[ESP+8]`, which is `[S+4]`. |
| `875866` | Unconditionally clear the actual publication DWORD `F878CC`. No publication read or identity comparison precedes this store. |
| `875870` | Store base profile DWORD `CE3818` at captured receiver + 0. The two MOV stores preserve TEST's ZF. |
| `875876` | Jump directly to `875881` if bit 0 was clear. |
| `875878..87587E` | Otherwise push captured ESI, CALL `BF65AC`, then `ADD ESP,4` after its ordinary return. |
| `875881..875884` | Set EAX to captured ESI, restore caller ESI and `RET 4`, on both branches. |

Ordinary destruction therefore always occurs, regardless of deleting flags.
With bit 0 clear, the raw8 allocation remains, its profile is `CE3818`, and its
section slot is null after successful nonnull-section release (or was null).
With bit 0 set, the same captured allocation is freed after ordinary destruction.
EAX still contains its original address: this is a returned address value, **not**
evidence that freed storage remains usable. Neither branch returns free's EAX.

At the section CALL, pre-call ESP is `S-4`; the child receives no stack arguments
and returns to that value. The optional free CALL has pre-call ESP `S-8`, with
the receiver as its cdecl argument. Its caller removes four bytes, restoring
`S-4`. POP ESI restores `S`; RET 4 leaves caller ESP `S+8`. ESI is explicitly
preserved. EBX, EDI and EBP are untouched locally and rely on the established
callee preservation contracts. EAX is explicitly the original receiver; ECX,
EDX and EFLAGS are not semantic results. There is no hidden-register input or
x87 instruction in this complete body, and no local EH frame installation.

## Actual section lifecycle

The captured receiver's **own +4 section slot** is distinct from the first
singleton manager's +10h construction guard used by the getter. The accepted
constructor audit establishes that `874BC0` writes `D0DEA0`, obtains the raw
section from `BD1860`, stores its returned pointer to owner +4 after normal
return, and returns the original raw8 receiver. The admitted section creator
allocates a raw malloc-backed **1Ch** object, calls the real
`InitializeCriticalSection`, and sets the physical signed depth at +18h to zero.
Those facts are inherited from pinned evidence, not newly expanded native bodies.

The currently admitted
`release_native_tracked_critical_section_0041cc80(TrackedCriticalSection**)`
provides this corresponding ordinary teardown:

1. Capture the actual slot address and its current section once. A null section
   returns without writing the slot. The slot address itself is not validated.
2. While the section's physical +18h depth is **signed-positive**, decrement it
   before each real `LeaveCriticalSection` call and reread it after that call.
   Zero or negative depth skips this drain.
3. Call real `DeleteCriticalSection` and then the paired source free service on
   that same captured section. Only after free returns, clear the original slot.

The slot is not recaptured to choose which section to free. No alternative
manager, projection or older new/delete convenience section is substituted.
The caller must supply the actual paired section on the owning/quiescent thread.
This is destructive section retirement, not the getter guard's single Leave.

## Captured owner versus current publication

The scalar entry never loads `F878CC`. It releases and optionally frees the
captured ECX receiver even if the global currently points elsewhere, then clears
that global without an identity guard. If a service changes the publication,
the subsequent clear still targets the current actual cell. An alias or
substitute default cell would change this contract.

The getter's accepted ordinary contract publishes constructor return EAX;
after a second manager lookup it registers the **then-current `F878CC`**; after
releasing the first captured manager section it returns a later publication
reload. These addresses can differ under reentrancy. The scalar's captured
receiver is not replaced with any of those current publication values.
No manager lookup, registration-vector erase or publication lookup is present
in the 55-byte scalar body. Manager teardown ordering and caller ownership are
outside this packet; a separate manager operation may already remove an entry.

## Existing Source services and remaining production binding

The existing declarations in namespace `bsp` are:

```cpp
void __fastcall release_native_tracked_critical_section_0041cc80(
    TrackedCriticalSection** actual_owner_slot);
void cleanup_native_pending_registry_base_008748f0(
    void* actual_receiver,
    void* volatile& actual_registry_publication_00f878cc);
void destroy_native_generic_singleton_base_00412430(void* actual_receiver) noexcept;
void singleton_lifetime_free(void*) noexcept;
```

The release definition additionally uses `__declspec(naked)`. Its declaration
and the ordinary base cleanup declaration do not carry `noexcept`. The latter's
body contains only the volatile cell clear and a call to the `noexcept` generic
profile helper; it introduces no explicit throw, catch or compensating cleanup.
Composing it still adds a C++ call boundary absent from the native inline stores;
its potentially throwing function type does not establish an actual throw or
original unwind/fault compatibility.

The current base cleanup API is already admitted by the primary review:
`cleanup_native_pending_registry_base_008748f0(actual_receiver, actual_cell)`
first clears the borrowed `void* volatile&` cell, then calls the existing generic
base helper to stamp `CE3818`. Its whole emitted leaf, actual child and positive
core archive linkage were reviewed and the normal MSVC Win32 build and three
existing checks passed in that prior packet. The four relevant helper source
and header hashes still equal that primary review. No new build is claimed here.

That API can express the scalar's ordered **clear publication / reset base**
effect after actual-section release. Native `875850` does not call `8748F0`:
the two stores are inline. Reusing an ordinary C++ helper adds a source call
boundary and does not preserve the native TEST/store/JZ instruction arrangement,
stack/register ABI, fault behavior or instruction addresses. Source reuse must
retain the actual receiver and actual publication binding, not create a cell.

`singleton_lifetime_free` calls real host `std::free`, paired with the existing
malloc/new-handler/retry allocator. Native `BF65AC` remains the separately known
game CRT free boundary. Host allocation pairing is available; original heap,
CRT, exception identity and hardware-fault equivalence are not established.

A scoped `include/bsp` / `src` search currently finds `F878CC` only in the
ordinary base cleanup API and finds no `D0DEA0` or `875850` implementation.
The complete slot-zero/retirement contract is now established, but this packet
does not implement or admit a scalar Source API, constructor/getter, a genuine
publication producer, or callable profile delivery. Numeric native profile
stamps alone are not callable Source tables. No other slot, caller, bootstrap,
handler or native descendant body was queried. Existing constructor/getter EH
audits remain separate evidence and do not prove original FH3/fault execution.

If section release exits nonlocally, the later flag test, publication clear,
base reset and optional owner free have not been reached by this ordinary path.
No compensating action is inferred. The scalar has no local handler installation;
exception propagation or behavior of upstream handlers is not established here.

## Validation and scope

The [audit report](../reports/cc12_pending_registry_scalar_retirement_readiness.json)
records the four-byte slot, complete native bytes and decode, stack accounting,
callee metadata, current source pins and inherited evidence status. The project
is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 image base
`00400000`; the whole configured PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Both byte comparisons, exact branch/call targets, all 15 instruction boundaries,
the complete stack cleanup and source pin checks pass. Only this document and
the report are committed. No C++/CMake/ledger changes, builds, new tests, probes,
original execution, ABI compatibility, runtime or game validation are claimed.
