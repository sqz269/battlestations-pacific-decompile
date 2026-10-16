# Whole traffic vector allocation and length failure

This packet qualifies and reconstructs complete logical Source for `0047E970`
(85 bytes / 25 instructions) and `00490240` (105 bytes / 23 instructions).
Existing library names `STL_allocator_allocate_0047e970` and
`STL_xlen_throw_00490240` are retained. Source names describe behavior; no game
class is recovered. Both entries compose complete, already accepted real Source
implementations after full Native equivalence and dependency review.

## Allocation

Original `0047E970` consumes unsigned count in ECX, no incoming EDX or stack
argument, returns the raw pointer in EAX and uses plain RET. Zero still invokes
actual allocation with zero bytes. For nonzero count, unsigned
`FFFFFFFF / count >= 4` admits exactly counts through `3FFFFFFF`; the endpoint
requests `FFFFFFFC` bytes. No extra maximum, null return, zero shortcut or
initialization is added.

Native rejection builds the null-message 0Ch exception base through BF6340,
publishes D6923C and passes E03CC0 to BF6885. Current `std::bad_alloc` transport
is the existing declared Source boundary, not an emulation of that original
object's message, RTTI, vtable, ThrowInfo, catch identity or private CRT state.

The entire Native body matches accepted `00BCFEB0` after normalizing only three
true CALL rel32 operands; all three decoded Native targets also match. Source
`allocate_native_traffic_vector_0047e970(count)` therefore calls actual
`native_singleton_pointer_allocate_00bcfeb0(count, nullptr)`. Its real body checks
the equivalent unsigned bound, forms the 32-bit byte request and reaches
`singleton_lifetime_allocate`: real malloc, real `_callnewh` on failure, retry
when it returns nonzero, otherwise real current bad_alloc. It shares the
existing malloc/free domain. The canonical API's reserved EDX slot is unused;
the new API exposes only the actual ECX count. No Original EDX input is invented.

This is Source composition, not a Native call edge from 0047E970 to BCFEB0.
Original heap modes, zero-size identity, errno, maximum-request handling,
encoded/global new-handler state, callback counts and arbitrary handler-driven
mutation of compiler stack/request records remain outside the accepted contract.

## Owning length failure and exact cleanup qualification

Original `00490240` consumes no input and has no normal return. It reserves an
uninitialized 1Ch message and 28h payload, initializes only capacity15, length0
and first inline byte NUL, then assigns the actual 18-byte `vector<T> too long`
literal through completed 00408720. It leaves the leading DWORD untouched.
State0 is armed only after assignment returns. Actual 00411700 constructs the
payload; D69260 is published before D83F98/BF6885 throw.

The complete 105-byte body matches accepted `00BD0590` after relocating its
three true CALL operands and the actual handler word. All Native call targets,
message address/count, object offsets, initialization/state order, vtable and
throw-info words match. Handler C62E48 loads FuncInfo D8AAC0 and tails BF6B43.
FuncInfo magic19930522 has one state and map D8AAB8: state0 -> -1 through C62E40.
That whole eight-byte action is LEA ECX,[EBP-50h]; JMP004072D0. Its four fixed
bytes and actual destination match canonical CC5470 after the true JMP rel32
normalization. The handlers/maps agree semantically after their actual address
operands are resolved. Raw relative displacements are not falsely called equal.

The real canonical Source initializes the actual raw SBO, performs counted
assignment, then arms `CompletedMessage`, whose destructor invokes actual
`native_legacy_sbo_string_destroy_004072d0`. It constructs and throws the existing
owning `NativeSingletonVectorLengthError`.

The existing Source owner uses legacy entry 00411700 for construction and
publishes D69260. Its copy provider is 00411940 and destruction provider is
00411780. These use completed base/string providers and real allocation
services. Storage is exactly 28h; the new Source catch type/RTTI/EH owns that
payload. D69260 is verified address data and is never dispatched as a rebuilt
host vtable. No std::length_error substitution or invented exception object,
callback, provider stub, mutable global, guard or allocation domain is added.

