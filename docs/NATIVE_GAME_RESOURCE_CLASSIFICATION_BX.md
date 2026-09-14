# Actual game-resource classification

Addresses: 0071BB40, 006F9A80, 00721C90, 00B86950.

The source implements the actual CFD8CC game-resource slotC target, using the
74h object constructed by the existing 71B810/71B870 source. It first calls the
actual B87AA0 primary item append, including for a null pointer. For a nonnull
item it probes the current slotC predicate in order64,54,44. Only the first
matching list receives the raw pointer at resource+64,+54,+44. Neither primary
append nor classification adds a reference. A throwing predicate or secondary
list insertion leaves the completed primary append in place.

The first test reads the current E19B64 token before loading the item's table.
For the next two tests it reacquires the table before calling the E19A98 getter
6F9A80 or E19BE4 getter721C90, then reads slotC from that captured table. Only AL
is tested. The two getters each load one current DWORD and return. Source
bindings borrow the actual publication cells; no numeric type values or class
names are invented.

Fallback target B86950 ignores its receiver, compares the stacked token against
current109021C/1090220/1090224 in order, and returns AL1 on the first equality,
otherwise AL0. Upper EAX is unspecified in the original. The source binds only
this proven predicate; other item type targets go to an explicit supplied
implementation. Type44/54/64 are field-offset labels, not recovered class names.

`NativeGameResourceDispatchCalls` composes with the BW root dispatcher and its
existing concrete bindings. It handles target71BB40 and forwards other resource,
renderer and parser targets. Thus the game resource is not aliased to a default
resource whose only behavior is B87AA0.

The controlled-child fixture uses actual game resources, fallback items,
memory streams, raw readers/nodes, string and hierarchy pools, BW traversal and
the new list implementation. Actual fallback predicates classify root-produced
items. Explicit fixture predicates cover first-match priority,54/44 selection,
nonmatches, table/token changes between calls, a throwing predicate and null
primary append. Refcounts remain1. Derived lists own only their backing storage;
fixture teardown destroys each primary item once and frees all three arrays.

The fixture's parser and alternate predicates are boundary probes. It does not
execute the original71BB40 body, publish production type cells, reconstruct all
native parser/type bodies, or prove native CRT/FH3/SEH, private stack aliases or
gameplay. Actual manager registration, parser ownership, B80720 load/cache
composition and queue shutdown remain open. All entry names are hypotheses and
all source interfaces have a new C++ ABI.

See `NATIVE_GAME_RESOURCE_LISTS_BX.md` for the complete array dependency and
`reports/native_game_resource_classification_bx.json` for byte/call evidence.
