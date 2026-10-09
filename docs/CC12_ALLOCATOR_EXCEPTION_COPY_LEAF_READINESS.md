# Allocator exception-copy leaf readiness

The complete owned Native function is `00BF63A6..00BF63FD`: **88 bytes and
38 instructions**. Live Ghidra metadata, the complete listing and the unchanged
configured PE agree. Its last instruction is the three-byte `RET 4` at
`00BF63FB..00BF63FD`. The existing private Source helper's comment ends at
`00BF63FE`, a range of 89 bytes. That endpoint is one byte too far; no byte at
`00BF63FE` is selected or interpreted by this packet.

The existing helper also has a substantive contract gap: the Native
nonzero-control/null-message path uses `AND DWORD [destination+4],0`, a real
read-modify-write. The Source helper expresses a volatile null pointer store.
That does not explicitly preserve the Native prior-value read, access/fault
requirement or instruction flags. This is a Source-level comparison; no
compiler emission of that helper is inspected or claimed to differ.

This is read-only readiness evidence. No Source, CMake, ledger or Ghidra change,
build, probe, test or admission credit is made. The correct saved library name
`exception` and its prototype are preserved.

## Scope and target

Published base: `0bfa628055f152f329bdc62c6b2ae1fdfe678b24`. It contains the
worker's failure-object wrapper commit and the primary's registered admission.
The prior 24-byte base-message leaf and 25-byte wrapper are not counted again.
The accepted allocator-policy report supplies the motivating static-to-local
12-byte copy context; this packet does not query that allocator body again.

All live queries use `bsp.py ghidra`, which verifies existing project `bsp`,
program `/battlestationspacific.exe`, language `x86:LE:32:default` and image
base `00400000`. The configured project file is `C:/Users/sqz269/bsp.gpr`.
The initial live function count is 64729. Only the owned 88-byte body is read.
The three direct children are leased for metadata-only queries; their bodies
and all static owner/slot/string/profile/RTTI/ThrowInfo/handler/caller data are
outside this packet. No hidden incoming register, x87 expression, missing
instruction or flow-repair issue is present in the complete owned body.

## Entry, fields and access order

Let `D` be the incoming ECX destination and `S` the pointer in the first stack
argument. At entry ESP=`T`, `PUSH EBX` makes `[ESP+8]` refer to original
`[T+4]`, which is captured in EBX. ESI captures `D`. EBX, ESI and EDI are saved
and restored on the normal return; EAX returns the original destination.
`RET 4` consumes the one explicit source argument. No extra hidden register
input or local exception/unwind frame is selected by the body.

The following are actual four-byte accesses, with no inferred larger object:

| Order | Site | Access |
| --- | --- | --- |
| 1 | `00BF63AE` | Write raw `00D69370` at `D+0` |
| 2 | `00BF63B4` | Read the full control DWORD at `S+8` |
| 3 | `00BF63B7` | Write that unchanged DWORD at `D+8` |
| 4 | `00BF63BC` | Read the message pointer DWORD at `S+4` |
| 5 | Path dependent | Write or read-modify-write the DWORD at `D+4` |
| 6 | Allocation-success path only | Reload `S+4` after publishing the allocation at `D+4` |

The control test uses the entire DWORD, not a byte or a single ownership bit.
The copied value is not normalized to boolean. `S+0` is not read. The leaf's
own destination accesses fit bytes 0 through 11 and cover all three DWORDs
on a normal return. Pointer pointees are passed to children on one path;
those child accesses are not part of this bounded direct-footprint proof.
The intervening message load and `PUSH EDI` preserve flags from the control
test, so the first `JZ` still selects the zero-control case.

There is no null destination/source check, self-copy check, prior-destination
free, destructor or ownership reset. The profile write precedes source-field
reads, and the control publication precedes the initial message read. These
orders matter if the actual ranges overlap. Earlier writes can remain visible
if a later access or child call fails. The profile is opaque numerical data,
not a callable Source vtable or proof of Native exception identity.

## Four paths and call delivery

