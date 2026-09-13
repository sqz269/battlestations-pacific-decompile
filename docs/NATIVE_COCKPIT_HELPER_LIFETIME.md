# Actual cockpit helper destruction

Addresses: 00B3C5C0, 00B3C6C0. Names are descriptive hypotheses.

| Entry | Inclusive body | ABI | Coverage |
| --- | --- | --- | --- |
| B3C5C0 | B3C5C0..B3C649, 138 bytes | ECX actual 24h helper, no arguments, RET | Complete normal control flow and base-only C++ exception projection; explicit binding domain below |
| B3C6C0 | B3C6C0..B3C6DD, 30 bytes | ECX helper, DWORD flags, RET4; EAX captured allocation | Complete scalar body in the existing BF681B/free domain |

`NativeCockpitHelperStorage` views already-constructed storage. It does not
implement B3C800 or start replacement field lifetimes. Native +04 is the sole
reference count; +08 is an attached arbitrary node, +0C is a camera, and +18 is
a retained owner. Both native routines preserve +04/+10/+14/+1C/+20. Scalar
flags0 leaves ended storage; bit0 calls the existing `singleton_lifetime_free`
BF65AC provider. Other flag bits do not free. Allocate actual 24h storage in the
BF681B domain, never `sizeof(NativeCockpitHelperOwner)` or companion bytes.

| Site | Native target and verified preparation | Source provider |
| --- | --- | --- |
| B3C5F2 | B6DFA0, ECX current +0C, no args | Canonical actual node lookup then `unlink_and_release_render_model_00b6dfa0` |
| B3C601 | B6DFA0, ECX reloaded +08, no args | Same provider, without forcing the camera profile |
| B3C614 | IAT CE2220, pushed captured +18 owner's +04; stdcall | Existing actual retained-owner binding atomic decrement |
| B3C624 | Captured owner's current profile/slot0, ECX captured owner, no args | Existing binding callback must resolve current profile after decrement |
| B3C633 | BD30F0, ECX helper | Stamp CEB130 |
| B3C6C3 | B3C5C0, ECX captured helper | Full helper destruction |
| B3C6D0 | BF65AC, pushed captured allocation; B3C6D5 ADD ESP,4 | Existing `singleton_lifetime_free` |
| CBECC3 | BD30F0 tail; ECX saved [EBP-10] | Base-only C++ catch cleanup |

B6DFA0 was read through B6E007: it detaches parent/siblings or root, then loads
current virtual18. It is not a count decrement. The existing canonical node
runtime supports persistent `NativeCameraReference` and other actual node
descriptors. Missing identities fail explicitly before release. Concrete
descriptors are responsible for current native virtual-slot validation.

B3C5DF stamps D61854 before member operations. +0C is cleared only after
successful release; +08 is then reloaded and similarly cleared. +18 is captured
once, decremented through its registered actual +04 and cleared only after
successful zero handling. A member callback may alter later members. The
native +18 nonzero producer remains unestablished: B3C800's zero initialization
is known, but generic retained-owner registration is not proof of a concrete
producer. Unsupported nonzero owners are outside the binding domain, and fail
explicitly. Do not claim unrestricted concrete-member coverage.

Read-only Ghidra target: `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`; original PE SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The guarded CLI confirmed both extents and all listed call preparations.
D61854 contains exactly [BD30E0,B3C6C0]. BD30E0 reads current slot04 and passes
flag1 without decrement. It is never invoked by casting an integer to a host
vtable. The stable terminal companion validates both actual table words and
the current owner profile. It uses actual +04, adds no retain, and ends native
lifetime/free before retiring host companions. Once bound, direct destruction
is forbidden by contract. The retirement callback may dispose both companions;
neither native storage nor companions are accessed afterward.

FH3 evidence: B3C5C2 installs CBECC8; DF74A0 is magic19930522, maxState1,
unwind map DF7498 = [-1,CBECC0]. B3C5DB saves helper [ESP+0C]/[EBP-10].
B3C5EC arms state0 before any release. Escaping failures run only BD30F0 via
CBECC0, without retrying members or attempting later releases. B3C62B consumes
state0 before normal base cleanup. The source catches C++ exceptions and marks
the helper dead after the base stamp; scalar free is skipped on failure.
Existing terminal/node interfaces are noexcept and may terminate rather than
propagate. Native FH3/SEH, nested exception behavior, drop-in ABI compatibility,
and gameplay have not been validated. No Ghidra mutations or constructor work.

Validation is recorded in the JSON report. The worker source is compiled as a
separate strict MSVC Win32 TU; the parent's integration registers it with CMake.
The worker's standard build and existing tests validate the baseline libraries,
not execution of this unregistered TU. No synthetic member provider, new test
framework, or native runtime/fixture proof is claimed.
