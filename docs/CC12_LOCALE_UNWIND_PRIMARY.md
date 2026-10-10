# Locale constructor cleanup forwarding: primary acceptance

The complete8/11/11-byte actions reload ECX from[EBP-10h] and tail-jump to
403970,402F70 and435560, adding0Ch/28h for the latter two. There is no stack
adjustment, local state store or target-effect proof. Member+28h is the address
of the table-pointer member, not its stored allocation pointer. Each action
reloads the saved receiver independently.

The retained constructor saves the pool at registrationR-4, where its entry
ESP=S andR=S-0Ch. The funclet therefore requires dispatcherEBP=S=R+0Ch. Actual
FH3 setup is unproved. The table declares2→1→0→-1 even though the constructor
normally stores only-1,0,2. Dispatcher invocation, target completion, null
table/capacity32 handling, SEH and double-failure behavior remain open.

Root verified the copied guarded proof,4229 actual frozen Git occurrences,
13 commit/tree objects and4217 closed Source paths. Its later record Source
pair remains a separate epoch. Thirty worker Native bytes/eight instructions
are fresh; inherited constructor/profile/PE/handler facts preserve provenance.

Root added three evidence comments under the Ghidra write lock, preserved old
comments and all existing Unwind names, made no rename request, and saved GPR.
Before/after complete metadata is identical except modification79→83; bodies,
listing, flow, no-return flags and undefined(void) prototypes are unchanged.
Terminal JMP CALL_RETURN/effectiveCALL_TERMINATOR defects remain. Nine affected
exports were refreshed and physically retained; prior annotations are journaled.
The sharded address/name and reconstruction ledger records retain uncertainty.

Next gate is metadata and bounded body admission for435560.403970/402F70 effects
and FH3 frame setup remain separate. No owner C++, build, test, probe, OS/game
call, ABI compatibility or application activation is added by this acceptance.

Evidence: reports/cc12_locale_unwind_primary.json.
