# Native pending registry scalar retirement Source candidate (CC12)

This packet adds the bounded ordinary C++ body
`bsp::retire_native_pending_registry_scalar_00875850`. It expresses the accepted
complete native `00875850..00875886` retirement schedule using the actual
existing section release, base cleanup and paired free services. **Primary admission has passed the normal build and complete object/Core
review; see the admission section below.** The primary integrator
owns CMake registration, the normal MSVC Win32 build, whole emitted function and
actual callee review, and positive core archive membership verification.

## Borrowed inputs and operation order

```cpp
void* retire_native_pending_registry_scalar_00875850(
    void* actual_receiver,
    const volatile std::uint8_t& actual_deleting_flags_byte,
    void* volatile& actual_publication_00f878cc);
```

All three inputs borrow genuine caller-owned storage. The receiver is the
actual raw8 registry allocation; its +4 slot contains the actual paired tracked
section or null. The flags reference names the real caller flags byte, and the
publication reference names the actual mutable `F878CC` binding. The function
creates no input storage, supplies no defaults, validates no references and
extends no lifetime; captured bit 0 controls disposal of the actual receiver.

The body has this order:

1. Capture the receiver address, then write the volatile profile DWORD `D0DEA0`
   at +0 before entering the section release service.
2. Call `release_native_tracked_critical_section_0041cc80` on the **address of
   that captured receiver's actual +4 slot**.
3. After the service returns, read the borrowed volatile flags byte once and
   capture bit 0 in a local Boolean, before calling base cleanup.
4. Call `cleanup_native_pending_registry_base_008748f0` with the same captured
   receiver and actual publication reference. This concrete helper clears the
   publication first, then calls the generic base helper to stamp `CE3818`.
5. If captured bit 0 was set, call `singleton_lifetime_free` on the captured
   receiver. Return that receiver's address value on both paths. There is no
   dereference after the optional free.

The borrowed flags reference is deliberate: an early by-value copy would lose
the native late read if a service changes the real flag storage before returning.
The Boolean capture is also deliberate: later publication/base stores do not
cause the decision to reload an aliased flags byte. Other flag bits are ignored.
These are C++ sequencing and volatile access statements; emitted instruction
order and register/stack preservation still require the primary object review.

The function never reads the publication to choose an owner. It releases the
captured receiver's section, clears the actual publication unconditionally even
if that cell currently names another owner, then optionally frees the receiver.
The actual
section slot is not the first singleton manager's +10h getter guard. It is not
an older typed manager/projection or a same-layout substitute.

## Concrete existing services

The direct callees and relevant child declaration in namespace `bsp` are:

```cpp
void __fastcall release_native_tracked_critical_section_0041cc80(
    TrackedCriticalSection** actual_owner_slot);
void cleanup_native_pending_registry_base_008748f0(
    void* actual_receiver,
    void* volatile& actual_registry_publication_00f878cc);
void destroy_native_generic_singleton_base_00412430(void* actual_receiver) noexcept;
void singleton_lifetime_free(void*) noexcept;
```

The release definition is additionally `__declspec(naked)`. It captures the
actual slot and current section once, leaves a null slot value untouched, drains
signed-positive physical depth at +18h with real `LeaveCriticalSection`, calls
real `DeleteCriticalSection` and the paired free service on the captured section,
then clears the original slot after free returns. It requires the actual
malloc-backed 1Ch section and an owning/quiescent teardown context.

The ordinary base cleanup's clear-before-profile effect is already admitted by
the prior primary report, including its actual generic helper and positive core
archive linkage. Its declaration has no `noexcept`; its body has the volatile
cell clear followed by the concrete `noexcept` profile helper, with no explicit
throw, catch or compensating cleanup. The new candidate also has no `noexcept`.
No RAII, guard, catch, validation branch or additional manager operation is added.

`singleton_lifetime_free` uses real host `std::free`, paired with the admitted
malloc/new-handler/retry allocator. This does not make the game's original CRT
or heap interchangeable with the host service. The exact direct provider files,
generic child files and accepted reports are pinned in the candidate report.

