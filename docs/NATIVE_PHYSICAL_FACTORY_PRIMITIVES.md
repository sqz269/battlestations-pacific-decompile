# Physical factory publication and provider-pool primitives

Packet `orch2_physical_factory_primitives_aq` reconstructs seven complete bodies
over actual Win32 storage. It builds on discovery commit
`eeab5ff165516d44e6f2b16ff9d59653793c8f49` and
[the AP packet map](NATIVE_PHYSICAL_FACTORY_PACKET_MAP.md). Names are descriptive
hypotheses. No Ghidra definitions, annotations, or saved analysis were changed.

The source uses the existing raw singleton getter, registration wrapper, guard
cleanup, CRT allocation/free service, Win32 critical sections, and canonical
allocator-list domain. It introduces neither a singleton manager nor a pool
lifecycle. These are new C++ interfaces; original calling conventions, numeric
profile dispatch, FH3 identity, application startup, and game behavior are not
claimed to be drop-in compatible.

| Entry | Inclusive end | Bytes | Original ABI | Source API | Coverage |
|---|---|---:|---|---|---|
| BED990 | BEDA5D | 206 | No consumed arguments; EAX factory; RET | `get_native_physical_factory_00bed990` | complete |
| BED910 | BED917 | 8 | ECX secondary; SUB ECX,4; tail JMP BED950; inherited RET4 | `delete_native_physical_factory_secondary_00bed910` | complete |
| BED950 | BED989 | 58 | ECX primary; stacked flags; EAX original address; RET4 | `delete_native_physical_factory_00bed950` | complete |
| BF2D50 | BF2D8F | 64 | ECX block; stacked block index; EAX same block; RET4 | `initialize_native_physical_provider_block_00bf2d50` | complete |
| BF34D0 | BF360B | 316 | ECX initialized pool; EAX slot; RET | `acquire_native_physical_provider_slot_00bf34d0` | complete |
| BF2FC0 | BF3028 | 105 | ECX initialized pool; stacked slot; RET4 | `return_native_physical_provider_slot_00bf2fc0` | complete |
| BF3200 | BF320B | 12 | ECX slot; pushes it, selects ECX=109DBF0; CALL BF2FC0; RET | `return_native_physical_provider_slot_00bf3200` | complete |

The complete owned span is 769 bytes with 17 CALL instructions: 10 direct calls
and seven calls through the installed PE's Enter/LeaveCriticalSection imports.
The report records every call site and native callee. JMP transfers are separate
tail rows; they are not presented as CALLs merely because Ghidra describes some
incoming references as `UNCONDITIONAL_CALL`.

## Factory storage and ordering

`NativePhysicalFactoryContext` borrows stable references to the application's
actual volatile publications at 1090AA0 and 109DBE8. BED990 returns its first
factory read immediately when nonnull. Otherwise the first canonical manager
getter supplies the section pointer at manager+10. That captured section, when
nonnull, is entered and its explicit DWORD depth at section+18 is incremented.
The factory publication is rechecked under that section.

The eight-byte allocation receives +4=D68CF4, +0=D68CFC, then +4=D68CF8, before
publication. The primary factory interface is at +0 and its registered lifetime
interface is at +4. The code reloads the publication and captures +4/null at
BEDA1D..BEDA2D **before** the second manager getter at BEDA2E. Registration at
BEDA35 uses the second returned manager and that already captured pointer. The
normal exit releases the first captured section and reloads the factory
publication again. A later publication change must not change the registered
pointer retroactively.

The native state-zero unwind map at E02010 and FH3 info E02018 select
CC74C0 -> 411EE0 for the guard at EBP-14. The state is armed after Enter and the
depth increment. Registration failure retains the allocated and published
factory; only the captured guard is released. The source uses C++ exception
cleanup through the canonical raw guard service. Original FH3 exception
dispatch and hardware-fault unwinding remain outside that source contract.

