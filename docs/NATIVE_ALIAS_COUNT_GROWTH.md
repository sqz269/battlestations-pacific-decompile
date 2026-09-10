# Native alias-list count growth and owning length error

`grow_native_alias_list_count_004ce780` reconstructs all 147 bytes at
`004CE780..004CE813`, including the normally returning count update and the
length-error factory. The original uses ECX for the actual list owner, one
callee-popped stack DWORD for the increment, and RET4 with no semantic return.
The count occupies owner+8; no list header or node is copied into a projection.

The original captures the count once in EAX and compares the increment with
unsigned `0x1fffffff - captured_count`. On success it writes
`captured_count + increment` to the same owner+8. Both operations wrap as DWORD
arithmetic. An already-invalid count above the limit can therefore pass the
subtraction check: `0xffffffff + 1` becomes zero. The reconstruction preserves
this behavior rather than strengthening the predicate.

## Error construction and cleanup

On the failing branch, the original prepares a raw 1Ch SBO temporary in this
order: capacity=15, length=0, then the first inline byte=NUL. It calls
`00408720` with literal `00CE38F8` and count **16**. The text has sixteen visible
characters, `list<T> too long`; the counted copy excludes its terminating NUL
at `00CE3908`, and the string helper appends a NUL. The resulting string has length16
and capacity31, with a real 32-byte heap allocation.

Only after counted assignment returns does `004CE7D9` arm factory unwind
state0. The factory constructs the 28h legacy logic-error owner at `00411700`,
publishes length-error vtable DWORD `00D69260`, and calls `00BF6885` with
ThrowInfo `00D83F98`. It never stores a count on this path; count changes made
by an allocation callback therefore remain visible.

The state0 entry in unwind map `00D8E3C8` targets `00C65A80`. This eight-byte
funclet obtains the SBO temporary at EBP-50h and jumps to `004072D0`.
The following ten-byte handler at `00C65A88` loads FuncInfo `00D8E3D0` and jumps
to `00BF6B43`. A failed initial counted assignment has no completed temporary
in this factory's unwind state. A failed owner constructor does have one.

The C++ source creates its `CompletedTemporary` guard after successful counted
assignment. The direct prvalue expression
`throw NativeAliasListLengthError{temporary}` constructs the owning transport
without an additional named exception temporary or artificial copy.

## Concrete host exception transport

`NativeAliasListLengthError` contains only one `NativeLegacyExceptionStorage`,
with an asserted total size of 28h. The raw member is default-initialized
without value-initialization, preserving the native owners' untouched bytes.
Its constructor calls `construct_native_legacy_logic_error_00411700` and then
publishes the literal length-error vtable DWORD. Its copy constructor calls
`copy_native_legacy_length_error_00411940`; its noexcept destructor calls
`destroy_native_legacy_logic_error_00411780`. Copy assignment is deleted to
prevent a shallow pointer assignment. There is no `std::exception` base.

This is a new host C++ catch type, RTTI and exception ABI. Consumers can catch
`const NativeAliasListLengthError&` and inspect `native_storage()`, or preserve
the owning exception with an ordinary `throw;`. The literal native vtable is
address data, not a callable host vtable. Do not cast this class or its raw
payload to a modern `std::length_error`, `std::logic_error` or `std::exception`.

Native ThrowInfo `00D83F98` names destructor `00411780` and catchable array
`00D83FD4`. Its three entries describe length_error (40 bytes, copy00411940),
logic_error (40 bytes, copy004118D0), and exception (12 bytes, copy00BF63A6).
Those original metadata records are evidence for the native ownership chain;
they are not the reconstructed class's compiler-generated RTTI.

## Verification

The isolated worker began at main `f46f236` plus the previously reviewed
exception-owner dependency `46e7582` (local cherry-pick `58fb603`). No shared
CMake, ledger, Ghidra annotation or export changes are part of this packet.
The primary integrator must register the source and apply reviewed annotations.

Every byte-analysis batch checked `C:/Users/sqz269/bsp.gpr` and program
`/battlestationspacific.exe`. The 45 audited ranges comprise 16 complete code
spans, 11 boundary preimages, 13 data spans and five contextual support spans.
All matched the installed PE, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Eight seed ranges also matched before native execution. The strict MSVC Win32
build passed, followed by the existing two CTests.

One ignored Win32 fixture executes the complete original `004CE780`, native
exception-owner code, native temporary/base cleanup funclets and mapped
ThrowInfo/catchable records. SBO boundary calls bind the previously verified
concrete shared SBO implementation. Original absolute EH/data pointers are
relocated after checking preimages; handler registrations use compiled naked
entry points forwarding the original FuncInfo to the actual host
`__CxxFrameHandler3`. Vtable DWORDs remain literal.

The throw boundary observes the payload and forwards the original object and
mapped ThrowInfo unchanged to the actual host CRT `_CxxThrowException`.
It does not replace the throw with a fabricated callback or a host standard
exception. Native length errors are caught with `catch(...)`; no modern
standard-exception method accesses their legacy layout. The original
ThrowInfo destructor executes on leaving the catch. Catch-by-value is not
used, so this fixture does not claim the runtime invoked a native copy thunk;
the native copy routines were verified by the dependency packet, and this
fixture also checks the concrete host class makes an independent deep copy.

The native/reconstructed sequence has **127 matching words** covering:

- Count-only updates, exact limit, corrupt-count underflow and DWORD wrapping.
- Two real 32-byte allocations for successful error construction, followed by
  temporary release during unwind and owned payload release after catch.
- A real allocation callback repairing owner+8 to `0x55`, retained after throw.
- Initial temporary-allocation failure: two growth attempts and no heap free.
- Owner-allocation failure after the temporary completes: three allocation
  attempts total and exactly one temporary heap free.

The allocation failures use controlled nullable malloc results and an actual
registered CRT new-handler that throws a marker. The fixture observes only
its own executable's malloc/free/throw imports and restores them afterwards.
The final failure case produces three original versus four reconstructed
`_CxxThrowException` calls: the reconstructed owner cleanup uses an explicit
`catch (...) { ...; throw; }`, while the original uses its unwind table. This
transport-level difference is recorded separately from the identical count,
payload, allocation and disposal trace. Successful length errors each produce
one actual throw call, with no extra transport copy.

The ignored fixture and byte-audit artifacts are identified and hashed in
`reports/native_alias_count_growth_audit.json`. No tracked tests were added.
This establishes bounded reconstructed and fixture-tested behavior. It is not
a drop-in exception ABI replacement, full CRT reconstruction, or game validation.
