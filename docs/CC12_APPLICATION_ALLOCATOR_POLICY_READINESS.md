# Application allocator: outer policy and failure-object boundary

This read-only packet renews all **105 bytes / 33 instructions** of
`operator_new` at `00BF681B..00BF6883`, plus the separately leased adjacent
byte `00BF6884`. The complete body matches the installed PE. Its allocation /
new-handler retry policy agrees with the current Source helper for the
qualified stable-size case. The native static error object, copy, callback
registration and terminal exception service remain distinct work.

Evidence: [machine-readable report](../reports/cc12_application_allocator_policy_readiness.json).
Base: published main `c9c6808db971c603e57a469500ad3f7c24b69be5`.
The [CRT entry-point audit](CC12_APPLICATION_POINTER_VECTOR_CRT_BOUNDARY_READINESS.md)
already establishes the fixed `00BF55BE -> 00BF681B` reserve allocation route.
No Source, Ghidra, ledger or CMake changes, new credit, builds, tests, probes,
native execution or Application wiring are added.

## Capture and physical coverage

The existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86
little-endian target at image base `00400000` was verified before live queries;
the function count was 64,729. The 105-byte whole body passed the 160-byte gate
before fresh pseudocode, listing and byte capture. All 33 live instruction
starts, four internal branch targets and six direct call targets agree with an
independent complete decode. The saved library name and prototype were preserved.

The body ends with the five-byte call at `00BF687F..00BF6883`. Its physical
return address is `00BF6884`; that separately captured byte is `CC`, decoded as
`INT3`. No omitted `LEAVE`/`RET` lies in this one-byte gap. The next address,
`00BF6885`, was queried only as direct-child metadata.

Metadata labels `00BF6885` as `noreturn __CxxThrowException@8`; this is recorded
without treating it as verified runtime behavior. The read-only terminal
flow-property query returned `Script execution disabled`, so the call's current
override/fallthrough properties were unavailable. The complete error response
is pinned. No server setting, permission, annotation, listing or flow flag was
changed. Physical coverage and the `CC` tail do not depend on those flags.
If the terminal call returned, its first physical continuation would be `INT3`;
this does not supply a valid normal-return policy or prove what an external
debugger/exception handler would do next.

No Native child body, handler, caller, global/table contents, import or heap
internals were read. Six direct children received prototype metadata only.
There is no indirect call in the selected body; custom callback behavior
inside children remains outside the scope.

## Ordinary entry, size reads and retry policy

Let `S` be incoming `ESP`. The body pushes `EBP`, sets `EBP = S-4`, reserves
`0Ch` bytes, and initially jumps to allocation. The actual size argument remains
at `[EBP+8] = S+4`; local storage begins at `L = EBP-0Ch = S-10h`.

| Sites | Established operation |
| --- | --- |
| `00BF6830`, `00BF6833` | Read the CURRENT DWORD at `[EBP+8]`, push it, and call fixed `_malloc` at `00BF9F1A`. |
| `00BF6838..00BF683E` | Test `EAX`, clean the pushed size with `POP ECX`, and branch only for zero. `POP` preserves the test flags. Nonzero `EAX` returns unchanged through `LEAVE; RET`. |
| `00BF6823`, `00BF6826` | After a zero allocation result, reread CURRENT `[EBP+8]`, push it, and call fixed `__callnewh` at `00C055B1`. |
| `00BF682B..00BF682E` | Test `EAX`, clean four bytes with `POP ECX`, and enter failure construction only for zero. Any nonzero result, including negative, repeats allocation. |

The size is physically reread independently for each child call. It is not
cached as an entry value in a register or private local. No clamp, size-zero
normalization, overflow check, retry bound, pointer validation or catch appears
in this body. Its normal-return path requires a nonzero malloc result and
does not modify that pointer. Plain `RET` leaves the original size word for the
caller to clean. The normal path preserves `EBP` and does not touch `ESI`,
`EBX` or `EDI`.

These operations establish the outer policy, not `_malloc`'s internal zero-size,
heap/new-mode behavior or the custom handler's implementation. A zero result
invokes the new-handler service even when the passed size is zero. Children may
have their own failure, exception or nonreturn behavior; their bodies were not
expanded.

## Failure-object schedule and opaque operands

On a zero new-handler result, the native body performs this exact sequence:

1. At `00BF683F`, test bit zero of BYTE `[0109DD74]`. Set `ESI` to static
   address `0109DD68`; that `MOV` preserves the test flags used by `JNZ`.
