# Actual input modifier loading

Addresses: 00698A10 (prefix through00699039); 00697F40; 00698980.
Storage evidence: 00557590, 00698040, 006980E0, 00698310, 00698400,
006988F0, 004D49B0. Prior prefix: docs/NATIVE_INPUT_CONFIGURATION_LOAD.md.

The source now extends the established script prefix through all five
`InputModifiers` tables. It obtains the actual globals object from the same-base
Lua owner and returns that object to its caller, still owned. The caller must
destroy it after its remaining work. On an exception in the modifier stage,
the prefix releases it. This is a new source interface, and stops before the
input-manager call at69903A. It must not replace the full native698A10 entry.

The gate checks the pair vector at configuration+4D0: a nonnull begin and
nonzero arithmetic `(end-begin)>>4` skip **all five groups**. Otherwise it
looks up `InputModifiers`, then `SwapStickPairs`, and constructs real Lua key
and value temporaries. Each iteration appends an empty checked10h row before
reading Lua index1. It converts that value to an integer, captures the current
outer end, runs the original three returning CRT checks for the last row,
appends the integer, and destroys the Lua temporary. Index2 repeats that entire
schedule before advancing the key/value iterator.

The same retained table object is reassigned for `SwapStickGeneral` (+500),
`SwapStickMap` (+510), `InvertCameraY` (+4E0), and `InvertPlaneY` (+4F0), in that
order. Each name lookup has its own temporary, which is destroyed after the
assignment and before iteration. Values are converted and appended in Lua
iteration order. There is no sorting, deduplication or array-only substitute.
Normal cleanup destroys value, key, table and InputModifiers in that order.

The two append APIs are concrete, stateless storage adapters for recognized
library operations. They mutate the actual checked10h headers and preserve
opaque word0. The flat adapter reuses the existing single-DWORD insertion
contract for growth and adds the native direct-placement fast path. The nested
adapter deep-copies owning rows, retains an aliased input before relocation,
grows capacity by the observed 1.5 rule, and destroys old row allocations before
freeing the outer allocation. It uses the existing CRT allocation domain.
No parallel owning container or original STL ABI is introduced.

Ghidra had three false free-call termination overrides in698400. They were
repaired under the write lock and saved; the receipt is
`reports/native_input_configuration_modifiers_aj_flow.json`. The repaired body
has245 instructions and no call gaps. Its header publication after old-storage
destruction is capacity, end, then begin. Source storage and native byte
comparison retain that behavior on valid separately owning ranges.

The existing C7EE45 FH3 handler's map confirms states3..14: globals unwinds to
-1; InputModifiers to3; table to4; key to5; value to6; and the temporary row and
six lookup temporaries to7. Guards acquire ownership only after construction
returns and clear ownership before normal destruction. Original FH3 execution
and arbitrary stack aliases are outside the source ABI claim.

One ignored component fixture runs the original1578-byte prefix, the original
nested row append/insertion/copy/deletion instruction graph, and the original
flat append front end. Real source Lua5.1.1, file-loading and cleanup leaves,
the established flat insertion contract, and CRT allocation services are
shared. The fixture supplies installed fundamentals.lua and Inputs.lua through
external streams. Its explicit return tail observes and releases globals and
restores the original saved registers; it does not pretend native698A10 returns
at69903A. Unused library paths fail the fixture if entered.

The fixture compares1186 words: all five installed modifier groups, four prefix
invocations with one Lua bootstrap, repeated Inputs table creation, a prefilled
pair gate that skips every group, row growth and a source-row alias across
growth, and three returning CRT repairs around the captured last-row pointer.
All Lua registrations, strings and streams are released. The84 CALL rows and
eight disk/live seed comparisons pass. Final source/archive/build hashes and
the strict Win32 test result are pinned in
`reports/native_input_configuration_modifiers.json`.

The remaining Inputs/action parser starts at69903A and is still required before
application integration. Full startup, original exception ABI, physical/archive
VFS routing, device polling and gameplay have not been validated here. No game
launch or single-instance bypass occurred.
