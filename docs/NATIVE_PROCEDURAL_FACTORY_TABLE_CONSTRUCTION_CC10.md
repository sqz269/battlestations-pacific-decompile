# Native procedural factory table construction (CC10)

The bounded source pair covers BBC900[573] and BBCB40[96]: 669 complete normal
bytes. Fifteen compiler functions contribute 216 bytes. Complete disk decoding
and independently captured live bytes agree with the original PE. The compiler
cluster is 229 bytes, including thirteen INT3 padding bytes that are excluded
from coverage. Descriptive source names are hypotheses, not recovered symbols.

BBC900 takes an actual 10h table in ECX and returns captured ESI with RET.
BBCB40 takes no semantic argument, allocates 10h and publishes the returned
table or zero at actual 01090900. Its incoming ECX preimage is nevertheless
written into its live cleanup cell before allocation. Neither source interface
claims compatibility with those register/private-stack ABIs.

The constructor zeros slots in native order, creates four separately allocated
4-byte profile objects, and registers `CausticsTextureSource`,
`ShoreWaveTextureSource0`, `ShoreWaveTextureSource1`, and
`ShoreWaveTextureSource2` through the existing genuine raw factory providers.
The final profiles are D644E8 and D644F0. Registry node+14 borrows those factory
identities; no retain, rollback, unregister or hidden ownership map is added.

The frame exposes five contiguous initialized live DWORD cells, preserving
their preimages: entry S-20 mask, S-1C cleanup self, S-18 current allocation,
and S-14/S-10 raw name length/data. A raw header view borrows the last two cells
without beginning an owning NativeString lifetime. Allocated factory/table
payloads begin trivial DWORD lifetimes through byte-preserving placement new.
Metadata is separate. One prepared factory frame is reused with four fresh
persistent acquisition records; four separately prepared cleanup frames retain
slot-specific diagnostics. The complete reached acquisition tree is checked
for freshness before any native stores.

For each slot the source preserves allocation/state4/7/A/D, raw41E870 name,
captured EBX bit and name push/state5/8/B/E, current-mask publication, genuine
factory constructor, final stamp, captured slot publication and full state3.
The allocation-null branch skips the name/factory/stamp operations. Normal
name return captures CURRENT data, clears the first three bits in EBX only,
then captures CURRENT length+1 before genuine419CC0/BD1510. The final bit8
remains in EBX; the current stack mask and raw header are not cleared.

The complete DFEA64 unwind map is retained:

| State | Previous | Action |
|---:|---:|---|
| 0 | -1 | CC4BD0, current self+0, BBC4E0 |
| 1 | 0 | CC4BD8, current self+4, BBC500 |
| 2 | 1 | CC4BE3, current self+8, BBC500 |
| 3 | 2 | CC4BEE, current self+C, BBC500 |
| 4 | 3 | CC4BF9, current allocation free |
| 5 | 4 | CC4C04, current mask bit1/raw41DD20 |
| 6 | 3 | CC4C04, current mask bit1/raw41DD20 |
| 7 | 3 | CC4C1D, current allocation free |
| 8 | 7 | CC4C28, current mask bit2/raw41DD20 |
| 9 | 3 | CC4C28, current mask bit2/raw41DD20 |
| 10 | 3 | CC4C41, current allocation free |
| 11 | 10 | CC4C4C, current mask bit4/raw41DD20 |
| 12 | 3 | CC4C4C, current mask bit4/raw41DD20 |
| 13 | 3 | CC4C65, current allocation free |
| 14 | 13 | CC4C70, current mask bit8/raw41DD20 |
| 15 | 3 | CC4C70, current mask bit8/raw41DD20 |

Each prior state is consumed before the real action. Every slot cleanup reads
CURRENT cleanup self separately, then invokes the sibling real slot provider.
Allocation cleanup frees CURRENT allocation; mask cleanup tests and clears
CURRENT mask before throwing raw41DD20 on the CURRENT header. The normal body
does not arm states6/9/12/15. A second source cleanup failure stops further
actions and retains consumed state and residual diagnostics.

Partial raw-name construction before mask arming can retain an unarmed name
allocation. Failure in the normal getter after slot publication/full state3
cleans slots only and retains the outstanding name credit. Successful factory
registration may survive later factory free as a dangling borrowed registry
entry. Caller must quiesce all lookups and dispose/reconstruct the genuine
registry; the source invents no rollback or retained ownership. Acquisition
records and live backing remain available for explicit failure disposition.

BBCB40 seeds its incoming ECX cleanup preimage, captures the allocation, arms
state0, then calls the constructor or publishes zero. On source failure the
constructor's already executed cleanup precedes a free of CURRENT wrapper
allocation. It does not call a second table destructor, write publication, or
release a prior published table. Successful replacement can leave that prior
publication caller-owned; normal table shutdown can leave publication dangling.

The frozen readiness archive is
`local/output/cc10_procedural_table_construction_readiness_evidence.zip`, SHA256
`0b3b14da60de0610af60fa44cf00105dc497df5e4b68bf816f1a0e2094d4c65d`.
Its 98 entries pin 97 input rows, 19 source/API identities and 37 provider
transfers separately from internal JMPs. It remains immutable. Root owns
Ghidra repair/definitions/names; this worker performs no Ghidra mutation.

Validation results and exact artifact identities are recorded in
`reports/native_procedural_factory_table_construction_cc10.json`. Exported,
reconstructed, build-tested and focused fixture evidence are separate levels.
The final strict MSVC Win32 build and all three existing CTests passed. The
standalone SOURCE ONLY genuine-provider composition passed four derived slots,
borrowed registry relationships, publication, normal captured operands despite
current-header mutation, and a separate source state3 failure which redirects
CURRENT cleanup self, retains current mask/name credit and frees the wrapper's
CURRENT allocation. The host throw follows a completed real getter; it is not
a genuine getter failure or native exception comparison. Active assertions and
the embedded manifest are preserved with exact final source/object/lib/exe
identities. No tracked tests were added.

The copied 669-byte native comparison is explicitly **inconclusive**. The first
probe refused an unavailable exact reserve; the single diagnostic revision
found CE0000 in a reserved MEM_PRIVATE region based at CC0000 (error487), while
01090000 was free. No occupied memory was replaced and no further mapping
attempts occurred. The successful source-only mode uses borrowed live name and
publication cells, performs no fixed-address reservation and executes no copied
code. Probe01 retains its log but lacks preserved object/exe identity; it is
uncredited. Probe02/03 and the final attempt retain inputs and binary identities;
superseded single-read mask source is uncredited. Only the final corrected
two-read mask implementation receives source/build evidence.

Native FH3/SEH, saved registers, hardware faults, global/application startup,
resource34h, F8D420, source0 activation and game behavior remain unproved.
