# Native SAP pair storage, R136

Addresses: 00c3ffe0, 00c37290, 00c37300, 00c4bcd0, 00c32af0, 00c32b10, 00c32b30

## Result

Reconstruct seven complete normal bodies, totaling 761 bytes, over the existing
248h SAP manager, 50h proxy and 10h pair storage. Creation, lookup, per-proxy
append, removal and enumeration now have concrete source implementations.
The source APIs retain the original allocator through `AvoidZoneDynHullMemory`
and reuse R133's indexed removal and cleanup. No complete SAP runtime table is
supplied yet: endpoint updates, incremental processing and the >50 pending batch
path remain dependencies.

## Recovered contracts

| Entry | Normal bytes | Original input and cleanup | Behavior |
| --- | ---: | --- | --- |
| C3FFE0 | 339 | EDI manager, two stack proxies, RET8 | Suppress a found pair; allocate/link a pair; append to both input vectors. |
| C37290 | 109 | ESI proxy, stack pair, RET4 | Grow vector on count/capacity equality, then append. |
| C37300 | 122 | EAX proxy, stack proxy, RET4 | Search the shorter signed-count vector; EAX vector on a tie. |
| C4BCD0 | 139 | EAX first, EBX second, stack manager, RET4 | Find, remove both vector references, unlink and recycle. |
| C32AF0 | 26 | ECX manager, stack pair, RET4 | Return next unless it is manager+C0 sentinel. |
| C32B10 | 19 | ECX manager, RET | Return manager+BC head when raw count is nonzero. |
| C32B30 | 7 | ECX manager, RET | Return raw manager+D0 count. |

Lookup captures both signed counts and the selected vector base. Nonpositive
selected counts return null. A record matches when either endpoint equals the
other input; lookup does not independently validate both endpoints. Consequently
`find(p,p)` can return an ordinary incident pair, and creating a self-pair when
such a pair exists is a no-op. An otherwise empty proxy can receive a self-pair:
creation appends the same pair twice and removal removes both references.

Creation calls lookup with EAX=second and stack=first. A missing free slot causes
a 16,000-byte page allocation, initializes exactly 1,000 next links, and publishes
the free head before growing the page-pointer vector. Capacities grow as
`capacity*2+2`; page and proxy vector copy loops reload their current counts and
old bases. The old vector is freed before publishing the new vector. Other page
bytes and spare vector contents remain untouched.

The active pair is linked before appending its references. Its endpoints are
sorted by raw unsigned pointer value. Vector appends still occur in input order:
first input, then second, even when pointer sorting reverses them. The source
retains allocation/free call-site diagnostics in `NativeDynSapLifetimeProgress`.

Removal captures each vector's signed count separately, with the second capture
after the first removal. It uses the established C40D80 swap-with-last operation,
then rewrites the current next/previous links, decrements the manager count and
pushes the pair onto the free chain. Missing pairs are no-ops; stale fields are
preserved. Count/head enumeration deliberately tests raw nonzero, not signed
positivity.

## Ghidra and byte evidence

Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.
All 1,965 collected live bytes match the original PE: 761 new normal-body bytes,
1,097 existing R133 cleanup bytes, a 75-byte CRT reference and 32-byte SAP table.
Two false no-return overrides truncated the returning free continuations at
C40098 and C372D7. The locked repair restored their two three-byte `ADD ESP,4`
instructions. Final flow checks report zero gaps in all seven new bodies.
Names are descriptive hypotheses. Prior annotations are preserved, evidence is
saved in Ghidra, and affected exports are refreshed.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. An ignored native
comparison fixture executes the collected original bodies with relocated calls
and compares them with the source at the same fixed arena addresses. Six paired
cases match **38,523,008 exact bytes**, **2,240 allocator events**, and **90 full
storage snapshots**, including freed storage retained for observation. All
fixture allocations are released by the end of each case.

Four cases create all 2,016 pairs among 64 proxies, crossing the 1,000- and
2,000-pair page boundaries and multiple proxy-vector capacity boundaries. They
vary static/dynamic pools, input order and proxy cleanup order. Two smaller cases
exercise the same operations with three proxies. Coverage includes duplicate and
absent operations, list head/interior/tail removal, recycled slots, self-pairs,
shorter-list and tie lookup, negative signed counts, raw high-bit count/head,
direct empty-vector append, enumeration, queued proxy cleanup and scalar cleanup.

The initial fixture incorrectly expected a new self-pair while other incident
pairs existed. The original executable rejected that expectation; the corrected
fixture checks the actual behavior and also exercises self-pairs on an empty list.

Both sides share existing manager/proxy constructors. Body records are borrowed
storage unused by these pair operations; this is not a complete world simulation.
The fixture's manager table is deliberately uncalled, and is not a production
runtime table. Normal reverse destruction of three endpoint pools is a controlled
BF7C6E contract; its native exception machinery is not reconstructed.

## Remaining work and limits

Recover C36C30 endpoint allocation, C36E20 insertion, C4BE10 movement and C4BD60
pair decisions, followed by C4C2A0/C4C270/C4C320 and C40140's separate batch path.
Only after both processing paths are concrete can the full SAP table be supplied.

These explicit C++ interfaces do not establish native register/FH3/SEH/RTTI ABI,
allocation-failure behavior, private-stack aliases, concurrency, arbitrary
malformed storage, raw-game admission or gameplay. Callback-driven mutation of
storage and the retained C40D80 unsigned-underflow branch were not exercised here.
See `reports/native_dyn_sap_pairs_r136.json` and its flow companion for exact
CALL rows, byte evidence, annotations, build provenance and sealed artifacts.
