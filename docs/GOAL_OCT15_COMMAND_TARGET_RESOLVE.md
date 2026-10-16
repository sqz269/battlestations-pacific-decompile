# Actual command-target resolver, 00521EA0

This reconstructs the complete78B/23-instruction body [00521EA0,00521EEE).
The retained descriptive name BSP_CommandTarget_ResolveObject is a hypothesis.
Native ECX addresses the actual descriptor; EDX is unused; plain RET returns
the raw DWORD in EAX. There are no direct or computed calls in this body.
The old entity_handle_slot_00521ea0 computes a semantic table offset only;
it does not implement this body's kind/cache accesses and return schedules.

The first read is the kind BYTE. Zero returns0 without touching cache, id or
table globals. Nonzero tests the current cache DWORD at descriptor+4; if that
test succeeds, a separate current cache read supplies the return. A cache miss
captures the unsigned WORD id at+2 and signed-compares it against CURRENT
00F89A10. The low branch subtracts CURRENT00F89A0C, then reads CURRENT table
pointer00F89A54. The high branch similarly reads00F89A60 then00F89AA8. Index
subtraction, shift16-byte stride and table+offset+C all use modulo32 arithmetic.
The selected raw slot DWORD is written into the captured descriptor's cache.
The low branch immediately returns the captured slot word; the high branch
falls through to a fresh cache read. The implementation preserves all of these
actual reads and writes with exact-width volatile accesses. Signed comparison
uses sign-bit bias, so no signed-overflow assumption or implementation-defined
DWORD-to-int conversion is needed.

NativeCommandTargetHandleTables binds the actual five global cells rather
than copying their current values into a host table. Binding metadata is stable
and disjoint from all reached raw accesses. Reached payload aliases retain the
native schedules when those accesses remain valid. There is no new kind-type,
cache-pointer, table-bound, id-range or nonnull-table validation, and no helper
call, callback, synthetic table or singleton getter is substituted.

The still-incomplete entity issue-command parent has these actual transfers:

- 0077D6BD CALL00521EA0.
- 0077D6ED CALL00521EA0.
- 0077D782 CALL00521EA0.

That parent also needs genuine raw message construction,
routing, group forwarding and actual callable vtable targets; this leaf does
not close it. The separate parent inlined handle resolution is not replaced.
No original private register/argument/fault/binary ABI or game proof is claimed.
Report reports/goal_oct15_command_target_resolve.json records evidence and
the independently reviewed build state. No new tests are added for this leaf.