## Native evidence and interface qualifications

The accepted [scalar retirement audit](CC12_PENDING_REGISTRY_SCALAR_RETIREMENT_READINESS.md)
establishes exactly one profile slot, `D0DEA0[0] -> 875850`, and all **55 bytes /
15 instructions** of the scalar body. Its four-byte slot plus whole body matched
Ghidra and the configured PE. Native ordinary retirement is inline. The saved
listing omits `87587E ADD ESP,4`, while the physical bytes establish that returning
continuation. This Source packet reuses that accepted audit without fresh native
queries, child-body expansion, annotation or listing changes.

Native ECX carries the receiver; the current low byte of `[entryESP+4]` supplies
flags after section release; TEST sets ZF before the unconditional publication
and base MOV stores; JZ skips only the optional free. Native returns the captured
receiver in EAX, preserves ESI and uses `RET 4` to consume the four-byte argument.

This new C++ interface uses an ordinary cdecl call with an explicit receiver and
two reference arguments. The borrowed flags address is not the original by-value
four-byte flags stack slot, and the borrowed publication reference is not itself
proof of a genuine producer of `F878CC`. There is no native ECX or RET4 entry
adapter. Reuse of base cleanup adds ordinary C++ call boundaries absent from the
native inline TEST/MOV/MOV/JZ sequence, including its concrete generic child call.
Potentially throwing declarations do not prove an actual throw or original
unwind/fault equivalence. The candidate contains no local exception policy.

Reference sequencing preserves a late flags read and the pre-cleanup decision
at this interface. It does not establish arbitrary invalid-storage aliasing,
object-lifetime violations, races, hardware-fault delivery or exact native
instruction/return-PC identity. The caller must retain valid genuine borrowed
flags and publication storage through their respective accesses and supply the
actual paired receiver/section allocation. If a child exits nonlocally, this
body adds no compensation for ordinary operations not yet reached.

The numeric `D0DEA0` and `CE3818` stores retain native profile identities only;
they do not deliver callable Source tables. The candidate does not create an
owner, constructor/getter, publication producer, profile table or manager
retirement integration. Returning the original address after free conveys no
right to access that released storage. Native scalar ABI, original FH3, fault,
CRT/heap, runtime and gameplay equivalence remain unproved.

## Worker validation and primary gate

The [candidate report](../reports/cc12_native_pending_registry_scalar_retirement_source.json)
pins both new source files, this document, the accepted native audit and exact
existing service dependencies. It records the source schedule and borrowed
input contract, validates JSON and pins, and keeps candidate status explicit.

At worker handoff, only four owned source/document/report files had changed. No CMake,
ledger or Ghidra edits, new tests or ad hoc probes are part of this packet. No
worker build or execution claim is made. The primary admission below completes registration, the normal build, whole
function/callee review and unique Core verification. Native ABI, runtime and
game-validation credit remain zero.


## Primary admission

Primary review now admits the bounded ordinary Source body. The normal MSVC
Win32 build passed all three existing checks. Its entire emitted function is
62 bytes / 24 instructions. Profile store precedes the actual section CALL;
one volatile byte load into BL occurs after that return and before the actual
base-cleanup CALL. BL survives that helper, and a bit-zero test selects the
actual free CALL. EAX returns the captured receiver without a post-free load.

The three REL32 operands at offsets 18, 32 and 49 resolve through physical
symbol records to the actual 64-byte section helper, 25-byte base helper and
six-byte host-free forwarding leaf. The base helper reaches the actual 14-byte
profile helper. All five complete functions total 171 bytes / 65 instructions;
their whole Core members and positive unique definitions are verified. Real
Win32 Leave/Delete and host CRT free remain qualified external domains.

This adds one ordinary reconstruction covering 55 Original bytes. It supplies
no Native entry adapter, genuine publication/owner producer, callable profile,
constructor/getter/manager integration or gameplay proof. The root is absent
from the game map. Existing LNK4006 spawn-request duplication remains unrelated.
The primary report supersedes the worker's historical pending admission status.
