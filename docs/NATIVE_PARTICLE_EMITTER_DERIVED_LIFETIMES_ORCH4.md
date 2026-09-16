# Raw derived emitter lifetime (orch4 l12)

Six genuine raw overloads in `src/native_particle_emitter_derived_lifetimes.cpp`
compose the same `NativeParticleTypeLifetimeContext` and published AFA100 emitter
base. The existing `NativeParticleDefinitionBindings` APIs and their source are
unchanged. Names are descriptive hypotheses, not recovered symbols. Prior live
names/comments and full prior ledger records are preserved in the paired report;
the raw overloads extend the existing reconstruction rows rather than count six
already reconstructed native entries again.

## Exact bodies and schedules

| Body | Inclusive span | Bytes/instructions | Profile and parameters |
| --- | --- | --- | --- |
| Cone destructor | B039F0..B03AB3 | 196/56 | D5DEBC; +80/+84/+88/+8C/+90 |
| Sphere destructor | B02C40..B02CD3 | 148/42 | D5DE88; +80/+84/+88 |
| SmartArea destructor | B01D60..B01E1F | 192/51 | D5DE48; +80/+84/+88/+8C |
| Cone scalar | B03B40..B03B5D | 30/11 | calls B039F0 |
| Sphere scalar | B02FB0..B02FCD | 30/11 | calls B02C40 |
| SmartArea scalar | B01EA0..B01EBD | 30/11 | calls B01D60 |

The destructors receive the raw owner in ECX and RET, without a specified EAX
result. Their factory allocations are 94h/8Ch/90h respectively. Every scalar
receives ECX owner and stack flags, calls its complete destructor, frees the
original owner through the canonical BF65AC contract iff flags bit0, returns
that original address in EAX, and RET4. No free occurs when destruction throws.

Each destructor stamps its numeric profile, captures +80, then arms state0.
For every nonnull captured parameter, call AFFDF0 and then B00090 with that SAME
captured pointer. Only after both return is the next owner member read. There
is no parameter array snapshot or second member read between disposal and pool
return. The canonical parameter type is uint16 at +0A, and its real pool slab
index remains the separate +0C word; these are parameter-provider contracts.

Cone and Sphere leave all derived parameter member values stale. SmartArea
leaves +80/+84 stale, but clears CURRENT +88 and +8C after each respective
nonnull captured slot returns successfully. A null slot causes no store. These
clear operations must neither move before cleanup nor appear on an unwind path.
All other derived bytes stay untouched. The real AFA100 base then owns common
parameter/string/retained-object cleanup through the same raw pools and its
proven current-profile dispatch. No extra owner, allocator, or generic destructor
callback is introduced by these overloads.

## EH and profile evidence

| Family | Handler | Full FuncInfo | Map | State0->-1 action |
| --- | --- | --- | --- | --- |
| Cone | CBB698 | DF377C | DF3774 | CBB690 -> AFA100 |
| Sphere | CBB5C8 | DF3688 | DF3680 | CBB5C0 -> AFA100 |
| SmartArea | CBB4F8 | DF3594 | DF358C | CBB4F0 -> AFA100 |

All three maps contain one cleanup state. Each action reloads the saved owner
from EBP-10 and tail-jumps to AFA100; derived slots are not retried or completed.
Normal execution disarms before calling AFA100. The C++ noexcept guard therefore
cleans the base on an escaping derived failure and terminates if that cleanup
throws a second C++ exception. It does not rerun a failing normal base destructor.

D5DEBC, D5DE88, and D5DE48 hold BD30E0 at slot0 and the corresponding scalar at
slot4. Original numeric tables remain ABI evidence; callers require the genuine
source dispatch chain rather than treating an original code address as callable
host code. This packet does not alter the resource or runtime emitter container.

The report records all 626 original bytes and complete live/PE hashes, all 33
direct body calls and three cleanup tail jumps, complete EH spans, and profile
pairs. Entry-aligned disk decoding verifies each full body through its final RET.
All six initial live listings already had zero gaps; no Ghidra mutation or repair
was required. Queries used the existing `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`.

## Validation boundaries

The strict MSVC Win32 build and all three CTests passed after verify-seeds.
The report's 36 direct-call and cleanup-tail rows passed verification. The
ignored local fixture passed with all 626 copied original body bytes, actual
raw parameter/string pools, the genuine emitter base and Layer scalar cleanup,
and fixed heap dependencies. It compares complete owner images for retained
owners, checks actual derived-then-base slot return order and type1/2 payload
disposal, verifies foreign and surviving retained children, and exercises all
three scalars with flags2 and3. SmartArea-only member clears match the original.
The source, fixture, build log and dependency hashes are in the report. No
permanent test suite is added.

The old host-binding reconstruction remains a companion implementation; its
earlier validation is retained as historical scope. The new owning C++ interfaces
are not binary entry thunks. Original FH3/SEH, exceptional execution, asynchronous
faults, concurrency and gameplay remain unproved by the normal probe. Cleanup
schedules are independently supported by the original assembly and complete maps.
