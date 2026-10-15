# Actual scene database record and pending-object release

The complete306-byte0046BD10 body is reconstructed in
`src/native_scene_database_release_pending.cpp`. Full112-instruction assembly
and its callers, raw child providers and indirect call setups were inspected.
Target is `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 base400000.
The report contains the exact whole span, installed/live equality and typed
transfer rows. Names are descriptive hypotheses, not recovered symbols.

## Private backing and tree walk

Native ECX is database; plain RET has no defined semantic result. Source adds
raw pool context and explicit stable writable12-byte private backing:
saved database+0, iterator owner+4/node+8. Initial contents are not read. Capture
head.left BEFORE publishing saved database, iterator NODE, then iterator OWNER.
The original tree remains the captured database+18h throughout the function.

Capture current head before owner validation; a returning handler does not
reload that captured head or register-like owner/node. Nonend validates owner
and current owner.head, then captures mapped+14h and calls a nonnull object's
current deleting-destructor vtable slot0 with flag1. Slots remain intact.
The real00467980 provider consumes actual iterator backing; reload node FIRST,
owner SECOND afterward. Valid payload aliases may change later traversal.
No count-zero shortcut, node ownership adapter, rollback or new callback exists.

After the tree walk reload the saved database cell, then read pending begin
at+28h BEFORE computing vector=saved_database+24h. Thus a reached object or
handler may redirect the pending vector through valid saved-cell aliases, while
the subsequent map clear still targets the original captured tree.

## Pending objects and exact map reset

Capture begin and compare with current end; report an invalid range through the
real returning current-CRT provider. Each iteration captures current end BEFORE
checking current begin>end. Compare captured slot/end afterward, then validate
slot against current end before and after the actual nonnull deleting call.
Do not clear pointer slots. Increment captured slot by4 modulo2^32. No end
snapshot is carried across iterations or virtual calls.

The native CMP ESI,ESI at0046BD9D always takes0046BD9F's JZ; diagnostic0046BDA1
is present in the listing and transfer report but unreachable. Source does not
add a reachable report there.

After the object walk destroy current root of the ORIGINAL captured map.
Reuse complete0046B460's exact inlined native schedule: destroy subtree;
capture head/write parent=self; capture head BEFORE count0/write captured left;
reload head/write right=self. This new Source call is a concrete reuse of
identical raw operations, not a claim that native0046BD10 calls0046B460.
Failures propagate at the current frontier without rollback or partial cleanup.

## Final range erase

Capture erase_end before validating current begin. Capture erase_begin after
that handler; compare with current end and allow that handler to return too.
Equal captured endpoints skip all final memory movement/end publication.
Otherwise reread current end, subtract captured erase_end with DWORD wrapping,
arithmetically shift right2, and multiply by4 modulo2^32. Capture resulting end
as erase_begin+that byte count BEFORE any copy. Call actual current memmove_s
only for a strictly positive SIGNED shifted word count, with destination size
and source byte count both equal to that captured byte count. Ignore its return
and publish captured resulting end afterward. Negative and fractional wrapped
deltas retain the native publication behavior; the normal valid empty suffix
is not substituted for these returning-diagnostic paths.

## Verification scope

Callers are0046EFDF in0046EFB0 (`BSP_SceneDatabase_Destruct`) and004DAC03 in
the game teardown function; exact containing entry is checked in the report.
Direct CALL rows include the unreachable diagnostic as such. Two actual
indirect vtable calls remain dynamically target-dependent. Source requires
valid reached backing and current-provider-compatible raw allocations/vtables.
Original physical private-frame aliases, CRT/heap/exception/binary ABI,
application adoption and game behavior remain unvalidated. Existing Seg6Host
Source is preserved separately. Build and independent-review results are
recorded in the report once completed; existing checks are regression checks,
not native differential execution of this new body.