The compiler action is qualified Source guard semantics only. It is not a new
callable reconstruction at C62E40, and original private-frame/SEH/FH3/arbitrary
unwind-spill alias identity is not claimed. Native BF6885/private exception
runtime remains outside the established current-C++-EH transport boundary.

## True transfers and callers

At `0047E981` the allocator calls `00BF681B`.

At `0047E9A9` it calls `00BF6340`.

At `0047E9C0` it calls `00BF6885`.

At `00490278` the length helper calls `00408720`.

At `0049028D` it calls `00411700`.

At `004902A4` it calls `00BF6885`.

Fresh live incoming sites are 4924EF/4931CB/49431C for allocation and
4924E6/493169/494313 for length failure. Whole containing routines 4924C0
(71 bytes), 493120 (436), and 4942E0 (148) were reviewed and captured. Each
allocation setup places its computed count in ECX and explicitly zeroes EDX;
return EAX becomes actual vector storage. Length checks consume no extra input
and expect no return. The count comes from EDI, computed growth EDI, or signed
source-distance bits ESI respectively; the call sites do not infer a class.
No implementation or adoption of those callers is claimed.

## Source, build and fixture evidence

Whole candidate/canonical comparisons cover all 190 Native bytes and 48
instructions: 162 fixed bytes match, with only six true CALL operands and the
actual handler word relocated. This is **Native-to-canonical-Native** equality,
not Native-to-emitted-Source identity. The complete own Source object is 12
executable bytes in both strict and registered compilation: a seven-byte
fastcall composition and a five-byte tail transfer to the complete real
canonical operations. All sections, actual symbol offsets and true transfers
are accounted; no NativeSize prefix or padding/count trick is used.

Seven actual provider objects retain their complete CODE/data/relocations in
the audit, totaling 8605 executable bytes. New entry extents and both resolved
tail transfers match the linked fixture PE and actual registered archive origins.
MSVC x86 /MD /O2 /Gy /W4 /WX /fp:strict /EHsc passes. Actual scripts/build.ps1
passes both available CTests (reconstructed_math and tool_tests); optional native
seed-reference testing is not enabled in this worktree.

One ignored focused binding/transport case passes against the real registered
archive: ECX count with junk incoming EDX and RET0 stack balance; real zero-byte
allocation; overflow current bad_alloc; and the owning 28h error with D69260,
actual 18-byte message plus NUL. It uses public real providers, no handler or
allocator simulation. It does not execute original private exception frames,
request a four-gigabyte allocation, or establish native exception/ABI/game parity.
Existing checks, runtime fixture, full static proof and game validation remain
separate. No tracked tests or broad test framework are added.

## Scope and handoff

Both direct gates are ready under the already accepted allocation/string/
owning-exception contracts. Original private CRT/heap/handler/errno/RTTI/ThrowInfo/
FH3/SEH identity, hardware-fault timing, register/flag clobber identity, arbitrary
stack-spill mutation and complete binary ABI remain unproved. No game execution,
visual or gameplay validation is claimed. Whole 493120/495AE0, 49E560, layout
4C71C0, parent4D5BD0 and authored mutable-global producers remain unimplemented
here. Closing child contracts does not reconstruct those parents.

Ignored evidence is `local/goal15t_traffic_vector_failure_children/`: complete
live/installed Native/CODE/EH/data, Source/provider hashes and COFF, compiler,
registered build, fixture/map, self-contained handoff, receipt and full walltime
including qualification, failed static comparison correction, build and overhead.
Reproduce using `build_probe.ps1 -CompileOnly`, `python audit.py`,
`build_probe.ps1`, `python audit.py --reuse-native` and `verify_receipt.py` at that
path; `scripts/build.ps1` is the actual registered build. Reuse-native checks the
installed PE hash against the guarded live capture. Prior R Source/doc/report
and all54 artifact hashes are preserved; its actual archive/object/registration
are saved under `prior_goal15r/`. No Ghidra writes occur here. Primary annotates
the two owned entries under its write lock after release, saves and refreshes.