| Condition | Ordered behavior after the common prefix |
| --- | --- |
| Captured control is zero | Store the captured message DWORD at `D+4`; make no child call |
| Control is nonzero, captured message is null | `AND DWORD [D+4],0`; make no child call |
| Control and message are nonzero, allocation returns null | Compute length+1, allocate, publish null at `D+4`, then return with control unchanged |
| Control and message are nonzero, allocation returns nonnull | Publish the allocation at `D+4`, reload current `S+4`, call the copy provider, ignore its return value, then return `D` |

The null-message RMW is at `00BF63ED..00BF63F0` (`83 66 04 00`). It requires
the old destination DWORD to be readable as well as writable, although its
normal result is zero. Its instruction flags are distinct from a plain MOV.

Metadata-only child results are:

| Call site | Child | Saved signature |
| --- | --- | --- |
| `00BF63C7` | `00C03DE0`, `_strlen` | `size_t __cdecl _strlen(char*)` |
| `00BF63D0` | `00BF9F1A`, `_malloc` | `void* __cdecl _malloc(size_t)` |
| `00BF63E3` | `00BF93A4`, `_strcpy_s` | `errno_t __cdecl _strcpy_s(char*, rsize_t, char*)` |

The first call receives the initially captured message pointer. Its EAX result
is copied to EDI and incremented as a 32-bit value, so the delivered size is
`(length + 1) mod 2^32`. That value is pushed for `_malloc` while the previous
length-call argument is still on the caller's stack. After `_malloc`, two
`POP ECX` instructions discard the two argument words. They preserve the
flags from `TEST EAX,EAX`, allowing the null-allocation branch after the
destination pointer store to use that same test.

On allocation success, `PUSH DWORD [EBX+4]` at `00BF63DE` performs the actual
source-pointer reload. The pushes deliver `_strcpy_s(allocation, size,
reloaded_source)`, followed by caller cleanup of 12 bytes. If `D` and `S` are
identical, this reload observes the newly published allocation; the delivered
destination and source pointers therefore coincide. No provider behavior for
that case is inferred. The child's return is ignored and EAX is later set to
the captured destination.

The call sites and metadata establish the stated argument/normal-cleanup
contract. Child body behavior, allocator identity, invalid-parameter policy,
string validity, failure/unwind behavior and runtime equivalence are unproved.

## Current private Source helper

`copy_base` in `src/native_legacy_exception_owner.cpp` is in the anonymous
namespace and takes two `NativeLegacyExceptionStorage` references. That type
is 0x28 bytes, with the relevant fields at offsets 0, 4 and 8 and a separate
member starting at 0x0C. The helper happens to access those leading fields;
it is not a callable raw-12 adapter. A 12-byte allocation must not be cast to
that larger Source owner to reuse it.

The helper preserves the profile/control/message order, full-width control
copy, non-owning shallow pointer case, nullable allocation publication,
32-bit length increment and post-allocation source reload. It calls current
Source CRT functions, whose identity with the selected Native providers is
not established here. It returns `void`, rather than exposing the Native EAX
receiver result, and uses an ordinary private Source interface.

Two corrections are needed before claiming a new complete raw-copy adapter:

1. Use the proved inclusive endpoint `00BF63FD` / 88 bytes in its evidence.
2. Explicitly preserve the null-message DWORD RMW and its access order; a null
   assignment alone does not encode that contract.

A separate raw-12 interface, actual source/destination backing, provider
bindings and ownership/lifetime contract require explicit primary selection.
No new exception type, static failure object, string contents or ABI identity
is inferred from the existing helper.

## Validation boundary

The report retains the complete live listing and bytes, independently decodes
all 88 bytes against the installed PE, checks every instruction boundary,
branch target, call site, final `RET 4` and the RMW width/access flags. It pins
the current helper, its type/namespace/comment excerpts, target-verification
implementation and accepted upstream reports. All 39 current input/document
pins from the newly admitted wrapper's primary review are replayed.

The independent report check and exact two-file staged diff check are static
evidence only. No compiler output, Native execution, exception/CRT ABI,
allocator lifecycle, startup or gameplay behavior is validated by this packet.
