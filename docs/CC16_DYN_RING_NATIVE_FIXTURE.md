# Focused original-machine ring-family fixture

`tests/cc16_dyn_ring_native_fixture.cpp` compares original installed-image
instructions against the bound C++ family at frozen private candidate
`2f76638f6d2d0dfaa3caac8bb85ad7fa7b776cb3`. Both routes receive the same
actual input-owner record and packed point buffer. One fixed nondegenerate
concave-five polygon replaces the initial convex-four proposal, as
authorized in the followup: `(0,0), (2,0), (2,2), (0.8f,0.9f), (0,2)`.
There is no second geometry, random corpus or implementation-shaped oracle.

## Infrastructure and original reference

The existing `verify_cc13_leaf_reference.py` and
`cc13_leaf_fixture_tests.cpp` supplied the installed/live byte-audit,
strict-relocation and private-native-ABI pattern. The existing VFS fixture
supplied isolated process/manifest conventions. The leaf suite has no
ready fixture for this constructor/provider graph; expanding that broad
suite was unnecessary. This single-purpose fixture uses a fixed nine-span
table and rejects a changed installed image, with no general loader API.

The installed PE SHA256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All nine spans were compared byte-for-byte with the saved BSP program via
`bsp.py ghidra bytes`, whose client verifies project/program identity.
They are `00C33510`, `00C4D5A0`, `00C4D370`, `00C410E0`, `00C41190`,
`00C41230`, `00C377F0`, `00C37860` and `0040D7B0`; exact lengths and
individual hashes are in `reports/cc16_dyn_ring_native_fixture.json`.

The packed original-byte payload is 2,876 bytes before relocation, SHA256
`9e549deab27f23f5b442846e9c2aea69ad8e4aadbb79d682ee7b2f885bdd1f61`.
Capstone checked every instruction boundary and rejected unknown calls,
indirect flow and absolute image data. The complete normal-call graph has
14 private-body calls, ten allocation calls and ten free calls. One
additional fixup is the constructor's handler immediate. No unknown native
call or data dependency remained. The executable reads original bytes
from the hash-pinned installed PE at runtime; no duplicate math is the
oracle. Its copied code is private RX memory after relocation.

`python tools/ghidra_export.py verify-seeds` passed all eight ranges before
native execution and was repeated immediately before launching the probe.

## SEH and heap boundaries

The constructor's ordinary FS:[0] registration/restoration remains in the
original instructions and is checked on return. The immediate at
`00C4D5A2`, originally handler `00CC82DB`, is relocated to an owned
nonreturning failure handler. An own-process vectored handler also rejects
any exception while either kernel route executes. Exceptions never produce
an oracle result. Original EH dispatch, cleanup and failure behavior are
explicitly excluded; no original personality or unknown cleanup body is
stubbed to return a value. `/SAFESEH:NO` applies only to this probe image,
so its failure handler can be used without changing system settings.

The heap binding is **synthetic-normal-contract**: actual `std::malloc`
and `std::free` satisfy the known cdecl byte-count/pointer contracts. Fixed
test-only recording storage assigns logical allocation IDs. Native and
C++ raw pointers differ and are not compared as game/CRT identities. The
same real callbacks back both routes, with no fabricated geometry or
allocation result. Every remaining test allocation is released after its
snapshot, outside the compared constructor trace. Allocation failure and
original CRT retry/exception behavior are not tested.

## Observed normal-path result

The one geometry passed under PC24 and PC53, both round-to-nearest with
masked exceptions and normal MXCSR denormal controls. Both routes matched
all scalar count/cursor words, vector sizes/capacities, active contents,
logical allocation identities and ordered heap events. Both modes yielded:

| Storage | Original-machine result, matched by C++ |
|---|---|
| count / anchor / next cursor / previous cursor | `2 / 4 / 4 / 0` |
| next, size/capacity 5/5 | `[4,2,3,4,0]` |
| previous, size/capacity 5/5 | `[4,0,1,0,0]` |
| scratch, size/capacity 0/6 | no active elements |
| chains, size/capacity 2/2 | `[0,1,2,3]` (4/4), `[0,3,4]` (3/3) |
| pairs, size/capacity 1/2 | `[[0,3]]` |

The emitted pair implies both predicate calls were reached by the parent's
control flow; this is not an instrumented instruction-hit trace. There
were eight allocations and one constructor-time free, in this order
(`A` = allocate, `F` = free; ID and requested bytes follow):
`A1:20, A2:20, A3:8, A4:24, F3:8, A5:16, A6:24, A7:16, A8:12`.

The native shim also checked EBX/EBP/ESI/EDI canaries, ESP restoration and
EAX destination return. Both routes preserved FS:[0], x87 TOP and FP
controls. The probe restored its saved x87 control word and complete
MXCSR after both modes. FP status/exception identity and the original
binary ABI of the bound C++ public entry are not claimed.

## Build, process receipt and reproduction

MSVC x86 C++17 compiled the fixture plus the frozen binding, owner,
preparation, predicates, query, initializer, ambient-math and
`cc15_dyn_live.cpp` translation units with
`/permissive- /EHsc /fp:strict /O2 /Gy /Gw /W4 /WX`. Linking used
`/MACHINE:X86 /MANIFEST:EMBED /SAFESEH:NO /OPT:REF /INCREMENTAL:NO` and
`bcrypt.lib`. No kernel, CMake or ledger file was changed.

The safe-name executable was `local/cc16_dyn_ring_fixture/bsp_ring_native_probe.exe`,
SHA256 `06f93d8cc4ffe91480ea05cd3f44feac55dbe4d056af75533744ebf858c2b3e3`.
Its embedded `asInvoker` manifest was inspected before execution. Owned
child PID 81880 exited 0 in approximately 0.125 seconds with empty stderr,
under a 15-second watchdog. No timeout, exception or termination occurred.
Crash UI suppression and exception handlers applied only to that process.

To reproduce, use this fixture commit with kernel files identical to the
frozen candidate, require the same installed-image hash, run the seed
verification first, and compile the nine translation units above into a
local safe-name Win32 executable with the listed flags/embedded manifest.
Run it with the installed PE path as its only argument in an isolated
subprocess with a 15-second timeout; terminate only that owned child if it
times out. The fixed byte spans/fixups and hash checks are self-contained
in the fixture. Exact local commands, byte captures and stdout receipts
from this run are retained under `local/cc16_dyn_ring_fixture/`.

This is one positive synthetic normal-path differential, not proof of
other geometries, all FP controls, allocation failures, EH, native/public
ABI equivalence, CRT identity or gameplay. It grants no function credit
or activation acceptance. Original caller reachability/full dead-chain
closure and actual game/root activation remain with the primary.