2. If the bit was clear, OR bit zero into DWORD `[0109DD74]` at `00BF684D`
   **before** calling `00BF6802` with `ECX = 0109DD68`. Then pass callback
   address `00CE1122` to `00BF6FF5` and discard its return status. If the bit was
   already set, skip both calls. No local guard reset or atomic synchronization
   is present.
3. Push the captured static address, set `ECX = L`, and call `00BF63A6`. The
   reserved 12-byte local area is not pre-zeroed by this caller. No explicit
   caller cleanup follows this child; its metadata says `thiscall`, while its
   actual `RET`/cleanup remains unaudited here.
4. Push opaque address `00E03CC0`, then push `L`. Write DWORD `00D6923C` at
   `L+0` after the copy and call `00BF6885`. There is no local destructor,
   epilogue, catch or rollback after this call in the selected body.

Metadata names the constructor `bad_alloc`, the copy `exception`, registration
`_atexit`, and the terminal service `__CxxThrowException@8`. Those names suggest
library roles but do not establish the thrown class's layout, message ownership,
type identity, destructor, or compatibility with current host exceptions.
The guard, static object, callback, profile and metadata addresses above were
recorded solely as instruction operands; none was dereferenced as analysis data.

The failure path writes `ESI` without saving/restoring it and has no normal-return
epilogue. This is not a hidden incoming register argument or a newly accepted
exceptional register-preservation contract. There is no local FS registration
or unwind-state machinery in the selected body. If construction/registration
fails after setting the guard, no local reset is selected; child and caller
cleanup remain separate. Thread safety, reentrancy and asynchronous fault
behavior are not inferred.

## Current Source comparison

[singleton_lifetime_allocate](../src/singleton_lifetime.cpp), lines 51–62,
calls host `std::malloc(request.host_bytes)`, returns a nonnull result, calls
host `_callnewh(host_bytes)` on failure, retries on nonzero, and throws host
`std::bad_alloc` on zero. It uses the current MSVC host CRT and exception runtime.
It adds no explicit retry limit or zero-size normalization.

The concrete renderer adapter supplies `{object, size, size}` as established by
the prior CRT audit, so vector allocation's host size equals its incoming
32-bit size, including an already-wrapped zero. The Source expression reads
`request.host_bytes`; the adapter materializes a distinct request object, while
the Native function rereads its own argument word. Stable-size outer behavior
agrees. Arbitrary stack/request aliasing, custom-handler mutation, exact emitted
instruction timing, and original entry ABI are not thereby equivalent.

Source's direct `throw std::bad_alloc()` does not reproduce the observed native
guard/static-object addresses, callback registration, 12-byte local copy,
first-DWORD publication or opaque terminal-call metadata. The old singleton
audit already classifies this helper as a host CRT service boundary and excludes
a new-handler exhaustion fixture; this packet does not renew historical tests
or prove native C++ EH/FH3 identity.

## Existing related exception Source and further bounded work

[native_legacy_exception_owner.cpp](../src/native_legacy_exception_owner.cpp)
contains private `construct_base`, `destroy_base` and `copy_base` helpers for
logic/length-error owners. The copy helper explicitly references `00BF63A6`;
it uses nullable host malloc and message-ownership fields. The destructor helper
publishes a base profile and conditionally calls host free. Their current Source
is pinned, but no Native child contract was renewed.

The public owner in
[native_legacy_exception_owner.hpp](../include/bsp/native_legacy_exception_owner.hpp)
is `0x28` bytes with a leading `0Ch` base. It is not a proven `0Ch` bad-allocation
frame type or a callable adapter for this allocator failure path. Treating the
12-byte native reservation as a live `0x28` owner would invent storage/lifetime
that this packet has not established.

The copy helper's comment spans `00BF63A6..00BF63FE` (89 bytes); fresh metadata
ends at `00BF63FD` (88 bytes). No child bytes or repair were authorized, so that
extent difference is explicitly unresolved. The exact case-insensitive address
search found this one Source comment, with no binding for the other static /
constructor / callback / profile / terminal operands in `src` or `include`.

Further work would need bounded audits of the 25-byte constructor, the copy
extent/ABI, the registered callback's actual retirement behavior, and the opaque
profile/throw metadata/runtime. A callable 12-byte owner interface and its real
lifetime would then need an explicit composition contract. No such adapter,
exception type or destructor behavior is invented here.

Validation pins eleven repository inputs and bounded excerpts, replays 29
inherited canonical references over 17 unique paths, and independently verifies
all 106 captured bytes. Prior compiled graphs were not re-audited or rebuilt.