BED950 clears the factory publication, writes CE3818 through the adjusted
secondary pointer and CFE9F4 through the primary pointer, optionally frees the
whole allocation when flags bit zero is set, and returns the original address.
It does not unregister. BED910 is the required secondary-to-primary adjustment
for a lifetime dispatch on factory+4. Null or malformed owner storage is not a
supported source-domain input; the native stores would fault too.

## Pool storage and ordering

`NativePhysicalProviderPoolContext` requires an already initialized, linked
actual 38h-byte pool corresponding to 109DBF0 and the same canonical E188B4
`AllocatorListDomain`, including its concrete D68E68/BF3430 trim binding. The
domain reference expresses that precondition; these primitives do not create,
link, bind, validate, trim, or destroy a domain or pool.

| Offset | Actual field |
|---|---|
| +00/+04/+08 | Numeric D68E68 profile and shared allocator-list links |
| +0C | Real 18h-byte Win32 CRITICAL_SECTION |
| +24 | Explicit DWORD section depth |
| +28/+2C/+30/+34 | Block-pointer table, count, capacity, first available index or FFFFFFFF |

BF3250 is the external producer of those fields. Its complete body establishes
the canonical list membership, section, empty headers and initial table;
CD9010 invokes it for the global pool and registers CE10F0 with CRT atexit.
BF33A0 and BF3430 supply destruction and trimming. They are analyzed dependency
boundaries, not implementations hidden inside this packet.

Each 1F4h-byte block has eight 3Ch-byte slots, eight WORD free indices at +1E0,
a WORD free count at +1F0, and untouched padding at +1F2. BF2D50 initializes only
that metadata: count eight, reverse free indices seven through zero, and the
block index at each slot+38. Provider construction must retain slot+38. Ordinary
acquisition therefore starts at slot zero.

When no block is available, BF34D0 publishes available=count before allocating
the block. It initializes metadata with the then-current available index. On
table growth it uses wrapping DWORD `2*capacity+2`, publishes capacity before
allocating `4*new_capacity`, copies using the current count and current old
table pointer at the native load sites, frees the current old table, and then
publishes the replacement. Insertion and count increment also use current
fields. The source preserves the native conditional destination stores and
WORD arithmetic. It does not add overflow rejection or failure rollback.

Acquisition decrements the selected block's WORD free count, pops its WORD
index and scans later blocks only when that block is exhausted. Return reads
slot+38, selects the current table entry, computes the signed 32-bit slot-minus-
block difference divided by 3Ch, pushes the resulting WORD index, increments
the current free count, and lowers available using an unsigned comparison.
BF3200 explicitly uses the same supplied pool as the native global selection.
No exceptional unlock is installed: allocation failure can retain modified
headers and a held real section, matching the body without an EH guard.

## Evidence and verification

Every Ghidra CLI batch verified project `bsp`, program
`/battlestationspacific.exe`, x86 LE32, base 00400000. Installed binary SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The JSON report preserves owned/support bytes, disassembly, source contracts,
live prototypes/xrefs, imports, gaps, and hashes. AP evidence supplies the
producer, table and EH analysis; AQ refreshed all seven owned spans and nine
support spans against both live bytes and the installed PE.

Incoming uses were checked individually. BED990 is called from BEDA90 in the
provider-manager constructor. BED910/BED950 occupy D68CF8/D68CFC lifetime table
words. BF2D50 is called by BF34D0. BF34D0's live incoming calls are BF4E35 and
BF4E66 in BF4DF0. The snapshot's BF3670 -> BF34D0 edge is stale: the complete
current BF3670 listing calls only BF30C0. BF2FC0 is called by BF3206 and BF4DE5;
BF3200's incoming CC7CD3/CC7CDB instructions are unwind JMP tails.

Strict `scripts/build.ps1`, both existing CTests, seed verification, and the
report checker passed. The checker validated ten direct rows with zero failures;
the seven import rows were resolved independently from the installed PE. The
first compile caught a missing Windows include; the corrected build log and
original failed log are retained. No permanent tests were added.

