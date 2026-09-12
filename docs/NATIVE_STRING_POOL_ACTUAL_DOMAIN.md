# Actual string-pool and physical-factory lifetime composition

Addresses: 00419CC0, 00BD0400. These are adapter extensions to already
reconstructed routines; this packet adds no complete routine to the count.

`ActualNativeStringPoolStorage` and `native_string_pool_get_or_create_00419cc0`
now accept the application's actual `void* volatile&` 01090AA0 publication.
Their existing `SingletonLifetimeDomain` overloads remain available. The raw
overload shares the manager used by `SoundLifetimeAccess` and
`NativePhysicalFactoryContext`; it creates no second semantic lifetime domain.

| Routine | Coverage | Original ABI | Change |
| --- | --- | --- | --- |
| 00419CC0..00419D7F | complete | no arguments; EAX pool; RET | Additional raw-manager source transport, retaining the complete getter order and source `__finally` cleanup. |
| 00BD0400..00BD04C4 | complete normal schedule with existing finite dispatch boundary | ECX raw14h manager; RET | Add only the verified D68200 pool and D68CF8 factory-secondary deletion profiles and their required borrowed bindings. |

The getter captures the first manager's actual section at +10, enters it and
increments its actual +18 counter before arming cleanup. It rechecks 01090AA8
under that captured section. Allocation uses the existing native-sized CRT
service, and constructor failure frees the captured allocation. Publication
precedes the second manager lookup at 00419D49; only then does 00419D4E reload
the current pool argument. Registration failure retains the allocation and
publication. All slow-path exits release the originally captured section, and
the return reloads the pool after unlocking. The existing semantic branch is
unchanged; the storage bridge performs a getter on every allocation and return.

At 00BD0485 the raw manager has already popped the object; ECX is that actual
registered pointer, EDX comes from its current profile's slot zero, and the
stack carries flags1. Installed D68200[0] is BD1730. Installed D68CF8[0] is
BED910, whose `SUB ECX,4; JMP BED950` adjusts the registered factory+4 before
deletion. The new finite-dispatch bindings borrow the actual pool publication
and return gate, or the actual physical-factory context, and call those
existing reconstructed deleters. Missing bindings and other unsupported
profiles continue to throw the existing source contract error. Binding fields
are appended with zero defaults; the source binding structure is now 36 bytes.

The installed executable and live Ghidra bytes agree for eleven supporting
spans totaling 574 bytes, including both complete extended routine bodies,
the two deletion slots, their full deleting bodies and the getter unwind
metadata. The independent decode retains the full BED950 range through
00BED989. The only cited gap without a Ghidra function body is
00C5E121..00C5E122 inclusive (`POP ECX; RET` after state1's free). The getter
handler at 00C5E123 now has a live function through 00C5E12C. The report records
every outgoing call in the two extended routines and both getter cleanup
transfers. An archive of 10,105 incoming getter references is evidence of
reachability, not a claim that every caller's body was reconstructed.

The strict Win32 build and both existing CTests passed. The focused ignored
fixture's second immutable attempt passed five original-instruction/source
comparisons and two source-only unwind checks (13,280 assertions):

- BEDA60/BEDAC0 with empty and populated managers, flags0, flags1,
  80000000 and 80000001. Complete normalized A0h owner bytes, the real factory
  list and singleton registrations, actual pool returns, and allocation/free,
  lowercase and captured virtual-call order agree.
- The inherited base-constructor state9 lowercase failure cleans its members.
- After the derived D68D04 write, the fixture's existing CRT observation hook
  throws only on the physical factory's 8-byte allocation. Real BE1F60 cleanup
  unregisters VFS, clears all six heads, releases both tracked locks, retains
  the caller-owned allocation, and leaves the canonical pool available to drain.

The fixture executes 1,193 unchanged installed instruction bytes: the derived
constructor/deleter plus the three base manager bodies. External transfers
use separately mapped bridges to actual reconstructed source services. A
suspended-child launcher reserves five required original 64KiB pages before
CRT startup; the child verifies each private mapping and ownership marker
before writing it. It never overwrites an existing mapping. Before the first
execution, 531 files were physically frozen, including sources, headers, the
linked library and executable, and all 131 linked objects; every copied object
equals both its current build output and its archive member. Post-execution
hash verification found no changes. Attempt01 remains retained independently.

All fixture consumers borrow the same actual01090AA0/AA8/AA4 cells, with actual
0109DBE8 for the factory. The VFS manager is explicitly deleted first; its
base destructor unregisters it. Only then does the real source BD0400 drain
the factory-secondary and pool, clear their publications, set the pool return
gate, and release its section/vector. The caller frees the raw manager and
clears 01090AA0. No observed allocation remains live.

D68D04 VFS dispatch during singleton shutdown remains unsupported. This
pre-drain VFS deletion fixture is not proof of arbitrary full-process shutdown.
`NativePathCanonicalizerRuntimeServices` also retains its semantic constructor;
the fixture supplies its existing service interface with the real CRT/tolower
and the same raw pool getter. Original FH3/SEH execution, hardware-fault cleanup,
CRT/exception ABI identity, publication races/replacement and game startup or
gameplay are unvalidated. Neither source adapter is a drop-in original ABI.

Evidence: `local/native-string-pool-actual-domain-evidence/attempt02/verification.json`
and `local/native-derived-composition-au/attempt02/summary.json`. The reusable
local driver accepts `--repo <current-checkout> --attempt <fresh-name>`;
`--prepare-only` freezes without execution, and `--execute-only` rechecks the
current linked source/header/object/archive-member closure before execution.
Unrelated, unlinked archive-member changes do not invalidate that closure.
