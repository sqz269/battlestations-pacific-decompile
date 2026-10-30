# Genuine locale caller-owned inputs

Two separate Game Source fragments compose the existing real providers around
caller-owned raw8 strings. They cover language [73E06A,73E0BA) with hex state0x25
and globals [73E0BA,73E129) with hex state0x26. They do not wire startup or grant
full reload, loaded-locale, Font48, GUI or gameplay permission.

Language borrows the ready actual catalog CString, invokes real41E870, then
borrows the completed input to the pristine canonical locale operation. Globals
writes length0/data0, calls real41DD40(7,true), copies CURRENT length+1 from the
actual retained CF7774 span and borrows the completed input for registration.
Construction must return before the owning caller action is armed.

Both normal footers capture DATA, consume the selected edge, and only for
nonNULL data read CURRENT length+1, call real419CC0, check healthy metadata and
return captured storage through realBD1510. Selected exception cleanup consumes
its armed edge first and calls CURRENT-header41DD20. Prearm failures and consumed
normal-footer failures never retry cleanup. Secondary selected cleanup failure
terminates in the qualified C++ unwind model.

Immutable contexts and retained nonmoving one-shot frames preserve exact graph
identity and sticky failure/reentry. Rejected inner calls cannot consume the
outer invocation's action. Status/destruction inspect metadata and perform no
Native cleanup. Caller retains unresolved effects and providers, excludes
invalidating writers/drains and proves real catalog/span/backing validity. No
Original private-stack/reused-header, FH3/SEH/hardware or currentCRT identity is
inferred from the normalized interfaces.

Exact review, object, registration and common build receipts are recorded in the
matching report. The original settings, locale, string, host and provider bodies
are preserved; there are no new tests, entry/game runs or activation changes.

The retained-link build at8e6e82f2d passed2/2 existing checks10.64s; both
caller entries and C++ handlers/unwind tables are present. After overlay removal,
canonical Release Win32 passed2/2 checks11.25s. Cache/project have no forced
symbols; both dormant entries are absent from the canonical game map. Each Game
registration occurs once; all21 baseline provider/host paths and9Text flags
remain unchanged/off. No entry or game was invoked.
