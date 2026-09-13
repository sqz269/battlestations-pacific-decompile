# Native render-service parameter initializer

Addresses: 00b0cd80

`initialize_native_render_service_parameters_00b0cd80` reconstructs the complete
300-byte initializer for an actual writable 68h parameter block. The descriptive
name is a hypothesis. Original ECX is the receiver, there are no stack arguments,
EAX returns that pointer, and RET ends the 49-instruction body. There are no
calls, branches, EH records, allocation or ownership changes in this leaf.

At B14BCD the enclosing constructor forms ECX=actual service+2AC. Three sibling
parameter stores intervene before the sole observed CALL at B14BEF. The leaf
writes service+2AC..313. This does not reconstruct the larger 6ACh service
constructor or its global publication.

## Current values and ordering

The new context borrows 21 raw constant DWORD references. It captures their
binding addresses, then reads each referent at the original MOVSS point.
Binding storage must remain fixed and outside the writable destination;
referents themselves may alias that destination. No parameter values are frozen,
no struct initializer snapshots them, and no float conversion changes their bits.

The source preserves all four XMM register roles. CE7804 feeds offsets 0C and
4C even when its referent changes between those stores. The initial D5E134 read
is retained until offset 10; CE380C feeds 14/50, CE3800 feeds 2C/48, and D7A24C
feeds 44/64. Compiled COFF inspection matches all 47 native MOVSS operations:
21 reads and 26 stores, with one store at every aligned offset 00..64.

| Destination offset | Captured actual constant | Original PE DWORD bits |
| --- | --- | --- |
| 00 | 00ce4788 | 41c00000 |
| 04 | 00ce3850 | 40a00000 |
| 18 | 00d5e138 | 3de38e39 |
| 30 | 00ce3854 | 40400000 |
| 08 | 00ce3958 | 40000000 |
| 1c | 00cef1b0 | 3e555555 |
| 34 | 00ce54a0 | 3e4ccccd |
| 0c | 00ce7804 | 3ecccccd |
| 20 | 00ce69c8 | 3e99999a |
| 38 | 00ced318 | 3c360b61 |
| 4c | 00ce7804 | 3ecccccd |
| 24 | 00cf2548 | 3bb60b61 |
| 3c | 00cef0b8 | 3c088889 |
| 54 | 00d7a2f0 | 3dcccccd |
| 58 | 00d7a2f0 | 3dcccccd |
| 10 | 00d5e134 | 3caaaaab |
| 28 | 00d5e130 | 3daaaaab |
| 40 | 00ce3930 | 41a00000 |
| 5c | 00d5e12c | 3b888889 |
| 14 | 00ce380c | 3fc00000 |
| 2c | 00ce3800 | 3f000000 |
| 44 | 00d7a24c | 3f800000 |
| 48 | 00ce3800 | 3f000000 |
| 50 | 00ce380c | 3fc00000 |
| 60 | 00ce3d34 | 40800000 |
| 64 | 00d7a24c | 3f800000 |

The report records every native load/store address, source capture point and
current original constant value. Values are evidence for the installed PE, not
source defaults or recovered semantic field names.

## Evidence and validation

The full native body, 39-byte caller span and 21 four-byte constants match the
saved Ghidra program and original PE: 23 spans, 423 bytes. Ghidra queries verified
the BSP project/program through the repository CLI. Ghidra annotations, ledger
records and CMake registration are reserved for the integrator.

Strict standalone MSVC Win32 compilation passed. The object contains no calls
and preserves the exact MOVSS register/store schedule. The worker baseline build
passed all eight native seed checks and both existing CTests; that baseline
intentionally excludes this unregistered source. One focused original-byte fixture,
linked with the owned source and those three baseline libraries, passed two
original/source comparisons totaling 272 bytes. Source, fixture and all three
library hashes remained unchanged through the run. There are no new repository
tests. The numeric report checker passed zero outgoing call rows; the complete
49-instruction listing independently establishes that this leaf has no calls.

The external fixture is `C:/Users/sqz269/bsp-az-render-service-parameters`.
It compares installed constant values and a deliberate constant/destination
alias case, including later reads of overwritten words and early/held captures,
raw NaN bits, receiver return and eight surrounding canary words. It copies the
300 original bytes to its own executable allocation and relocates only the
21 explicitly verified absolute constant operands. No fixed image reservation,
original game memory, display state or larger service is used. Its default
`run.ps1` compiles only probe.cpp against the selected root's three current
libraries; `-WorkerSource` is only the pre-integration worker mode.

## Boundaries

These are new C++ interfaces with stable borrowed reference bindings. Native
incoming-stack aliases, invalid-address SEH and unrestricted concurrency are
not proved. There is no full-service construction, original-caller ABI, render
or gameplay claim. The larger B14A10 constructor remains incomplete. The alias fixture uses deliberate
operand remapping; it does not establish that the game uses those alias arrangements.
The immutable worker_capture.zip and adjacent preservation_manifest.json retain
source, bytes, recipes, results and exact baseline library copies.

## AZ integration analysis refresh

The integrator saved and read back all 27 AZ original signatures and complete
normal-body ranges, and refreshed exports. CBBBF0 and CBBC10 are ten-byte
analysis-only EH handlers defined under leases and the write lock. Earlier
missing-function observations remain worker capture history. The batch adds
22 complete body records and extends five existing bodies with raw interfaces;
the two EH definitions add no normal-body count. Exact combined validation
follows separately from worker fixture evidence.

## AZ exact merged validation

The exact combined source commit `eedda791230482a4ac7ccd26a6d1f214b72bd6ba` passed the strict Win32
build, both existing tests and five current-library-only original-byte fixtures.
See `reports/native_system_sources_az_validation.json` for hashes, preserved
captures, case coverage and limits. Earlier pending statements describe worker
capture stages. Full rendering, native ABI and general concurrency remain open.
