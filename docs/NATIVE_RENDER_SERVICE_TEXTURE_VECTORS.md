# Native render service texture vectors

Addresses: 00B51FC0, 00B52040, 00B52170, 00B521E0, 00B523C0, 00B523E0.

Six complete normal bodies, **502 native bytes / 206 instructions**, operate on
actual raw three-DWORD headers: pointer+0, signed count+4, signed capacity+8.
They provide the array cleanup dependencies of the CCh B52550 texture helper.
The descriptive names are reconstruction hypotheses, not recovered symbols.
No C++ vector, owner allocation, record lifetime or texture resolver is added.

| Entry / inclusive end | Bytes | Coverage | Original ABI |
| --- | --- | --- | --- |
| B51FC0..B5203E | 127 | complete | ECX header, one signed capacity DWORD on stack, RET4 |
| B52040..B520C1 | 130 | complete | ECX header, one signed capacity DWORD on stack, RET4 |
| B52170..B521DF | 112 | complete | ECX header, one signed count DWORD on stack, RET4 |
| B521E0..B52236 | 87 | complete | ECX header, one signed count DWORD on stack, RET4 |
| B523C0..B523D6 | 23 | complete | ECX header, no stack arguments, RET |
| B523E0..B523F6 | 23 | complete | ECX header, no stack arguments, RET |

No semantic return value is exposed. The new public fastcall count/capacity
interfaces take the requested DWORD in EDX. Four small naked entry shims insert
the native writable argument slot before jumping into full instruction kernels.
All native instructions, including padding LEAs, remain in those kernels;
allocator/free calls compose established providers. Destructor entries take ECX
directly. This does not certify original caller ABI or native unwind identity.

## Producers and callers

The fresh B52550 producer listing initializes pointer/count/capacity at actual
owner+24, +7C, +8C and +A4. Owner+24/+7C/+A4 use 24h records; +8C uses0Ch records.
B52550 is supplied the actualCCh allocation by B14EC1/B14EC6/B14EDD. This packet
does not implement that parent constructor or its whole lifetime.

All23 current incoming xrefs and their containing function bodies/argument
setups were captured. The full B529A0 listing establishes the receiver in EBP;
append paths address these same owner subarrays, double current capacity with
DWORD wrap and signed minimum1, and push that requested capacity. The small
append routines B520D0/B52120/B52350 similarly pass doubled current capacity.
Resize callers forward their signed requested count when it exceeds capacity;
destruction passes zero. B52400 establishes EBX=0 atB5243C before its observed
B524BF resize call. Its own later body remains outside this packet's proof.
Existing FH3 actions CBFF38/43/5C and CBFF88/93/AC tail to B523C0 for the three
24h arrays; CBFF4E/9E tail to B523E0 for the0Ch array. No hidden stack arguments
are invented for these destructor tails. The report includes numeric call rows.

## Exact effects

Reserve B51FC0/B52040 clamps a signed request below1 to1, compares signed current
capacity, and returns without reading storage/count when capacity is sufficient.
On growth it forms a native32-bit allocation size, requested times24h or0Ch,
with wrap. It calls BF55BE (tail BF681B), then copies by current signed count.
Both loops re-read the current source pointer and loop bound. The24h copy is
forward REP MOVSD for nine DWORDs; the0Ch copy is three explicit forward
load/store pairs. These are not bulk memmove replacements. A computed null
destination skips that element copy; it does not terminate the loop.

Reserve captures current old pointer only after copying and frees it before
publishing the allocated pointer and requested capacity. The count is left as
currently stored. There is no rollback/free of the new allocation if a later
operation fails. Normal caller/CRT direction-flag conventions apply; the source
retains REP MOVSD rather than pretending every possible DF input is a C++ copy.

Resize first calls its reserve body when signed requested count exceeds current
capacity. It then re-reads count, computes wrapped stride offsets, and zeroes
new records in native word order, including a computed-null slot skip. It reads
the header pointer again for each added element. Shrinking decrements the actual
count one step at a time until requested count is reached, then stores requested
count again. Negative requests are not rejected. If a record aliases its header,
the raw stores and subsequent current reads still occur in the original order.

