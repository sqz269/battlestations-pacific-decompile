# Raw procedural factory base storage

This packet composes actual four-byte factory construction/registration and
scalar disposal. Five complete normal bodies total 235 bytes; four compiler
boundaries total 36 bytes. It uses the existing raw resource registry and
insertion providers. It does not construct the four-slot factory table, publish
procedural globals, create/admit 34h resource owners or activate later startup.

| Entry | Exclusive end | Bytes | Native interface |
|---|---|---:|---|
| BBC5F0 | BBC643 | 83 | ECX factory, stack current name pointer; RET4/EAX factory |
| BBC740 | BBC793 | 83 | ECX factory, stack current name pointer; RET4/EAX factory |
| BBC440 | BBC447 | 7 | ECX factory; base profile stamp; RET |
| BBC650 | BBC66F | 31 | ECX factory, stack flags; RET4/EAX captured address |
| BBC7A0 | BBC7BF | 31 | ECX factory, stack flags; RET4/EAX captured address |

An actual factory is one live DWORD. There is no count at +4, host wrapper,
canonical companion, retain/release or implicit ownership registration. The raw
registry's actual node+14 borrows its address. Caller backing must remain usable
through all lookups, or lookups must be quiescent before factory disposal.
Erasing registry keys/nodes does not dereference or destroy their factory values.

The constructor context borrows the actual manager01090AA0 and registryF8D41C
publication cells, the same raw insertion/string-pool context, and current
eight-byte profile storage for D5E594/D5E59C. Both verified current slot+4 entries
are B1B3A0. Numeric profile resolution is side-effect-free; the reached table's
current +4 is read before invoking that genuine body. Missing/unknown profile or
target is a source diagnostic, with no generic callback or success fallback.
Callable genuine getter/CRT providers and valid reached storage are required.

The immutable frame binds initialized live DWORD cells for incoming name and
cleanup-self, plus the existing registration frame. With S=entry ESP, name is
S+4, cleanup-self is S-10 (EH EBP-10). PUSH ECX seeds that local; the subsequent
MOV repeats captured ESI before state0 and profile stores. The source preserves
both writes. View metadata and persistent fresh diagnostics are disjoint from
all native cells; no aggregate is cast over reused scratch. Nested provider
frames are supplied at their own native call sites, without a claim of private
stack-address coincidence, saved register or original caller ABI equivalence.

BBC5F0/BBC740 capture the four-byte factory, arm state0 and stamp respectively
D64470/D644AC. They call genuine B1B730. Only after it returns do they capture
the CURRENT incoming name, read CURRENT returned-registry profile, and read
CURRENT captured-profile slot+4. They retain the getter's returned registry
identity even if F8D41C changes. Native pushes captured factory before captured
name; the source seeds those nested argument cells in that order and calls real
B1B3A0. It returns the captured factory without a later profile stamp.

The getter may allocate, publish and register a genuine registry through its
raw manager. Its established source failure/guard cleanup policy remains
unchanged. Diagnostics record getter entry/normal return and captured result;
they do not invent visibility into the getter's private unwind state. The
nested insertion diagnostics persist alongside all completed tree mutations,
publication state and outstanding provider credits. The new constructor never
installs a manager deletion binding. A manager containing this registry must
already receive real same-publication/string/CRT `resource_registry` bindings
before drain.

BBC440 writes D64468 to the actual four-byte receiver and returns. BBC650 and
BBC7A0 test bit0 of the CURRENT flags LOW BYTE before that stamp. They call real
CRT free only according to that captured test, then return the captured pointer
bits with no post-free payload read. Flags0 leaves caller-owned backing with the
base profile; it does not grant a new lifetime or imply a second scalar call.
No unregister, publication clear, callback, field+4 access or extra cleanup is
added. The optional BBC460 base scalar and BBC890/BBC8E0 derived scalars remain
outside this packet.

DFE998/maxState1/mapDFE990 has state0 -> -1/CC4B50; DFE9F0/maxState1/mapDFE9E8
has state0 -> -1/CC4B90. Both have no try map and flags1. The eight-byte cleanup
funclets read CURRENT EBP-10 and tail-jump to BBC440. The ten-byte dispatchers
CC4B58/CC4B98 load their respective descriptors then tail-jump to BF6B43.
On source propagation, the constructor consumes state0 once, stamps CURRENT
cleanup-self and rethrows. It does not free the allocation/name, undo registration
or erase residual credits. Its unsupported-binding diagnostic uses the same
explicit source cleanup policy outside the qualified native domain. On success
the diagnostic state remains the final native state0, marked completed; it is
not an instruction to run a deferred destructor.

Fresh Ghidra still lacks four functions: BBC650 through inclusive BBC66E
(last RET4 BBC66C/3B), BBC7A0 through BBC7BE (BBC7BC/3B), CC4B58 through CC4B61
(last JMP CC4B5D/5B), CC4B98 through CC4BA1 (CC4B9D/5B). Full live/PE bytes,
bounded listings and exact call rows are recorded for primary definitions.
The worker performed no Ghidra mutation.

Strict MSVC Win32 `./scripts/build.ps1` and both configured CTests passed. One
ignored standalone copied-original/source fixture compiled and passed on its
first attempt. It rejects NDEBUG at compile time and uses /MD /EHsc /std:c++20
/O2 /Gy /W4 /WX /fp:strict with /link /OPT:REF /MANIFEST:EMBED. No tracked tests
or production callback seams were added.

The fixture uses genuine isolated raw manager/registry/name pool construction,
actual four-byte CRT allocations with explicit DWORD lifetime, and real manager,
registry and CRT disposal. Two mapped eight-byte registry profile cells contain
the pinned native values. A B1B3A0 trampoline invokes the genuine source provider
with prepared live frames. Original constructor profile loads and indirect
calls remain unmodified; only two getter and two free CALL rel32 operands are
relocated. All five complete native normal byte ranges are copied, and the
seven-byte base stamp is compared directly over a receiver plus adjacent canary.
Original compiler EH is not executed.

At the genuine getter return boundary the fixture changes the actual incoming
name cell and returned registry profile, and once clears the publication. It
checks the correct names, captured returned registry identity, actual borrowed
factory node values and duplicate cardinality/value preservation. Scalar cases
cover flags0/1, observe the base stamp before genuine free, change the current
flags cell during free, and compare returned freed pointer identities without
dereferencing them. Registry/manager draining does not dispose the borrowed
factory allocations. This is a selected relationship/store comparison, not
whole-tree byte, private-register/stack or end-to-end application equality.

A separate SOURCE-ONLY case completes the genuine getter, then the observed
provider seam changes cleanup-self and deliberately throws. It verifies cleanup
of that current receiver, the original factory's completed profile prefix,
unregistered/empty registry state and explicit caller disposition. This tests
the source exception projection, not a failure of the genuine getter, original
FH3 execution, hardware fault behavior or native double-exception transport.

The report and ignored `local/output/cc10_procedural_factory_storage/` evidence
pin all ranges, compiler maps, ten transfers, exact build/probe command and
source/object/library/executable identities. Frozen readiness ZIP SHA-256:
`c10bc394f704bb114fd662a4678306a42233707240d1698e0398e79e0eb00cb9`.
Table/derived lifetime, 34h resource admission, procedural-global publication,
native FH3/private ABI, original executable and application behavior remain
explicitly outside this packet.
