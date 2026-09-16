# Raw string-pool interface for the native VFS date query

This packet adds a source overload for the existing **249-byte BDD340** body.
It adds no newly recovered native function. The established name, previous
reconstruction records, and [date-route report](../reports/native_vfs_date_route_audit.json)
remain intact. The implementation is in `src/native_vfs_date_route.cpp`; its
contract is in `include/bsp/native_vfs_date_route.hpp`.

The caller supplies `(manager, output, name, dates, strings)`, where `strings`
borrows the actual 01090AA8 pool publication, 01090AA4 return gate, and 01090AA0
singleton-manager publication. BDD340's own copy, normalization and destruction
use the complete raw-pool string interfaces. `dates` still supplies the existing
BDD0A0 mount traversal and concrete BD9E80 provider dispatch. Its owning storage
and the raw context must refer to the same actual pool. This change does not
extend traversal or provider domains or replace their existing storage contract.

`GameNativeVfsRuntime::borrow_raw_services()` additionally returns `dates`, a
reference to the existing `Impl::date_context`. It allocates no context and
retains the existing runtime/input lifetime requirements. The original four-
argument query interface remains available.

## Native evidence and ordering

Original ABI: ECX captured manager; stack output/name; EAX output; RET8.
The additional C++ context parameters are source interfaces, not original ABI.

- BDD35F establishes the D683B0 stack identity, then clears the five date words
  from +14 down to +4. The initial name copy precedes name ownership.
- BDD393..BDD3B8 matches the existing raw 425F40 assignment: resize from source
  length, re-read source length, then read current destination length, source
  data and destination data before copying. BF7680-compatible overlap behavior
  and the existing zero-byte-call omission remain explicit.
- BDD3BF arms the copied name; BEE690 normalizes it before BDD0A0 traverses the
  captured manager. Existing provider dispatch keeps its returned-pointer
  semantics rather than assuming the hidden output buffer was returned.
- BDD3DE..BDD3FD publishes five words with the recovered interleaved loads and
  ascending stores. The overload returns the supplied output pointer.
- E00778 points to the two-entry unwind map E00768. State0 calls CC6300 to reset
  the visitor through BD90B0; state1 calls CC6308 to destroy the name through
  41DD20, then continues to state0. The initial copy has no name cleanup.
- BDD406 changes state1 to state0 before the normal 419CC0/BD1510 name return.
  A getter exception there must not retry name destruction. Successful return
  omits the visitor reset. A `noexcept` unwind guard makes a second cleanup
  exception terminate instead of replacing an active exception.

Live byte queries use `tools/bsp.py ghidra bytes`, which verifies `bsp` and
`/battlestationspacific.exe` against the configured `C:/Users/sqz269/bsp.gpr`
target before each query. No Ghidra mutation was performed by this packet.

## Validation and limits

The accompanying [report](../reports/native_particle_date_raw_orch4.json)
records fresh live/installed-PE parity for 16 spans (1,732 bytes): the 13 existing
native fixture bodies, BDD340's unwind funclets/dispatcher and metadata, and the
D683B0 profile. All 13 fixture bodies also match the installed PE.
The report additionally lists all six direct BDD340 call sites and the two
unwind-action tail jumps. `tools/verify_report_calls.py` checked all eight rows
against their exact live instructions and containing functions, with zero
failures. The broader fixture spans remain separate from these call rows.

The ignored probe reuses the existing date-route composition fixture, with its
borrowed pool services changed to the same raw publication cells and its rebuilt
BDD340 call changed to the new overload. It compares 11 original/rebuilt routes
using the actual rebuilt children. Checks include the returned pointer, complete
2,048-byte mount/provider arena, full pool prefix before the critical section,
returning CRT head repair, name normalization with output/input aliasing,
ordered FileStore/MPKG/MSAR hit/miss traversal, and enabled/disabled physical UTC
dates. Original string and physical dependencies still use the fixture's real
rebuilt bridges; this is native composition evidence rather than an independent
execution of every original dependency. Fixture profile/address binding and
synthetic mount storage are confined to the ignored probe.

The report records the strict standard Win32 build (`/W4 /WX /fp:strict`), eight
seed checks, the three existing CTests, and the focused composition result.
No permanent tests or CMake changes were added. Raw getter allocation failures,
second-failure termination, native hardware SEH, binary ABI compatibility, and
gameplay remain unvalidated dynamically. The new guard's failure behavior is
supported by the native unwind map and source review.
