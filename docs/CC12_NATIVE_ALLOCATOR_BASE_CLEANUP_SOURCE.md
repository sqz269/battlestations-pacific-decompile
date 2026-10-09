# Native allocator base-cleanup Source

`bsp::cleanup_native_allocator_base_00bf6454` implements the seven physical
operations of `00BF6454..00BF6469`, the accepted 22-byte Original cleanup.
It compares the full control DWORD before profile publication, conditionally
loads the current message pointer, calls current Source CRT `free`, pops the
argument into ECX and uses plain `RET`.

The private typed `destroy_base` helper now delegates using its actual owner
address and explicit unused EDX word zero. Its existing `noexcept` Source
policy is retained, and its evidence comment is corrected from `00BF646A`
to `00BF6469` / 22 Original bytes. All other legacy-owner Source behavior is
unchanged outside the one include and helper edit.

Primary review registered and admitted this Source function after the normal
MSVC Win32 build passed all three existing checks. The complete emitted adapter,
actual typed consumers, Core membership and final UCRT binding were verified.
The worker report retains its historical candidate state; the primary report
records current admission. Original ABI and gameplay remain unproved.

## Raw boundary and exact operations

```cpp
void __fastcall cleanup_native_allocator_base_00bf6454(
    void* actual_receiver,
    std::uint32_t unused_edx);
```

The header requires MSVC Win32. The naked implementation asserts four-byte
pointers and DWORDs. ECX supplies the actual raw receiver. The second Source
argument occupies EDX but is not read by the owned instructions; the current
CRT child may clobber volatile registers. There are no stack arguments.

The raw function returns `void`, imposes no EAX result and has no `noexcept`
specification. Its implementation contains only these seven operations:

| Original site | Source assembly |
| --- | --- |
| `00BF6454` | `CMP DWORD [ECX+8],0` |
| `00BF6458` | `MOV DWORD [ECX],00D69370h` |
| `00BF645E` | `JE cleanup_complete` |
| `00BF6460` | `PUSH DWORD [ECX+4]` |
| `00BF6463` | `CALL free` |
| `00BF6468` | `POP ECX` |
| `00BF6469` | `cleanup_complete: RET` |

The compare reads the entire control DWORD. The profile MOV preserves its
flags for the branch. The message pointer is read only on nonzero control,
after profile publication, and its current value is passed without a null
guard. The physical POP removes the child argument on normal return. This
remains a call followed by cleanup and return; no tail jump is substituted.

Zero control leaves ECX equal to the receiver, with EAX untouched by owned
instructions. A normal child return is followed by POP, leaving ECX equal
to the pushed message. EAX/EDX/flags on that path remain subject to the
current CRT child. The public interface makes no semantic return-value or
whole-call register/flag equivalence claim.

## Backing, fields and provider qualification

The receiver requires only the actual leading 12-byte backing needed by
the accesses: readable DWORD at +8, writable DWORD at +0, and readable DWORD
at +4 when control is nonzero. There is no receiver check, message check,
control normalization, ownership reset or local message/control-field store.
The raw body adds no EH frame, catch, callback, helper stub or exception type.

The single call binds through the current Source CRT declaration from
`<cstdlib>`. It replaces the old private helper's direct `std::free` boundary
with that same selected Source operation. Actual emitted symbol/import
resolution remains a primary build check. The accepted Native child is
metadata-only `_free` at `00BF9DC8`; its implementation, placement, register,
flag, failure and exception behavior are not proved by the Source name.

The Source import call may be six bytes, making the seven-operation adapter
one byte larger than the Original's five-byte relative CALL. No exact 22-byte
Source-emission claim is made. Branch labels preserve the intended target
regardless of the emitted call form; actual instruction sizes and relocations
remain pending primary review.

The raw `00D69370` profile is numerical data, not a callable vtable, RTTI,
static failure owner, Native slot or exception/runtime identity. Pointee
ownership, allocator compatibility, backing lifetime, aliased child effects
and failure handling remain caller/provider contracts. No field clear is
introduced after free, and no guarantee is inferred that aliased backing
remains alive. Original placement/callers, child ABI, flags/faults, exception
runtime and gameplay equivalence remain unproved.

## Actual typed consumer

The private helper retains its signature and existing Source policy:

```cpp
void destroy_base(NativeLegacyExceptionStorage& owner) noexcept {
    cleanup_native_allocator_base_00bf6454(&owner, 0u);
}
```

This passes the actual address of a valid existing 0x28-byte typed owner,
whose leading 12 bytes supply the raw contract. No raw 12-byte allocation is
cast to that larger type. Its `noexcept` policy is kept on this existing
helper only; it is not added to the raw entry or promoted to Native EH proof.
Compiler-generated consumer behavior remains part of the primary review.

Whole-file comparison proves that the current legacy-owner Source differs
from the selected baseline only by the added cleanup header and exact
private-helper replacement. The copy adapter delegation, constructors,
member-string handling, cleanup guards and scalar-retirement code are retained.

## Evidence and verification boundary

The accepted cleanup report supplies the complete 22-byte physical body,
primary Astra SHA and the visible listing's missing `POP ECX` at `00BF6468`.
The Source candidate includes that independently proved instruction. No
new Ghidra query or mutation, cause/no-return-flag investigation, adjacent
byte read or Native child/handler/data inspection is made.

The report independently rechecks the complete allowed Original span against
the configured PE and maps all seven Source operations to it, with the one
explicit current-CRT call binding. It pins the raw header/body, modified
consumer, documentation and unchanged inputs at published baseline
`cc2ce843bc3af96242a0beb901dc33f4b82bc7c2`.

All 59 inherited references, including all 50 current copy-primary input and
document pins, match that baseline before migration. Two references point to
the one intentionally changed legacy-owner path. The report keeps their
historical baseline hashes separate from the current candidate hash and
verifies every other referenced path is unchanged. Earlier admitted copy and
constructor routines are not counted again.

The new file is unregistered; primary registration must precede the relevant
normal build. No worker compile, emitted-body/Core, actual consumer-call,
Native ABI or gameplay validation is claimed by these static checks.

## Primary registered build and admission review

The normal MSVC Win32 build passed all three existing checks. Primary review
verified the complete emitted body, physical Core membership and unique public
definition, with all selected relocations resolved by physical symbol indices.
The raw cleanup emits 23 bytes / seven operations because its current UCRT
free import uses a six-byte call. Existing typed cleanup calls this adapter;
the complete typed object/EH graphs and final import were reviewed.
The typed destructor's physical body is 93 bytes (88 code plus five alignment
bytes), and the private helper is 64 bytes (59 code plus five alignment bytes).
The retained typed `noexcept` policy emits FuncInfo flags `5` and terminate
references around the non-`noexcept` raw declaration. These are current Source
compiler effects; the naked adapter has no local EH and neither establishes
Original exception-runtime parity.
This is Source build/admission evidence; Original placement, caller/provider
runtime parity, arbitrary faults/concurrency, startup and gameplay are unproved.
