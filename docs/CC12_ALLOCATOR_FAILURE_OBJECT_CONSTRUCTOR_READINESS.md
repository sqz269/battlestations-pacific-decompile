# Allocator failure-object constructor: wrapper and base boundary

This read-only packet establishes the complete **25-byte / nine-instruction**
constructor at `00BF6802..00BF681A`. Its owned body makes one child call,
publishes one DWORD at receiver offset zero, and returns the original receiver.
Complete initialization of the proposed 12-byte base remains dependent on the
24-byte child at `00BF638E`, which was inspected only as metadata.

Evidence: [machine-readable report](../reports/cc12_allocator_failure_object_constructor_readiness.json).
Base: published main `2fa724372622e47bb1b10a9feb5c25492e5402a1`.
No Source, Ghidra, ledger or CMake changes, new credit, builds, tests, probes,
native execution or Application wiring are added.

## Evidence boundary

Live queries verified the existing `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86 little-endian program at image base
`00400000`; the function count was 64,729. The full 25-byte body passed the
64-byte gate before pseudocode, listing and byte capture. Every live instruction
start and all bytes match the independent complete decode and installed PE.

The saved library name `bad_alloc` and its `thiscall` prototype were preserved.
The decompiler identifies a `std::bad_alloc` constructor, but its labels do not
prove exact exception ABI, message contents, or thrown-type identity. There is
no x87, hidden incoming register argument, branch, terminal gap or listing repair
in the selected wrapper.

The lease was extended to `00BF638E` before querying its metadata. It is named
`exception`, with nominal signature `exception(this, char**, int)`, spans
`00BF638E..00BF63A5` (**24 bytes**), and reports seven instructions and zero calls.
Its bytes, listing and body were not queried. Static object contents, the
argument slot or string, profiles, RTTI, ThrowInfo, callbacks, handlers, callers
and heap internals were not read.

## Complete constructor schedule

| Site | Instruction | Established effect |
| --- | --- | --- |
| `00BF6802` | `PUSH ESI` | Save incoming ESI for ordinary return. |
| `00BF6803` | `PUSH 1` | Push numeric second child argument; its meaning is unproved. |
| `00BF6805` | `PUSH 00E154B4` | Push an opaque first child argument address. No local dereference occurs. |
| `00BF680A` | `MOV ESI,ECX` | Capture the original receiver; child ECX remains the same receiver. |
| `00BF680C` | `CALL 00BF638E` | Delegate base initialization with those two arguments. |
| `00BF6811` | `MOV DWORD [ESI],00D6923C` | After the child returns, unconditionally publish the raw first DWORD. |
| `00BF6817` | `MOV EAX,ESI` | Return the original receiver, ignoring child EAX as a replacement owner. |
| `00BF6819` | `POP ESI` | Restore saved ESI under the child stack-restoration contract. |
| `00BF681A` | `RET` | Plain return; no constructor stack arguments are popped. |

With incoming `ESP = S`, saved ESI is at `S-4`, numeric argument one at `S-8`,
opaque argument address at `S-0Ch`, and the child return address at `S-10h`.
The child sees its first and second arguments at its incoming `ESP+4` and
`ESP+8`, with the unchanged receiver in ECX.

There is no explicit caller cleanup between the child and `POP ESI; RET`.
Correct normal frame restoration therefore requires the child to consume its
eight argument bytes before returning to `00BF6811`. Its `thiscall` metadata is
consistent with that requirement; an actual child `RET8` has not yet been
verified. The wrapper also relies on ordinary child preservation of ESI, whose
captured value is used after the call.

The wrapper has no receiver null check. On reaching its direct store, the
captured receiver must designate writable storage for at least its first DWORD;
the child may access a wider extent first. There is no selected catch, local
SEH registration, allocation, zero-fill, cleanup or destructor in this body.
No `noexcept` or exceptional frame-preservation behavior is inferred.

## Exactly what is established about the 12-byte base

The owned body directly writes **four bytes at offset zero**, only after the
child returns. It makes no direct receiver reads and no explicit stores at
offsets four or eight. The immediate `00D6923C` is retained as a raw value;
its contents and callability as a host vtable are not established.

The [parent allocator audit](CC12_APPLICATION_ALLOCATOR_POLICY_READINESS.md)
already establishes that this constructor receives `ECX = 0109DD68` after the
guard bit has been set. That parent later reserves `0Ch` local bytes for an
exception-base copy. These facts, together with the related Source's leading
`0Ch` base, identify the intended dependency; they do not prove every store or
the complete footprint of this constructor's uninspected child.

In particular, this packet does **not** establish initialization of offsets
four/eight, ownership of a message pointer, the meaning of numeric argument one,
or that all child accesses stay within 12 bytes. Filling in those facts from
the function name or another constructor would exceed the evidence.

If the constructor/child faults or leaves exceptionally, the parent's guard has
already been set and the previously audited parent has no local reset. This
wrapper adds no rollback. No actual static-object contents or storage lifetime
were read or fabricated.

## Related Source and valid lifetime boundary

[native_legacy_exception_owner.cpp](../src/native_legacy_exception_owner.cpp)
contains three related private helpers:

- `construct_base` corresponds to a different default-base constructor,
  `00BF632F`. It writes a null message, zero ownership, then base profile
  `00D69370`; it does not perform this `00BF638E` call or publish `00D6923C`.
- `copy_base` corresponds to the separately discussed `00BF63A6` operation,
  with nullable host allocation and message-field ownership handling.
- `destroy_base` captures ownership, publishes the base profile, and may free
  the message through the host CRT. This does not establish ownership or
  cleanup for the new constructor.

Their reference type,
[NativeLegacyExceptionStorage](../include/bsp/native_legacy_exception_owner.hpp),
is a real `0x28`-byte Source owner containing the leading three DWORD fields and
an embedded SBO member at `+0Ch`. Public logic/length-error constructors and
copies also write those additional member fields. The private helper signatures
are not callable 12-byte adapters. Casting a 12-byte native region into this
larger owner would invent object lifetime and storage.

Current [singleton_lifetime_allocate](../src/singleton_lifetime.cpp) throws the
host's `std::bad_alloc` when its host new-handler retry fails. That remains a host
service boundary, not a binding to this native constructor or its static receiver.
An exact case-insensitive address search across `src` and `include` found no
provider for `00BF6802`, `00BF638E`, `00E154B4`, `0109DD68`, or `00D6923C`.
This is a bounded Source search, not a claim about unknown external consumers.

Any future callable Source needs actual storage and a live object of the proved
complete base extent, correct child arguments, the observed publication order,
and the original-receiver return. No larger-owner cast, new global, invented
exception type, catch or automatic cleanup is supplied here.

## Next bounded dependency and validation

The next small dependency is the **24-byte / seven-instruction `00BF638E`**
base constructor. Its complete body can resolve direct field stores, argument
slot consumption, numeric-argument meaning, exact raw extent, and the expected
eight-byte stack cleanup. Child-body and pointer/string/profile reads require
their own subsequent scope; none was performed in this packet.

Nine repository inputs and their bounded Source excerpts match the base commit.
All 40 inherited canonical references over 21 unique paths were replayed. The
complete 25-byte PE/decode check and exact two-file diff check passed. Native
exception identity and a valid 12-byte callable Source adapter remain open.
