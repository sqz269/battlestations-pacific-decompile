# Model-base bootstrap readiness BG

Addresses: 00CD7EB0, 00CD7F20, 00CE0E60. Evidence-only; no native body credit.

The next bounded implementation packet is ready at the source-contract level:
implement the model-base type static initializer and its separate pool startup/
shutdown adapter in new `include/bsp/native_model_base_bootstrap.hpp` and
`src/native_model_base_bootstrap.cpp`. Integration must first define the two
missing Ghidra functions under the write lock and then annotate/refresh exports.
This worker did not mutate Ghidra, sources, shared ledgers, or build configuration.

## Evidence and boundaries

All live queries used BSP CLI, whose `client()` calls `Client.verify()` before
the request. Local config names `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, bridge `http://127.0.0.1:8089`. Missing bodies were
decoded from disk with `disasm-raw` and checked against live `ghidra bytes`.
The raw listing beginning CD7ED0 starts inside a CALL operand and was discarded
for instruction boundaries; the aligned complete CD7EB0 listing is authoritative.

| Proposed descriptive name | Inclusive range | Coverage | Original ABI |
| --- | --- | --- | --- |
| BSP_ModelBase_InitializeStaticTypeDescriptor | 00CD7EB0..00CD7EFE | complete static body; missing Ghidra function | no stack arguments; RET; no meaningful return contract |
| BSP_ModelBase_InitializeStaticNodePool | 00CD7F20..00CD7F35 | complete static body; missing Ghidra function | no stack arguments; RET; EAX retains atexit status |
| BSP_ModelBase_DestroyStaticNodePool | 00CE0E60..00CE0E69 | complete body | no stack arguments; selects ECX then tail JMP |

Names are hypotheses. CE0E60 is currently named `CG_static_init_00ce0e60`,
despite being the registered destructor. B74EC0 is the model allocator, not
this destructor; its BF ownership and evidence are not changed here.

## Exact type publication

CD7EB0 tests byte01090031; nonzero branches directly to CD7EFE RET. On zero,
CD7EB9 selects ECX=0108FF90; CD7EBE writes guard=1; CD7EC5 writes
01090050=D62DE0. CD7ECF calls B6F110. CD7ED4 loads0108FF90 into EAX and
CD7ED9 loads0108FF94 into ECX **before either destination store**. CD7EDF
writes01090048=EAX and CD7EE4 writes0109004C=ECX. CD7EEA calls006FAC20;
CD7EEF reads old counter+04, CD7EF2 computes old+1, CD7EF5 publishes the
increment, and CD7EF8 publishes old into01090044. There is no local exception
cleanup, rollback, atexit registration, descriptor allocation, or guard reset.
An exception after guard publication leaves a partial descriptor and guard=1.
The guard is shared process storage, not a C++ once flag or a lock.

B6F110's whole body confirms its ECX descriptor contract: guard0108FF54,
name at target+08=D62C7C, call BEA780(ECX=0109DB84), current root copy to+04,
006FAC20 counter postincrement to target+00. BEA780 uses guard0109DB80,
postincrements the same counter, and writes root name D68BBC at target+04.
Thus own/node/root tokens cannot be guessed as fixed numbers. The BF consumer
reads current01090044,01090048,0109004C; its name word is01090050. The separate
01090040 cell belongs to the neighboring object type and is not model-base data.

006FAC20's complete live listing confirms: fast read of0109DB7C; otherwise
00415350, capture manager+10 critical section, enter/increment depth if present,
recheck singleton, BF681B(8) with ADD ESP,4, initialize CFB6C4/+04=0 if nonnull,
publish0109DB7C, call00415350 again, reload published pointer and BD0C30 register,
then release captured section and reload singleton. No atexit is called here.
Allocation/new-handler behavior belongs to the existing singleton provider;
the bootstrap must call it directly and must not add null-success semantics.

## Exact pool startup and shutdown

CD7F20 sets ECX=0109008C; CD7F25 calls B6E980; CD7F2A pushes CE0E60;
CD7F2F calls BF6FF5; CD7F34 POP ECX cleans the one stack argument; CD7F35 RET
leaves EAX status. No local guard or unwind exists. Registration failure leaves
the initialized pool in place. CE0E60 selects the same ECX and CE0E65 jumps
to B6E3D0. The target is actual38h storage, not a pool pointer cell.

Full B6E980/B6E3D0 listings and `native_node_pool_owner.cpp` agree on generic
actual-owner operation: same E188B4 allocator list, D62C78 profile, real embedded
critical section at+0C, lock depth+24, table+28/count+2C/capacity+30/first+34;
startup reserves32 pointers (80h bytes). Shutdown frees slabs/table, drains
positive signed depth, deletes the section, stamps D7A0C0 and unlinks the same
element; it does not destruct live model nodes. Lower allocation and unwind
providers were not re-audited in this evidence-only packet.

## Reuse and ownership contract

Reuse concrete `LightTypeBootstrap::initialize_node_00b6f110` and
`TypeIdCounterLifetime::get_006fac20` from `light_type_bootstrap.cpp`, bound to
the application's same singleton lifetime and actual descriptors/counter slot.
Reuse generic `initialize_native_node_pool_00b6e980`,
`destroy_native_node_pool_00b6e3d0`, and `trim_native_node_pool_00b6ea60` with
the same `AllocatorListDomain`; its source maintains actual links and requires
a matching concrete virtual-zero binding. New pool adapter must register
D62C78/B6EA60 against0109008C before list publication, with real trim dispatch.
Do not call `bind_static_native_node_pool_0108ff58`: that existing adapter owns
one distinct canonical pool binding, which would be overwritten.

The new module can expose borrowed guard/descriptor storage (no constructor
reset), a source static type entry, and a separately bound static pool pair
using real std::atexit and the same raw owner/domain through shutdown. A second
shadow allocator list, synthesized token sequence, opaque success callback,
duplicate singleton, or silent replacement of0108FF58 is outside the contract.
Keep BF model lifetime sources and camera bootstrap files disjoint. Integrator
owns CMake, function definitions/annotations, and sharded metadata.

## Startup frontier and limitations

Live DATA xrefs plus bytes identify CRT-style array entries CE356C=CD7EB0 and
CE3574=CD7F20, with CE3570=CD7F00 between them. This proves table membership
and relative storage order only. The table-walking CRT caller and whole startup
registration range were not recovered here; invocation order is not claimed.
CD7F00/B74B80/CE0E50 (neighbor pool) and earlier bootstrap table dependencies
remain named-but-incomplete upstream work. Existing manager/allocator-domain
installation and shared raw storage wiring are required integration prerequisites;
their source providers exist, but end-to-end game startup is not established.

No tests/build were run because no C++ changed. `verify_report_calls.py` checked
all five numeric edges and exited1: CD7ECF/CD7EEA/CD7F25/CD7F2F fail solely
because the sites are in no Ghidra function. CE0E65 tail jump passed. Integration
must define the missing bodies and repeat this check. This is static readiness, not reconstructed,
build-tested, ABI-compatible, or game-validated completion.
