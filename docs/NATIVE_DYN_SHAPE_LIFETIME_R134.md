# Native convex shape lifetime R134

Addresses: 004062C0, 00408040; bindings00C4DAA0, 00C4DC60, 00C4DDE0.

The type4 ConvexMeshShape attachment produced by C57F50 now has a concrete scalar
destructor. Physics teardown can borrow the exact `DynBodyCreationContext` used
for its shapes and return slots to that context's original convex allocator.
The binding travels from C4DDE0 through C4DC60 to both body lists in C4DAA0.
It selects the concrete body only when the attachment's table matches the borrowed
construction table. Absent bindings and other tables retain actual virtual slot4
dispatch. A supplied context with no pool or table is rejected before graph access.

## Native contract

`004062C0..004062DC` is a complete29-byte normal body: ECX shape, stack flags,
RET4 and EAX captured shape. The D7A0AC vtable's slot1 identifies it. It writes the
base Shape profileD7A04C and, when flags bit0 is set, calls00408040 with the slot
in EAX. The shape's borrowed mesh at210 and other fields remain stale. No partial
runtime vtable or generic shape destructor is synthesized.

The existing113-byte00408040 reconstruction is reused. It enters the original
pool's real critical section, increments lock depth, uses shape214 to select the
page and writes the slot index into its uint16 free stack. The count increments,
the earliest free page decreases when necessary, then lock depth decrements and
the critical section is left. Shape size218h includes the page-index metadata;
each10D04h page owns128 slots and its free-index table/count at10C00/10D00.

The allocator and its pages must outlive all shapes. This packet returns slots;
it does not destroy the global allocator or free the borrowed mesh. Flags0 and100
stamp the base profile but retain the slot. Descriptive names remain hypotheses,
and these explicit source interfaces do not reproduce the native register ABI.

## Validation

- Strict MSVC Win32 build and three existing CTests pass.
- 3,414 live Ghidra bytes match the original executable, including29 new scalar
  bytes,113 existing pool-release bytes, the SAP/physics envelopes and table data.
  No new listing repair was required; function bounds and CALL sites were checked.
- Thirty-six native/source pairs match **790 observations /20,777,328 normalized
  bytes**. Eight use actual constructed convex shapes through world teardown;
  twelve retain SAP regressions; sixteen cross1/128/129/257 slots with flags
  0/1/100/101, checking reverse frees, base profile, return value, all pool/page
  bytes and reuse of the earliest free slot across one, two and three pages.
- Copied native4062C0 calls copied408040 with relocated allocator globals and
  real Win32 critical-section APIs. The source reuses its established pool release.
  Constructors and task-manager shutdown are shared; the consumed normal CRT
  reverse iterator remains controlled as documented in R133.
- Missing pool/table guards, absent/foreign table virtual dispatch and source
  attachment-boundary failure/no-replay checks pass. The existing52-case parent
  comparison matches3,537 snapshots /141,402,076 bytes, with four failure cases.

The fixture normalizes only identified pointer fields within convex pages. Packed
16-bit free indices, matrices, counts and other scalar fields compare as raw bytes.
This fixed an initial fixture mismatch caused by packed indices coinciding with a
heap address. Critical-section OS bytes are omitted. Pool pages and the fixture's
critical section are disposed after comparison; source failure retains diagnostics
until ownership has been resolved. Other R132/R133 task/handle limits still apply.

## Remaining work

Other attachment classes and the remaining shape virtual methods still require
their actual runtime implementations. Native exception/register compatibility,
allocator failure, malformed provenance, private stack aliases, concurrent pool
mutation, repeated cleanup, raw-game admission and gameplay remain unproved.
No ordinary application rerun is presented as evidence for this packet.

`reports/native_dyn_shape_lifetime_r134.json` records exact bytes, call sites,
fixture provenance, Ghidra annotations and tested/integrated artifact archives.
