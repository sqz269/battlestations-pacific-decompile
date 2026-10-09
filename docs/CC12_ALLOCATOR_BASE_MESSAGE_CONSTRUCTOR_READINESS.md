# Allocator base-message constructor: complete leaf readiness

The complete **24-byte / seven-instruction** leaf at `00BF638E..00BF63A5`
establishes the exact **12-byte receiver footprint** and the earlier wrapper's
required `RET8`. It writes the profile first, reads one pointer slot, zeros the
last DWORD with a read-modify-write, then publishes the captured middle DWORD.
The numeric second argument is never read. This is readiness evidence for a
small Source leaf; no implementation or ownership policy is added.

Evidence: [machine-readable report](../reports/cc12_allocator_base_message_constructor_readiness.json).
Base: published main `f208c80441c7e9a54b49700cc5eb0ed45a06d3da`, reached by
successful guarded fetch and fast-forward merge. No Source, CMake, ledger or
Ghidra changes, new credit, builds, tests, probes, native execution or Application
wiring are added.

## Complete native evidence

Queries verified the existing `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`, x86 little-endian target at image base `00400000`;
the function count was 64,729. Metadata identified the 24-byte extent before
the full pseudocode, listing and bytes were captured. All bytes match the
installed PE, and all seven live instruction starts match independent decoding.

The saved `exception` library name and nominal
`thiscall exception(this, char**, int)` prototype were preserved. The decompiler's
class/reference labels do not establish message contents, exception ABI or
ownership. This body has no calls, branches, local stack frame, x87 operations,
hidden incoming register argument or incomplete tail. No adjacent bytes,
strings, pointer-slot contents, profiles, RTTI or child bodies were read.

| Site | Instruction | Exact effect |
| --- | --- | --- |
| `00BF638E` | `MOV EAX,ECX` | Preserve the original receiver `H` in EAX. |
| `00BF6390` | `MOV ECX,[ESP+4]` | Capture the first argument as slot address `P`, before any receiver store. |
| `00BF6394` | `MOV DWORD [EAX],00D69370` | Publish the raw profile word at `H+0`. |
| `00BF639A` | `MOV ECX,[ECX]` | Read current DWORD `M` from slot `P`, after profile publication. |
| `00BF639C` | `AND DWORD [EAX+8],0` | Read-modify-write the full DWORD at `H+8`, producing zero. |
| `00BF63A0` | `MOV DWORD [EAX+4],ECX` | Publish captured `M` at `H+4`, after the state-word RMW. |
| `00BF63A3` | `RET8` | Return and discard both four-byte argument slots. |

The three receiver stores cover offsets `0..3`, `8..11`, then `4..7`.
There are **no untouched holes within `0..11`** and no receiver access at or
above offset `0Ch`. The minimum touched span is 12 bytes; this does not prove
the total allocation/derived-object size or create a live C++ exception object.

`AND DWORD,0` is a genuine read-modify-write. In addition to the reached writes,
`H+8..H+11` must support a read. Replacing its evidence with a write-only zero
would discard an actual memory access and its fault/flags behavior.

## Pointer-slot order, ignored argument and partial effects

The leaf reads exactly one DWORD from the caller-supplied slot. It never
dereferences the captured value `M`, measures a string, allocates, copies message
contents, or increments a reference. `M` can be zero or any raw pointer word
without a pointee access in this function. The slot itself and reached receiver
storage must be valid; neither receives a null guard.

The order matters when the slot aliases the receiver. With valid ordinary
accesses, `P = H+0` observes the newly written profile word; `P = H+8` observes
the previous state word before zeroing; `P = H+4` captures the old middle DWORD
and later writes it back. The slot address is loaded before profile publication,
but its value is loaded afterward. Do not replace it with an entry-value
snapshot or pre-copy the message before this function's first store.

The second argument at `[ESP+8]` is never read or used for a branch, store or
ownership decision. Its numeric value—including the wrapper's supplied one—has
no selected semantic effect. `RET8` discards its slot together with the first
argument; this stack cleanup is not a read of that value.

