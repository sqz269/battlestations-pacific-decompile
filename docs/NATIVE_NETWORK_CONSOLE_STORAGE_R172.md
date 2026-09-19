# Network-console channel, queue and singleton storage

R172 reconstructs twelve complete normal bodies, totaling 706 original bytes.
These are dependencies of the 4003F0h owner allocated at 73DC9D and constructed
at 73DCBF. Native strings identify its send/receive entries as
`cNetworkConsoleSend::sSendThread` and `cNetworkConsoleSend::sRecvThread`.
Descriptive source/function names remain hypotheses, not recovered symbols.

| Entry | Bytes | Behavior |
| --- | ---: | --- |
| A3A330 / A3A340 | 13 / 1 | Actual 10h slot constructor and RET-only destructor |
| A3A3D0 / A3A3E0 | 11 / 1 | Actual 82Ch queue element constructor and RET-only destructor |
| A3A500 / A3A520 | 18 / 1 | Actual 820h queue element constructor and RET-only destructor |
| A3A770 | 22 | Reverse destruction of 64 slots at channel+8C |
| A3AD10 | 223 | Channel reset through current clock sampling and exact x87 division |
| A3CE60 | 88 | Construct all 64 slots, then reset the actual 49Ch channel |
| A3CF00 / A3CFA0 | 145 / 153 | Publish/register and unregister/clear the network-console singleton base |
| A3D040 | 30 | Scalar base destruction; only flags bit0 releases allocation |

## Storage and arithmetic

Storage types have exact native extents and no default member initialization.
The 10h constructor writes only byte0 and DWORD4. Both queue constructors write
the one-past-element pointer at+0; only the 820h instantiation clears DWORD1C.
Their destructors are distinct RET-only bodies. No send/receive direction is
inferred merely from the queue strides.

A3AD10 writes channel byte0=1 and byte18=0 before sampling the current AB0+20
clock. The source uses the existing actual 80h clock/publication adapter and an
explicit caller-supplied 16-byte output preimage. It does not substitute a typed
clock projection. The native QPC sampler's ignored return behavior is unchanged;
the new fixture exercises its deterministic fixed-counter branch.

The reset preserves FILD/FILD/FDIVP, the intervening twelve DWORD clears, and the
later FSTP into float20. One inline assembly block preserves this operation and
store order without a temporary double. The rest of the scalar writes retain
their original order. A 32-iteration loop clears the byte/DWORD pair in each of
two interleaved halves of the 64-slot array. Padding stays untouched. The final
D7A260 load occurs after the loop; its bits, normally BF800000 (-1.0f), are stored
unchanged at+24 after clearing DWORD14.

A3CE60 constructs the 64 actual elements at+8C before reset. On a source C++
failure it invokes the corresponding reverse member cleanup, whose element
destructors have no effect. Native private-frame FH3/SEH equivalence is not
established by this projection.

## Singleton base lifetime

The base context borrows the actual F8ABDC and 1090AA0 publication cells. A3CF00
stamps D23F44, captures the first manager's tracked section, enters/increments
that section, publishes the captured owner, then gets the manager again and
registers the **current** publication. A3CFA0 likewise unregisters the current
publication, clears it, leaves/decrements the captured section and stamps
CE3818 on the captured owner. Publication replacement is not confused with the
destructor's captured ECX owner.

The existing actual manager, vector registration/removal, tracked guard and
generic-base services are reused. The source scalar wrapper requires matching
`new NativeNetworkConsoleStorage` storage for flags bit0 and returns the
captured identity even after deletion. It does not construct or destroy the
derived arrays, critical sections or worker threads.

## Analysis and verification

- Seven missing functions were defined in the existing BSP project with exact
  byte ranges. The A3D040 call-site flow repair restores ADD ESP,4 at A3D055
  after the returning free. No global callee no-return flag was changed.
- 1147 live Ghidra/PE bytes agree: the twelve bodies, full existing A3D060 parent,
  parent allocation/call site, labels/profiles, reset bits and initial publication.
  Nineteen direct CALL rows are independently verified.
- Strict MSVC Win32 build and all three existing CTests pass.
- One local differential fixture executes all 706 copied original bytes. Its
  38 original/source pairs compare 8,433,752 bytes, including complete 4003F0h
  base-owner preimages and padding. Queue pointers are compared at the same
  address before normalization to relative offsets in archived observations.
- The channel comparisons cover 32 combinations of x87 precision, rounding and
  signed timestamp ratios, including masked zero/zero. A separate reset checks
  untouched bytes and exact non-default D7A260 bits. Singleton checks cover a
  replaced current publication, captured lock depth, flags2 retaining storage
  and flags101 releasing it. One source-only null-clock failure check retains
  initial byte writes and constructed-slot padding.

Fixture boundaries use the existing actual fixed-clock sampler, manager vector
registration/removal and Win32 sections. The native CRT vector-iterator boundary
uses a fixture normal-path loop over the copied original callbacks; free uses
the same MSVC CRT allocation domain. Original FH3, new-handler/allocator identity,
unmasked floating faults, native hardware exceptions and concurrency are not
claimed. The fixture adds no repository test suite or runtime substitution.

The full derived A3D060 constructor, A3D1A0 destructor and two worker loops still
need source composition and runtime validation. Ordinary application startup
does not create this owner yet. This packet does not establish online peer,
network exchange, visual or gameplay parity.

[The report](../reports/native_network_console_storage_r172.json) pins source,
objects, fixture outputs, Ghidra repairs/annotations and integration evidence.
