# Actual checked DWORD vector insertion and append

This packet reconstructs whole ordinary entries `00697DC0` (137 bytes, 56
instructions) and `00697F40` (131 bytes, 53 instructions). Fresh typed bodies,
live bytes, installed PE bytes and every current instruction start agree.
The original library identities `STL_Vector_InsertOne_00697dc0` and
`LIBSTL_CheckedDwordVector_Append_00697F40` remain unchanged. New Source names
are descriptive hypotheses. The old qualified append adapter remains separate.

The actual 16-byte header has opaque/begin/end/capacity at offsets 0/4/8/C;
elements are unowned four-byte DWORDs. Both wrappers use raw DWORD MOVs and
unsigned modulo-2^32 arithmetic with explicit x86 SAR2 distances. They introduce
no object overlay, provider, iterator class, snapshot, extra guard or rollback.
The directly reused insertion is accepted pin `12c552ba08279135011315e172d498ecc5f105f4`;
its actual allocation, length exception, progressive ranges and catches retain
their own Current C++/CRT qualifications. No new allocator is introduced.

`00697DC0` receives the header in ECX, ignores incoming EDX and stacks output,
iterator owner, position and value address. Its single exit is RET10h and EAX
returns the original output pointer. Capture owner and initial begin. A null
begin, or a zero SAR2(end-begin), sets index0 and skips preliminary validation.
Otherwise compare captured begin/end, report invalid order, then report an
absent/different captured owner. If reporting returns, keep those captures and
derive index from the position and initial begin. Forward the value ADDRESS and
count1 to the actual insertion; discard the child's ambient EAX.

After insertion, capture new begin and compare it to CURRENT end. Keep that
begin if reporting mutates the header. Compute wrapped newbegin+index*4, compare
it to a fresh end, and only when result<=end read/compare fresh begin. Retain the
result across reporting. Write output+4 position BEFORE output+0 receiver and
return the original output pointer. The native scratch overwrite of its own
position stack word is private-frame evidence, not another Source storage API.

`00697F40` receives header in ECX, ignores EDX, stacks one value address, and
both native exits RET4. Caller continuations discard its inconsistent ambient
EAX; the Source result is void. Capture initial begin, conditionally derive size
from CURRENT end, then conditionally derive capacity from CURRENT capacity.
The comparison is unsigned even when a SAR2 result has its high bit set.

With spare capacity, forward the physical value address without reading its
DWORD early, capture CURRENT end, and invoke actual uninitialized fill_n with
count1. The core's null-output skip and progressive value read remain intact.
Publish CAPTURED end+4 after the child, without a current-end reload or RMW.
Only the native tag's low byte is initialized; its high24 bits are dead in the
complete child. Source passes dead0 without evaluating indeterminate bytes.

On the slow path capture CURRENT end, compare INITIAL begin against that end,
and retain the captured end if reporting returns. Use actual InsertOne with
owner=receiver, that position and private8B output scratch; ignore its result.
Scratch words are never read by append and require no owner/destructor.

All eight owned native transfers are independently decoded:

- `00697DE6 CALL 00BF6713`: initial begin/end diagnostic.
- `00697DF3 CALL 00BF6713`: captured iterator-owner diagnostic.
- `00697E10 CALL 00697990`: actual count1 insertion.
- `00697E1D CALL 00BF6713`: captured new begin versus current end.
- `00697E33 CALL 00BF6713`: computed result bounds diagnostic.
- `00697F89 CALL 006970D0`: spare-capacity uninitialized fill_n.
- `00697FA3 CALL 00BF6713`: initial begin versus captured slow-path end.
- `00697FB6 CALL 00697DC0`: slow insertion with private output.

Neither complete body establishes a local EH frame, action, catch or owned
temporary. Child C++ exceptions propagate; this packet adds no catch or
cleanup. The accepted invalid-parameter service uses the selected current CRT
policy and may return, throw or terminate. Returning diagnostics preserve the
specified captured values and subsequent current header reads.

Reached raw backing must permit the original accesses and accepted child
termination. Header and output must remain writable through the specified
operations; public output/header/value aliases retain ordered raw writes.
Private Source argument/scratch aliases, native stack layouts, dead tag bytes,
register/flag residue, original FH3/RTTI/heap/CRT identity, faults/SEH,
asynchronous mutation and binary/game runtime equivalence are not established.
Matching physical formals and compiled RET shapes do not establish native ABI
compatibility. No original data or Ghidra analysis is modified by this worker.

The report pins exact evidence, Source hashes, compiled physical-interface
inspection and the existing MSVC Win32 build/test result. The three existing
suites provide their existing coverage; no new wrapper fixture, native
execution or gameplay test is claimed. Primary frozen-pin review is separate.

Compiled COFF inspection confirms standalone InsertOne RET10h/EAX output and
position-before-owner writes. Append retains both RET4 exits, saved-end+4
publication and late current header reads. MSVC inlines the same-TU InsertOne
Source call into append's slow path; the emitted standalone entry remains.
Its expanded diagnostics and direct count-insertion relocation are recorded,
not mistaken for another native transfer or a handwritten replacement. Neither
wrapper has current compiler EH tables. This is Source compilation evidence.