The ignored `local/physical_factory_aq/probe.cpp` is compiled for Win32 with
`/MANIFEST:EMBED` and links the built library. Original code pages are translated
by 30000000; all seven tested bodies remain byte-identical. Separate allocator/
free targets use the existing real CRT service, imports use real Win32 section
functions, and the original getter/registration bodies execute against actual
raw manager slots. Original manager construction and original CRT internals
are not asserted by this composition fixture.

The probe compares factory creation/reuse with optional real sections, primary
and adjusted secondary deletion, and a PAGE_GUARD-triggered change during the
second manager lookup. Both sides register the captured allocation+4 in the
second manager and return the replaced publication. A source-only real CRT
invalid-parameter-handler throw verifies retained publication and released
guard; original FH3 exception execution is not claimed.

Pool comparisons cover dirty block initialization, 18 acquisitions over three
blocks including table growth, direct and global-wrapper returns, exact slot
reuse, all metadata/header traces, and real section/list storage. An explicitly
synthetic header with count=capacity=9FFFFFFE forces capacity to wrap to
3FFFFFFE and requests FFFFFFF8 bytes. Both original and source throw through
the real allocation service with available/capacity changes retained, old
table/count retained, and depth/OS recursion still one. Fixture cleanup releases
the lock afterward; each native failure path abandons one 500-byte block until
process exit. This malformed-size case verifies arithmetic and failure order;
it is not evidence of a naturally reachable valid pool size.

The resumed review preserved all source, object, library, probe and prior log
hashes in `local/physical_factory_aq/resume_preserved_manifest_20260912.json`
(SHA256 `886679e65bb1b686de4b05463be32d5a2b4aafebc368cff60896f8a963421293`)
before executing the existing probe again. No resumed build or source edit was
needed. The unchanged probe passed again in
`local/physical_factory_aq/resume_probe_result_20260912.log`; the live call
checker again passed ten direct rows. The current installed PE still matches
all 16 recorded code spans (1,344 bytes), and the live undefined starts and two
three-byte gaps remain unchanged. The report embeds the artifact manifest and
separately hashes the new logs. Original passing evidence was not overwritten.

Repository name and reconstruction records now cover the seven owned entries.
There were no previous owned name or reconstruction records to supersede;
the report records the original saved names and proposed descriptive names.
These are repository records only: the six existing Ghidra function names
remain unchanged, and BED910 has `no_ghidra_function` until a separately owned
definition repair. Source CRT/FH3 identity, original registration-exception
execution and game behavior remain unverified.

## Remaining integration and definition work

Request a read-only-reviewed definition for raw BED910..BED917 inclusive
(exclusive BED918), eight bytes: `SUB ECX,4; JMP BED950`. BED950's saved body
omits BED981..BED983 and BF34D0 omits BF3566..BF3568; both raw three-byte spans
are `83 C4 04` (`ADD ESP,4`) after returning free calls. The complete source
includes those continuations. No definition or no-return repair was applied.

Supporting missing starts CD9010..CD9025 and CC74C8..CC74D1 are recorded with
inclusive ends in the report; their ownership belongs to later lifecycle/EH
work. Pool lifecycle/startup/trim, factory numeric-table shutdown binding,
BF4DF0/BF4D30 provider construction, and the external BB5590/BB5380 base remain
separate integration work. Build and fixture success do not establish those
contracts or game execution.

## AQ parent integration, 2026-09-12

All seven functions are now defined and named. BED910 was created over the verified eight-byte body ending at exclusive BED918. Standard locked flow repair restored BED981..BED983 and BF3566..BF3568; both listings now have zero call gaps. Ten direct CALL rows passed; seven import rows retain their separate import evidence. The 38 original proof files were frozen in full before further validation. Earlier statements that Ghidra was unchanged or BED910 was missing describe the worker capture, not the integrated state. Pool startup/trim, complete physical provider construction and original FH3 execution remain outside this packet.
