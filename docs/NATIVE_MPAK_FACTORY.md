# Actual MPAK factory and publication

Addresses: 00736B60, 007370D0, 00735D30, 00735840, 00BB83A0.

The five complete bodies in `native_mpak_factory.hpp/.cpp` use the application's
actual singleton manager, factory, lock and cached-provider publications. The
factory owns no semantic domain, private pool or implicit provider dispatcher.
BB8240 construction and BB82F0 cache replacement are explicit operations supplied
by the actual provider implementation. This packet adds no archive parser.

| Inclusive native span | Bytes | Operation | Original ABI |
| --- | ---: | --- | --- |
| 00736B60..00736C2D | 206 | Factory getter | no inputs, EAX factory, RET |
| 007370D0..00737109 | 58 | Primary scalar deleter | ECX primary, flags stack, EAX captured primary, RET4 |
| 00735D30..00735D37 | 8 | Secondary deleting thunk | SUB ECX,4/JMP7370D0, inherited RET4 |
| 00735840..00735868 | 41 | Temporary base scalar deleter | ECX base, flags stack, EAX captured base, RET4 |
| 00BB83A0..00BB8516 | 375 | Factory Create | ECX unused factory; system/unused virtual headers stack, EAX result, RET8 |

All 688 original bytes are accounted for. The primary repaired the returning-free
continuation737101..737103 and defined735840..735868, preserving old comments and
library names, saving and refreshing exports. This worker made no Ghidra writes.
The source uses new C++ interfaces; it is not a binary replacement.

## Storage and captured publications

736B60 is the producer of the raw8h factory: temporary CFEA00 at+4, primary CFEA20
at+0, then secondary CFEA1C at+4. Readback verifies CFEA20 slots7370D0/BB83A0,
CFEA1C slot735D30, and CFEA00 slot735840. The publication is010904D4; lifetime
registration belongs to the existing raw01090AA0 manager.

The first nonnull factory read returns without consulting the manager. Otherwise
the getter captures the first manager's physical section+10, enters and increments
that captured section+18, then rechecks publication. Allocation uses the existing
BF681B new-handler service. After publication it reloads the factory and captures
its secondary+4 before the second manager getter and BD0C30 registration. Both
normal leave and the guard cleanup use the original captured section. Failed
registration retains the published allocation. No reference is acquired.

7370D0 clears the publication, writes secondary CE3818 before primary CFE9F4, then
optionally frees according to flags bit0. The secondary thunk subtracts4; the base
deleter receives its allocation directly and writes only CE3818. None unregisters
itself, and null input is not made safe by the source.

## Create and the provider boundaries

BB83A0 captures the unsigned original header length. Length>4 invokes the complete
469840 substring with start=length-5/count=7FFFFFFF, then425850 compares the returned
header to CEBA84 `.mpak`. The actual helpers preserve null-data, signed-start,
DWORD-wrap, embedded-NUL and CRT case-insensitive behavior. Bare `.mpak` therefore
passes this gate. Current suffix data and length are released before lock access;
declines do not read010904E0 or010904DC.

BB40B0/BD1860 establish that010904E0 holds an actual1Ch tracked critical section.
Create captures this lock for Enter and the subsequent+18 increment, then reloads
the current publication. It captures cached provider010904DC before decrementing
and leaving the reloaded lock. These reads are distinct: callback mutation can
leave the first lock held, and the source does not repair native ownership.

For a captured cached owner, Create constructs an empty actual8h string through
41E870, calls `replace_cached_00bb82f0` with that header alone, cleans up its current
data/length, then returns the captured owner. BB82F0's full assembly consumes ECX
as the header, overwrites incoming ESI before use, takes no stack arguments and
returns RET0. Its empty-name branch clears the global cache under the current lock.
This corrects the abbreviated direct-cached-return description in
`APP_INIT_VFS_SINGLETONS.md`; Create does not merely return the cache.

With no captured cache, Create allocates raw44h and calls
`construct_00bb8240(allocation, original_system_header)`. The original header
pointer is forwarded without copying or normalization. Returned EAX is forwarded,
not forced to the allocation. Native ECX supplies the owner and RET4 consumes the
header. Factory/virtual-header arguments are unused. The complete provider/parser
graph remains a separate primary-owned dependency, never a successful stub.

## Exception evidence and limits

Getter FuncInfoDB553C/mapDB5534 has state0 C86060..C86067 ->411EE0, armed after
section entry. Create FuncInfoDFE34C/mapDFE33C has two independent states lowering
to-1: state0 CC4760..CC4767 ->41DD20 for the current empty-string header, and state1
CC4768..CC4772 frees the captured allocation. CC4771..CC4772 (`POP ECX; RET`) is
outside the Ghidra action body and is retained as a raw inclusive tail, not another
function. Neither suffix cleanup nor lock entry is an armed Create EH owner.

The source expresses the C++ cleanup order. Existing actual-string release is
noexcept; throwing native lazy-pool cleanup, double cleanup exceptions, original
FH3/SEH identity, hardware faults and arbitrary aliases into compiler spills are
outside the interface. Allocation-null paths remain present but the established
new-handler service normally throws on exhaustion. No installed MPAK archive,
complete provider loading, runtime startup or gameplay is claimed.

## Verification

The focused driver is `local/mpak_factory_ay/run_fixture.py --repo BUILT_REPO
--attempt NEW_DIRECTORY --build-log BUILD_LOG`. The fixture uses genuine actual
manager/pool lifetime, the existing shared allocator and real Win32 sections.
BB8240/BB82F0 are explicitly recorded call boundaries, with opaque owner storage;
they do not produce a valid loaded provider or archive. Native bodies stay
byte-identical in private relocated code pages; existing actual helpers are ABI
bridges. Source-only exceptions do not execute native FH3/SEH.

Strict MSVC Win32 `/W4 /WX /fp:strict` build01 and both existing CTests passed.
The preliminary build00 failure is retained: changing the deferred registration
during the warm-up build caused concurrent CMake regeneration/FetchContent
failure. The completed source and registry were stable throughout build01.

Attempt02 passed three original/source comparisons and two source-only cleanup
checks, totaling 1,034 assertions. It covers cold allocation and original-header/
EAX forwarding, cached bare `.mpak` acceptance, publication changes during real
Enter/Leave, captured cache return, current temporary-header cleanup, decline
inputs and primary/secondary/base lifetime variants. Both source-only provider
operation exceptions release their correct owner. The cached exception also
verifies that the changed string block can be recycled from the actual pool.

The cached test deliberately republishes the lock during Enter and the cache
during Leave. It records first-lock depth 1/current-lock depth 0 and checks real OS
recursion before explicitly balancing the first lock for teardown. This harness
normalization occurs after the comparison observations; no production ownership
change is implied. Every observed allocation and canonical registration drains.

Attempt01 remains read-only after its cold comparison passed and the cached
boundary rejected an overly strict fixture assumption that an empty actual
string had nonnull data. The genuine constructor permits `{0,null}`. Only that
fixture assertion changed before attempt02; production code stayed unchanged.

Before execution, 494 inputs were physically sealed, including 122 map-derived
linked objects equal to their current archive members and 194 dependent BSP
headers. All 503 successful-attempt files are read-only, every output is accounted
for, and a final read-only audit rehashed source/header/object/library/native-byte
parity without another native execution. The report retains exact hashes, all 24
CALL/tail rows (20 checked,4 IAT),37 relative transfers, prior names/comments and
the two source exception boundaries. Primary owns the central annotation batch.