A slot-read fault may occur after `H+0` has changed. The RMW follows the slot
read, and the final `H+4` store follows profile publication and state zeroing.
No local catch, handler, rollback or destructor exists. Concurrent mutation and
asynchronous fault equivalence are not established.

## Register and wrapper composition

EAX retains the original receiver through return; ECX ends as captured `M`.
EDX and the nonvolatile EBP/EBX/ESI/EDI registers are not written. With incoming
`ESP = S`, the return address is at `S`, arguments are at `S+4` and `S+8`, and
`RET8` leaves `ESP = S+0Ch` on return. There are no own pushes or local bytes.

The only flags-writing instruction is the AND. On normal completion it leaves
ZF/PF set, SF/CF/OF clear, and AF undefined; the later MOV and RET preserve those
flags. A future ordinary C++ interface must not claim this flags ABI without
emitted-code proof.

The accepted [25-byte wrapper audit](CC12_ALLOCATOR_FAILURE_OBJECT_CONSTRUCTOR_READINESS.md)
is reused without new caller queries. It passes the same receiver, opaque slot
address `00E154B4`, and numeric one. This leaf's actual `RET8` and lack of ESI
writes close the wrapper's previously qualified normal stack/register dependency.
The wrapper then replaces `H+0` with `00D6923C` and returns the original receiver.

The composed two bodies write 12 unique receiver bytes, with 16 bytes of stores
when counting the repeated profile publication. On normal completion they leave
`H+0 = 00D6923C`, `H+4 = captured slot DWORD`, and `H+8 = 0`. The actual slot,
string and profile contents, static owner lifetime and native exception identity
remain outside this packet. Zero in the last DWORD alone is not a newly
fabricated ownership contract.

## Bounded Source comparison

The private `construct_base` in
[native_legacy_exception_owner.cpp](../src/native_legacy_exception_owner.cpp)
writes `+4 = null`, `+8 = 0`, then `+0 = 00D69370`. Its arguments and order differ
from this leaf, and it does not express the native pointer-slot/RMW schedule.
The related private copy/destructor helpers handle the larger owner's message
and state fields, including conditional host allocation/free. They establish
neither a replacement for this constructor nor new native message ownership.

[NativeLegacyExceptionStorage](../include/bsp/native_legacy_exception_owner.hpp)
is a `0x28`-byte owner with the corresponding leading fields and an SBO member
at `+0Ch`. Matching leading offsets do not permit treating a bare 12-byte region
as a live larger object. Existing public logic/length-error operations also
touch that additional member storage.

A bounded type search additionally found the eight-byte record/context pair
`NativeCrtExceptionPair` and the distinct math-error record
`CameraAxesCrtException`; neither is this three-DWORD owner. The libm header only
forward-declares that math record. These exact snippets are pinned, with no
broader CRT investigation. The exact-address search found no existing `00BF638E`
provider in `src` or `include`. Current `singleton_lifetime_allocate` continues
to throw the host's `std::bad_alloc`, separate from native object/profile identity.

## Source candidate and validation boundary

This leaf now has sufficient body evidence for a separately authorized small
Source packet. It needs actual live storage compatible with the proved 12-byte
extent, the actual four-byte slot address, and the observed read/store order.
It must preserve the real DWORD RMW and original-receiver return, keep the
second value unused, and distinguish its public interface from ECX/stack/RET8.

No larger-owner cast, global/static owner, message allocation, string copy,
reference retention, catch, automatic destructor or exception-type identity is
supplied. Actual owner/slot production and later copy, cleanup, profile and throw
runtime contracts remain separate work.

Nine directly consumed repository files and bounded excerpts match the base
commit. All 49 inherited canonical references over 22 unique paths were replayed.
The complete 24-byte PE/decode, footprint, argument-use and exact two-file diff
checks passed. No compiled graph was rebuilt or re-audited.
