# Actual DWORD vector copy primitives

This packet supplies complete ordinary Source entries for `00555000` (38 bytes,
15 instructions) and `004CB130` (42 bytes, 13 instructions). It preserves the
correct library names `STL_UninitCopy_00555000` and `STL_Vector_Tidy_004cb130`.
New descriptive C++ names are hypotheses. Existing qualified APIs remain intact.

Fresh complete typed bodies, every current instruction start and live bytes
agree with the installed PE. All 38 copy bytes equal accepted `00696D40`
without a mask. The 42 tidy bytes equal accepted `00468840` except the four
operand bytes of their offset0B E8 instructions; both independently resolve to the
same actual free service. No instruction, field access or operation is masked.
Source delegates directly to those accepted actual services; it adds no loop,
count bridge, callback, provider, object overlay or allocation model.

The copy entry receives first in ECX and last in EDX, then four stack words:
destination, unused owner, unused metadata and unused tag. It returns the
advanced destination in EAX and RET10h. Destination is captured even for empty
input. For each nonempty step, a nonnull output reads the CURRENT source DWORD
immediately before storing it; null output skips both accesses. Source and output
cursors both advance4 with DWORD wrapping until first equals captured last.
All six physical values are forwarded unchanged, including dead words. There
is no memmove, snapshot, element count, overlap guard or pointer subtraction.

The tidy entry receives opaque16B storage in ECX, ignores incoming EDX, takes no
stack inputs and uses plain RET. Its Source result is void; ambient native EAX
is not a semantic result. Capture DWORD+4 ONCE and conditionally free that
captured pointer, then zero+4, +8 and +C in order on both returning paths. The
header remains writable through those post-free stores. There is no second
pointer read, element destructor, early null return or vector-clear substitution.

The owned native transfer and independently audited reference are:

- `004CB13B CALL 00BF65AC`: free the captured pointer.
- `0046884B CALL 00BF65AC`: same-target reference used for the exact mask proof.

Neither complete native body has local EH registration, catch, action or owned
temporary. Source adds no cleanup, noexcept promise, guard or exception
conversion. It inherits the accepted current CRT free and raw-copy C++ policies.
Current CPP/CRT behavior does not establish original FH3, RTTI, heap, fault or
SEH identity. Reached raw accesses/releases must be valid and copy equality
termination finite; public progressive aliases retain their native order.
Original private-stack/argument aliases, ambient registers/flags, asynchronous
mutation, binary ABI and game/runtime equivalence remain unproved.

The report records exact native and Source fingerprints, COFF physical-interface
inspection, full MSVC Win32 build and all three existing suites. No new test,
wrapper fixture, native execution or gameplay validation is claimed. Compiler
delegation is Source reuse, not evidence that the original entries called the
reference implementations. This packet does not reconstruct the constructor,
its catch or the placement wrappers. The historical placement same-body/ABI
correction remains explicit in the separate read-only constructor readiness.

COFF inspection finds a 9-byte copy delegate with a restored EBP frame before
its sole tail jump, and a 5-byte tidy delegate containing only its tail jump.
Exact external symbol relocations identify the accepted services. Neither path
changes ECX/EDX or the caller's existing argument slots before transfer; the
target supplies the established return/cleanup policy. No wrapper EH tables or
extra context/adapter words are emitted. This is current Source compiler proof.
