# Raw scene map and database cleanup

Complete Source for43-byte0046B460 and174-byte0046B680 is in
`src/native_scene_record_map_clear.cpp`. The report records exact whole spans,
live-versus-installed equality and typed native call rows. Target is the existing
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 base400000.
Descriptive names are hypotheses. The existing `Seg6Host` reconstruction is
retained separately; these entries operate directly on actual raw storage.

## Concrete provider composition

0046B460 receives ECX tree and returns with plain RET/no semantic result.
Tree+4 is head, head+4 is root. It calls complete0046AD10 before any reset.
The latter uses the real raw pool/getter/free services, not a semantic host or
new cleanup callback. On success, capture current head and write parent=self;
capture current head again BEFORE count+8 is zeroed, then write that captured
head's left=self. Reload head again before right=self. The allocator word+0,
head color/sentinel and mapped values are not written by this wrapper.
Even the sentinel-root case reaches the destructor's sentinel read; no new
count-based shortcut or rollback is added. Provider failure escapes before reset.

## Database mapped-object destruction

0046B680 receives ECX database and returns with plain RET/no semantic result.
The map is database+18h. Count at database+20h is checked first; zero skips all
iterator, head, object and cleanup work. Nonzero captures head.left, initializes
the actual iterator NODE then OWNER, and preserves those captured values in
the same register-like variables as native EBX/EDI between increment calls.

At each loop iteration capture the CURRENT head before validating captured
owner nonzero/equal-tree. The actual current-CRT invalid-parameter provider may
return; compare the captured node with the already captured head afterward.
Nonend validates owner, checks against current owner.head, then compares against
current owner.head again. That second comparison's condition is captured BEFORE
the mapped+14h load. A returning final handler therefore does not change which
captured object is subsequently destroyed. Zero the mapped slot even for null.
For nonnull mapped objects read current vtable, slot0, and issue the actual
Win32 `__thiscall` with flag1. No invented virtual interface or callback is used.
Its returned value is unused. A throw leaves the slot cleared and does not add
iterator advancement, node cleanup or rollback.

Call the complete00467980 iterator on the actual eight-byte backing; afterward
reload node first, owner second. Thus a reached handler/object call may change
the later iterator path through valid aliases. Once the captured node reaches
the captured head, destroy current root and perform the same exact reset schedule
as0046B460. The native POPs do not introduce further memory reads in that reset.
The only live caller is004D2BB0 at004D2BD4 (`BSP_Game_DestroyWorld`).

## Interface, evidence and remaining limits

Source exposes caller-owned stable iterator backing plus the raw pool context.
Valid payload aliases are supported, but original physical private-frame aliases
and native register residue are not claimed. The backing and all reached
native-layout storage must remain valid through returning handlers, virtual calls
and cleanup. Vtable entries must be callable Win32 deleting-destructor entries
with the observed this/flag convention. Nominal mapped object type is unproved.
Original allocator/CRT/FH3/private-stack ABI, binary replacement, application
adoption and game behavior remain separate from this Source implementation.

Both full listings (16/62 instructions), original callers, direct transfers,
indirect call setup, concrete child contracts and all alias-sensitive reloads
were inspected. No Source function is substituted for missing branches; current
CRT handler return paths remain complete. Native bytes match the preserved PE.
Build/review results are recorded in the report when completed. Existing CTests
are regression checks; no new native differential or game execution is claimed.