Destructor calls resize(0), captures the resulting current pointer, frees it,
and returns. Pointer and capacity remain unchanged, even though the pointer may
now be dangling. It does not skip the resize call merely because this parent
usually initializes an empty vector. Aliases to private native argument slots,
new C++ stack frames and allocator-internal storage are outside the supported
source interface; accessible pointed-to raw record/header aliases are retained.

## Concrete allocation and analysis repair

BF55BE is the verified five-byte tail to BF681B's malloc/new-handler retry loop.
The fixed local adapter calls `singleton_lifetime_allocate` with both
native_bytes and host_bytes equal to the SAME wrapped DWORD. This creates actual
CRT storage, not a semantic vector or a replacement allocation callback.
The matching `singleton_lifetime_free` calls the existing CRT free domain.
Allocation failure invokes the current CRT new handler and propagates bad_alloc
when it returns zero. Native CRT exception-object/global identity is not proved.

Root commit5896CB0F repaired these exact native coverage gaps before this work:
B5202F..B52038 and B520B3..B520BC (reserve publication and epilogues),
B523D2..B523D6 and B523F2..B523F6 (returning-free destructor epilogues).
The fresh live listings now include them. All502 bytes match the installed PE.
The worker only read/exported existing analysis; it made no Ghidra mutations,
ledger changes, CMake registrations or shared-source edits.

## Verification and limits

MSVC Win32 /O2 /W4 /WX /fp:strict compiles the source. The emitted assembly is
checked against the206 native instruction sequence; new entry shims and actual
callee bindings are explicit substitutions. All206 native and16 entry-shim
instructions match the emitted MSVC assembly after spelling-only normalization.
All eight native seed rows, the baseline build and both existing CTests passed
at5896CB0F. All29 unique numeric call rows passed, including eight explicitly
classified tail jumps. The baseline excludes this unregistered source; integration must add
it centrally and replay against the resulting current libraries.

One external original-byte fixture lives at
`C:/Users/sqz269/bsp-ba-texture-vectors`. It retains all502 bytes with only six
external CALL operands relocated to the SAME existing rebuilt CRT allocator
and free provider. Internal native relative calls keep their original offsets.
It compares concrete reserve/grow/shrink/destruction, minimum1, wrapped byte
counts, a header aliasing its zeroed record, computed-null record skips, and a
real CRT new-handler failure which mutates the actual header. Heap addresses
are normalized by identity; unwritten newly allocated tail bytes are not
compared as deterministic values. The native CRT bodies themselves are shared
provider assumptions, not independently replayed original CRT code.

The fixture passed **20 original-byte pairs / 464 payload bytes**, plus normalized
header identity/count/capacity and failure/new-handler state. Both original and
source failures reached the real CRT new handler once and propagated bad_alloc;
the handler's writes to current count/capacity remained visible. All recorded
source/header/probe/include/recipe and three library hashes stayed unchanged
across compile/run, and the relocated native code bytes remained unchanged.

`run.ps1 -Repo <integrated root>` defaults to compiling only the external
probe.cpp against that root's three current libraries. `-WorkerSource` is for
the initial unregistered source run only. Linking embeds a manifest. Immutable
worker_capture.zip preserves source/header, actual library inputs, native bytes,
probe/includes/recipes, caller/listing evidence and logs, with verified hashes.

These routines do not complete B52550, render service startup, raw lifetime
integration, original binary ABI compatibility, unrestricted access-fault or
native EH/SEH identity, concurrency, GPU behavior or gameplay. No repository
tests or broad test framework were added.

## BA integration checkpoint

The integrator reviewed the complete native body and actual producer evidence,
saved its original signature and complete stored range in the existing BSP
project, and registered the source. Current combined validation follows
separately from the source or worker checks above. No complete owner lifetime,
original binary replacement or gameplay claim follows from this checkpoint.
