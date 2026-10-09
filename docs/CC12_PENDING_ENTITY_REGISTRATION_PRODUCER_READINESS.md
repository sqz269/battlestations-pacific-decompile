# Registration producer `00875890`

This read-only packet captures the complete **139-byte / 42-instruction**
ordinary body through the `RET8` at `00875918`. It has **three physical CALLs**:
external `875280`, imported `EnterCriticalSection`, and imported
`LeaveCriticalSection`. Three fresh Ghidra/PE windows total **167 bytes**,
including the late float operand and base-table data. No Source is implemented.

The complete node-store/publication schedule is established. A production
constructor binding remains held on the actual `875280` service, common
pending owner and callable Source base profile. The group bootstrap/common
owner blocker from the [splice audit](CC12_PENDING_ENTITY_REGISTRATION_SPLICE_READINESS.md)
remains independent; this producer neither initializes nor accesses the groups.

## Inputs, preserved bytes and exact writes

Native ECX is the actual 34h-byte node. The first stack DWORD is the payload
pointer; the second is the raw group DWORD. The constructor returns that same
node in EAX, pops eight argument bytes, and preserves EBX/ESI/EDI. The body has
no node allocation/free operation; the getter's internal effects remain
external. It validates neither the node, payload nor group. Group bits
are copied unchanged; a caller's signed interpretation adds no range check.

| Phase | Native stores and boundaries, in order |
| --- | --- |
| Before the getter, `87589E..8758B7` | Node `+0 = D0DEC8`; `+14 = raw group`; byte `+18 = 0`; byte `+19 = 0`; DWORD `+20 = 0`; DWORD `+1C = 0`; DWORD `+24 = 0`; `+28 = payload`. The +20 store precedes +1C. |
| Getter/capture, `8758BA..8758C2` | Call `875280`; capture its returned owner's `+4` section pointer exactly once into EDI. The owner itself has no null guard. At this callsite ECX still contains payload and EAX still contains the raw group bits; the excluded getter's complete input contract is not inferred from Ghidra's empty prototype. |
| Nonnull section, `8758C6..8758D0` | Call real `EnterCriticalSection(section)` through IAT `CE2218`, then add1 to the DWORD at captured section `+18`. A null captured section skips both operations and still performs the node append. |
| Append, `8758D3..8758F3` | Read pending tail.prev at `E0B708` and store into node.prev `+4`; set node.next `+8` to actual tail `E0B704`; **reload** `E0B708`, write that old last node's `+8 = node`, then publish `E0B708 = node`. `XORPS XMM0,XMM0` also occurs before these stores. |
| Late fields, `8758F4..875905` | After publication, MOVSS positive-zero bits into node `+2C`; freshly MOVSS-load `[D7A260]`, then MOVSS-store those exact bits into node `+30`. Current operand bits are `BF800000` (-1.0f). This is bit-preserving movement, not an x87 evaluation. |
| Release/return, `875906..87591A` | If the captured section was nonnull, add-1 to its DWORD `+18`, then call real `LeaveCriticalSection(section)` through IAT `CE2210`. Restore registers, return the actual node, and `RET8`. No section or owner re-fetch occurs. |

The constructor writes **42 of the node's52 bytes**. It preserves the ten
bytes at `+0C..+13` and `+1A..+1B`; no whole-object zeroing is justified.
Before append, prev/next still retain their incoming bytes. Before the late
stores, `+2C/+30` likewise retain their incoming bytes. The payload pointer is
borrowed and stored, never dereferenced or retained by a refcount here.

The initial lock comparison at `8758D1` remains in EFLAGS through all append
and MOVSS/XORPS instructions and controls the release branch at `875906`.
The depth arithmetic is raw 32-bit ADD with wrap, not a saturating count or
checked signed increment. A future Source helper must preserve that behavior
without relying on C++ signed-overflow semantics.

## Publication, alias and failure boundaries

The node becomes reachable through the old last node before tail.prev is
updated, and both publications precede the final two float stores. A helper
which initializes every field first and only then links the node changes this
schedule. The append does not distinguish an empty list: old_last.next is the
head's next field only when old_last is the head sentinel. It is not an
unconditional direct store to `E0B6D8`. The captured 33-byte append span is
identical to the preceding splice audit's evidence.

There is no Native EH frame, local catch, ownership guard or rollback in this
body. If the external getter fails, the pre-getter stores already happened.
After the critical section is entered, invalid-memory/OS-fault behavior is
not replaced by automatic unlock, unlink or node destruction. The body's
ordinary leave decrements depth **before** calling Windows. No exception/SEH
compatibility is admitted from an ordinary C++ helper.

The actual owner returned by `875280` and its captured section have distinct
requirements: the owner must be readable for the one `+4` load, and a nonnull
section must remain live and initialized through the matching leave. The
pending tail and current old-last nodes must remain live and writable. A
different pending/observer lock with the same layout is not the same service.
The producer's lock also does not establish a lock in the splice consumer,
which performs no such call; their phase relationship remains a caller contract.

## Current Source reuse and minimum binding boundary

`TrackedCriticalSection` already has actual Win32 `CRITICAL_SECTION` at `+0`
and the depth DWORD at `+18`, with size1Ch assertions. `NativeObserverLockOwner`
documents the raw8 owner shape with section at `+4`. These establish reusable
storage shapes, not identity equivalence with the unknown `875280` owner.
The actual Enter/Leave imports are available as real Windows operations.

The pending-entity producer's private `CapturedPendingSection` uses a wrapping
depth update and real APIs, but its destructor adds C++ unwind cleanup. It is
not a drop-in contract for this body, which has no such cleanup frame. The
random-thread helper also has a private RAII guard and is not a shared registry
owner. Neither helper's lock may be silently substituted for `875280`.

`GameNativeReadOnlyData` can expose the actual read-only float at `D7A260`
when its band is retained. The table at `D0DEC8` contains six original code
addresses (`875920`, `42BB70`, `42BB80`, `42BB90`, `42BBA0`, `42BBB0`);
its targets were not expanded. Copying that numeric table identity is not
publishing a callable Source profile. `tick_element_overrides.cpp` contains
address catalogs; its `Complete` classification means overriding slot bodies
were read, not that this constructor or a callable Source table is supplied.

The smallest eventual producer would take actual node/payload/raw-group
inputs and borrow only the actual pending **tail object** (whose own `+4`
must be the producer/consumer's common last cell), current operand data, a
qualified callable base profile, and the actual getter/section service.
Passing tail and an unrelated last-cell snapshot as independent substitutes
would admit an invalid binding. It must leave the ten unobserved bytes alone
and must not own allocation, queues, payloads, profiles or lock teardown.

This is a **required binding boundary, not an admitted Source API**. No
current concrete `875280` Source service, common registry owner or callable
base profile was demonstrated, so no default getter, synthetic owner or
constructor implementation packet is recommended yet. Establish those
ordinary-body dependencies first. Keep the five-group initialization,
producer/consumer phase and all subsequent wave/virtual lifetimes separate;
even a qualified constructor would not close them.

The [report](../reports/cc12_pending_entity_registration_producer_readiness.json)
pins all42 body instructions, inputs/stores/imports, inherited append/bootstrap
evidence and current Source inputs. No getter descendants, nine callers or
group bootstrap were swept; no Source/Ghidra/ledger/CMake change, flow repair,
build, test, probe, new Original credit or runtime admission occurred.
